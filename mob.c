#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <curses.h>

#include "mob.h"
#include "level.h"
#include "utils.h"
#include "effect.h"
#include "player.h"
#include "status.h"
#include "enemy.h"

#define DEF_MOB(sym, n, col, hlth, atk, cn) {\
		.symbol = (sym), .colour = (col), .name = (n), .is_bold = false,\
		.hostile = true,\
		.health = (hlth), .max_health = (hlth), .con = (cn), .attack = atk, \
		.level = NULL, .xpos = 0, .ypos = 0,\
        .moblist = {.next = NULL, .prev=NULL},\
        .turn_action = NULL,\
		.score = 0,\
        .darksight = true, .luminosity = 0}
/* should keep the same structure as MobType in mob.h */
static const struct Mob default_mobs[] = {
	DEF_MOB('H', "Hedgehog", COLOR_YELLOW, 5, 1, 0),
	DEF_MOB('S', "Squirrel", COLOR_YELLOW, 10, 2, 0),
	DEF_MOB('o', "Orc", COLOR_YELLOW, 15, 2, 7),
	/* pack mobs come after here */
	DEF_MOB('W', "Wolfman", COLOR_YELLOW, 25, 10, 10),
};

#undef DEF_MOB

/**
 * Create and return a mob of the specified type.
 * @param mobtype The type of mob to make.
 * @return The mob created.
 */
Mob * create_mob(enum MobType mobtype){
	Mob * new = xalloc(Mob);
	*new = default_mobs[mobtype];
	new->turn_action = &simple_enemy_turn;
	new->death_action = &drop_corpse;
	
	if (mobtype == ORC){
		Item * sword = xalloc(Item);

		sword->symbol = '/';
		sword->name = "Orcish Sword";
		sword->type = WEAPON;
		sword->value = 5;
		new->inventory = insert(new->inventory, &sword->inventory);
		wield_item(new, sword);
		
		if (rand() % 2) {
			Item * food = xalloc(Item);

			food->symbol = '%';
			food->name = "Food Ration";
			food->type = FOOD;
			food->value = 5;
			new->inventory = insert(new->inventory, &food->inventory);
		}
	} else if(mobtype == WOLFMAN) {
		new->turn_action = &hunter_turn;
		new->death_action = &hunter_death;

		Item * food = xalloc(Item);

		food->symbol = '%';
		food->name = "Nourishing Food Ration";
		food->type = FOOD;
		food->value = 7;
		new->inventory = insert(new->inventory, &food->inventory);
	}

	return new;
}

/**
 * Move the given mob to the new coordinates.
 * @param mob Entity to move.
 * @param x Target x position.
 * @param y Target y position.
 * @return false if the given space can't be moved into.
 */
bool move_mob(Mob * mob, unsigned int x, unsigned int y) {
	Level * level = mob->level;
	Cell * source = level->cells[mob->xpos][mob->ypos];
	Cell * target = level->cells[x][y];

	if(source == target)
		return true;

	/* allow for digging through rock */
	if (mob->weapon != NULL && mob->weapon->can_dig == true && 
		target->occupant == NULL && 
		target->solid == true && target->baseSymbol == '#') {
		target->solid = false;
		target->baseSymbol = '.';
	}

	if(target->solid == true || target->occupant != NULL) {
		return false;
	}

	source->occupant = NULL;
	target->occupant = mob;
	mob->xpos = x;
	mob->ypos = y;

	/* Check for poison water - this should not be in move, but it
	   works for now. */
	if(target->baseSymbol == '~' && mob->effect_action != &effect_poison) {
		if(mob == mob->level->player) {
			status_push("You have been poisoned!");
		}

		int duration = 5;
		if(mob->con != 0) {
			duration -= rand() % mob->con;
		}
		duration = (duration < 1) ? 1 : duration;
		afflict(mob, &effect_poison, duration);
	}

	return true;
}

/**
 * Move a mob by a relative position
 * @param mob Entity to move.
 * @param xdiff x-coordinate difference.
 * @param ydiff y-coordinate difference.
 * @return If the mob was moved successfully.
 */
bool move_mob_relative(Mob * mob, int xdiff, int ydiff) {
	return move_mob(mob, mob->xpos + xdiff, mob->ypos + ydiff);
}

/**
 * Attack a mob, modified by the weapon of the attacker and the armour
 * of the defender.
 * @param attacker The mob doing the attacking.
 * @param defender The mob being attacked.
 */
void attack_mob(Mob * attacker, Mob * defender) {
	int damage = attacker->attack;

	if(attacker->weapon != NULL) {
		damage += 1 + rand() % attacker->weapon->value;
	}

	if(defender->armour != NULL) {
		damage -= 1 + rand() % defender->armour->value;
	}

	/* Can always do at least 1 damage */
	if(damage < 1) {
		damage = 1;
	}

	/* Update the status */
	if(attacker == attacker->level->player) {
		status_push("You attack the %s for %d damage.",
		            defender->name,
		            damage);
	} else {
		status_push("The %s attacks you for %d damage!",
		            attacker->name,
		            damage);
	}

	damage_mob(defender, (unsigned int) damage);
}

/**
 * Damage a mob.
 * @param mob Entity to damage.
 * @param damage Amount of damage to apply to the mob.
 * @return If this killed the mob, a mob is dead if its health
 * drops to zero or below.
 */
bool damage_mob(Mob * mob, unsigned int damage) {
	mob->health -= damage;

	return (mob->health <= 0);
}

/**
 * Kill a mob - free it, and remove it from the lists.
 * @param mob The mob to kill
 * @return The next mob in the list, or NULL
 */
Mob * kill_mob(Mob * mob) {
	if(mob->death_action != NULL) {
		/* Any mob-specific freeing should happen here */
		mob->death_action(mob);
	}

	Level * level = mob->level;
	Cell * cell = level->cells[mob->xpos][mob->ypos];
	Mob * next = fromlist(Mob, moblist, mob->moblist.next);

	/* Remove it from the cell */
	cell->occupant = NULL;

	/* Remove from the mob list */
	level->mobs = drop(&mob->moblist);

	/* Drop its items */
	if(mob->inventory != NULL) {
		cell->items = append(cell->items, mob->inventory);
	}

	/* Free it */
	xfree(mob);

	return next;
}

/**
 * Determine if one point can be seen from another. All points are visible
 * unless there is a wall in the way. This uses Bresenham's line
 * algorithm to determine line-of-sight.
 * @param level The level to check
 * @param x0 The starting X
 * @param y0 The starting Y
 * @param x The target X
 * @param y The target Y
 */
bool can_see_point(Level * level,
                   unsigned int x0, unsigned int y0,
                   unsigned int x, unsigned int y) {
	int dx = abs(x0 - x);
	int dy = abs(y0 - y) ;

	int sx = (x0 < x) ? 1 : -1;
	int sy = (y0 < y) ? 1 : -1;

	int err = dx - dy;

	while(x0 != x || y0 != y) {
		if(level->cells[x0][y0]->baseSymbol == '#') {
			return false;
		}

		int e2 = err * 2;

		if(e2 > -dy) {
			err -= dy;
			x0 += sx;
		}

		if(e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}

	return true;
}

/**
 * Determine if a mob can see the given point. 
 *
 * Note: as this will be primarily used to render the level, a better
 * version might be to operate on a 2d array of three-state variables
 * ("visible", "blocked", and "unknown"), and just iterate the
 * algorithm with different starting points until every point is
 * known. This would avoid the need to check each individual point,
 * possibly duplicating work.
 *
 * @param mob The mob
 * @param x The target X coordinate
 * @param y The target Y coordinate
 */
bool can_see(Mob * mob, unsigned int x, unsigned int y) {
	Level * level = mob->level;

	unsigned int x0 = mob->xpos;
	unsigned int y0 = mob->ypos;

	if(!can_see_point(level, x0, y0, x, y)) {
		return false;
	}

	if(level->cells[x][y]->illuminated || mob->darksight) {
		/* Cells can be seen if they're illuminated or the mob can see
		 * in the dark */
		return true;
	} else if(sqrt(pow(abs(mob->xpos - x), 2) + pow(abs(mob->ypos - y), 2)) <= 5) {
		/* Or if they're sufficiently close to the mob */
		return true;
	} else { 
		return false;
	}
}

/**
 * Wrapper for can_see, to determine if a mob can see another mob.
 * @param moba One of the mobs
 * @param mobb The other. It really doesn't matter which way around they are.
 */
bool can_see_other(Mob * moba, Mob * mobb) {
	return can_see(moba, mobb->xpos, mobb->ypos);
}

/**
 * Drops a mobs corpse, intended to be run at the mob's death.
 * @param mob The mob whose corpse should be dropped
 */
void drop_corpse(struct Mob * mob) {
	Item * corpse = xalloc(Item);
	Cell * cell = mob->level->cells[mob->xpos][mob->ypos];
	corpse->type = FOOD;
	corpse->value = 4;
	corpse->symbol = '%';
	corpse->name = xalloc(strlen(mob->name)*sizeof(char) + 8*sizeof(char));
	strcpy(corpse->name, mob->name);
	strcat(corpse->name, " Corpse");
	corpse->effect = &corpse_effect;

	cell->items = insert(cell->items, &corpse->inventory);
}

/**
 * Moves the given mod to the next or previous level.
 * @param mob Mob to move.
 * @param toprev Determines which direction the movement is in. If true, the movement is to the previous level, otherwise to the next.
 * @return If the mob moved sucessfully.
 */
bool move_mob_level(Mob * mob, bool toprev) {
	Level * level = mob->level;
	int newx, newy;
	Level * newlevel;

	if (toprev) {
		if (level->levels.prev == NULL) {
			/* top of the cave so no previous level */
			return false;
		}
		newlevel = fromlist(Level, levels, level->levels.prev);
		newx = newlevel->endx;
		newy = newlevel->endy;
	} else {
		if (level->levels.next == NULL) {
			/* no next level so make one */
			Level * nextlevel = xalloc(Level);
			build_level(nextlevel);
			nextlevel->levels.prev = &level->levels;
			nextlevel->depth = level->depth + 1;
			level->levels.next = &nextlevel->levels;
			/* Assumes only the player can create levels */
			mob->score += 25; // arbitrary value
		}
		newlevel = fromlist(Level, levels, level->levels.next);
		newx = newlevel->startx;
		newy = newlevel->starty;
	}

	/* remove the mob from the current level */
	mob->level->mobs = drop(&mob->moblist);
	level->cells[mob->xpos][mob->ypos]->occupant = NULL;

	/* Puts the mob in the new level, inserting
	   it at the front of the list of mobs */
	mob->level = newlevel;
	newlevel->mobs = insert(newlevel->mobs, &mob->moblist);
	newlevel->cells[newx][newy]->occupant = mob;
	mob->xpos = newx;
	mob->ypos = newy;

	if (mob == level->player) {
		PlayerData * playerdata = (PlayerData *)level->player->data;

		level->player = NULL;
		newlevel->player = mob;

		if(toprev) {
			playerdata->terrain = fromlist(Terrain, levels,
			                               playerdata->terrain->levels.prev);
		} else {
			List * next = playerdata->terrain->levels.next;
			if(next == NULL) {
				Terrain * nextlevel = xalloc(Terrain);
				memset(nextlevel->symbols, ' ', LEVELWIDTH * LEVELHEIGHT);
				playerdata->terrain->levels.next = &nextlevel->levels;
				nextlevel->levels.prev = &playerdata->terrain->levels;
				playerdata->terrain = nextlevel;
			} else {
				playerdata->terrain = fromlist(Terrain, levels, next);
			}
		}
	}

	return true;
}

/**
 * Drop the given item from the mob's inventory to the floor.
 * @param mob The mob which is doing the dropping
 * @param item The item to drop
 */
void drop_item(Mob * mob, Item * item) {
	Cell * cell = mob->level->cells[mob->xpos][mob->ypos];

	if(item == mob->weapon) {
		if(item->luminous) {
			mob->luminosity --;
		}
		mob->weapon = NULL;

	} else if(item == mob->armour) {
		if(item->luminous) {
			mob->luminosity --;
		}
		mob->armour = NULL;

	} else if(item == mob->offhand) {
		if(item->luminous) {
			mob->luminosity --;
		}
		mob->offhand = NULL;
	}

	/* Update the cell luminosity */
	if(item->luminous) {
		cell->luminosity ++;
	}

	/* Update the inventories */
	mob->inventory = drop(&item->inventory);
	cell->items = insert(cell->items, &item->inventory);
}

/**
 * Drop the given items from the mob's inventory to the floor.
 * @param mob The mob which is dropping the items
 * @param items The list of items to drop
 */
void drop_items(Mob * mob, List ** items) {
	if(items == NULL) {
		return;
	}

	for(unsigned int i = 0; items[i] != NULL; i++) {
		Item * item = fromlist(Item, inventory, items[i]);
		drop_item(mob, item);
	}
}

/**
 * Pick up an item from the floor
 * @param mob The mob which is getting the item
 * @param item The item to get
 */
void pickup_item(Mob * mob, Item * item) {
	Cell * cell = mob->level->cells[mob->xpos][mob->ypos];

	/* Update luminosity */
	if(item->luminous) {
		cell->luminosity --;
	}

	/* Update inventories */
	cell->items = drop(&item->inventory);
	mob->inventory = insert(mob->inventory, &item->inventory);
}

/**
 * Pick up the given items from the floor into the mob's inventory.
 * @param mob The mob which is getting the items
 * @param items The list of items to get
 */
void pickup_items(Mob * mob, List ** items) {
	if(items == NULL) {
		return;
	}

	/* Update the cell luminosity */
	for(unsigned int i = 0; items[i] != NULL; i++) {
		Item * item = fromlist(Item, inventory, items[i]);
		pickup_item(mob, item);
	}
}

/**
 * Wield an item
 * @param mob The mob which is doing the wielding
 * @param item The item to wield
 */
void wield_item(Mob * mob, Item * item) {
	Item ** pos;

	switch(item->type) {
	case WEAPON:
		pos = &mob->weapon;
		break;
	case ARMOUR:
		pos = &mob->armour;
		break;
	default:
		assert(false);
	}

	if(*pos != NULL) {
		(*pos)->equipped = false;
		if((*pos)->luminous) {
			mob->luminosity --;
		}
	}

	*pos = item;
	item->equipped = true;

	if(item->luminous) {
		mob->luminosity ++;
	}
}

/**
 * Heal a mob, up to a maximum of mob->max_health.
 * @param mob Mob to heal
 * @param amount Amount to heal by
 */
void heal_mob(Mob * mob, unsigned int amount) {
	mob->health += amount;

	if((unsigned int)mob->health > mob->max_health) {
		mob->health = mob->max_health;
	}
}

/**
 * Consume an edible item. This removes it from the inventory and frees it.
 * @param mob The mob which is consuming
 * @param item The item to consume.
 */
void consume_item(Mob * mob, Item * item) {
	if(item->effect != NULL) {
		/* Apply the effect if the item has one */
		item->effect(mob);
	} else {
		/* Otherwise, heal or damage the mob */
		if(item->value >= 0) {
			heal_mob(mob, item->value);
		} else {
			damage_mob(mob, -item->value);
		}
	}

	mob->inventory = drop(&item->inventory);
	xfree(item);
}
