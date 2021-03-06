#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mob.h"
#include "effect.h"
#include "status.h"

/**
 * Check if a mob is suffering from an effect or not
 * @param mob Entity to examine.
 * @return true if effected by something.
 */
bool is_afflicted(Mob * mob) {
	return (mob->effect_action != NULL);
}

/**
 * Apply an effect to a mob (this removes any prior effect)
 * @param mob Entity to afflict.
 * @param effect Effect to apply.
 * @param duration Number of turns to apply the effect for (<1 = infinite)
 */
void afflict(Mob * mob, void (*effect)(Mob *), int duration) {
	mob->effect_action = effect;
	mob->effect_duration = duration;
}

/**
 * A simple poison effect - damages every turn.
 * @param mob The mob which is poisoned.
 */
void effect_poison(Mob * mob) {
	if(mob == mob->level->player) {
		status_push("The poison damages you!");
	}
	damage_mob(mob, 1);
}

/**
 * A cure poison effect
 * @param mob The mob which is poisoned
 */
void cure_poison(Mob * mob) {
	if(mob->effect_action == &effect_poison) {
		if(mob == mob->level->player) {
			status_push("You have been cured of poison.");
		}
		mob->effect_action = NULL;
		mob->effect_duration = 0;
	}
}

/**
 * Damage a mob every turn
 * @mob The mob which is burning
 */
void effect_burn(Mob * mob) {
	if(mob == mob->level->player) {
		status_push("The fire burns you!");
	}
	damage_mob(mob, 2);
}

/**
 * An effect for corpses, which poisons 50% of the time
 * @param mob The mob which is affected
 */
void corpse_effect(Mob * mob){
	if (rand() % 2) {
		if(mob == mob->level->player) {
			status_push("The corpse was rotten! You are poisoned!");
		}

		int duration = 7;
		if(mob->con != 0) {
			duration -= rand() % mob->con;
		}
		duration = (duration < 2) ? 2 : duration;
		afflict(mob, effect_poison, duration);
	}
	heal_mob(mob, 4);
}

/**
 * Armour which reflects damage. The defender is the owner of this
 * item.
 * @param owner The owner of this item (one of attacker or defender)
 * @param self The item
 * @param attacker The attacker
 * @param defender The defender
 * @param damage The damage inflicted upon the defender
 */
void reflect_damage(Mob * owner, Item * self,
                    Mob * attacker, Mob * defender,
                    unsigned int damage) {
	assert(owner == defender);

	unsigned int reflected = damage / 2;

	if(reflected == 0) {
		return;
	}

	damage_mob(attacker, reflected);

	if(owner == attacker->level->player) {
		status_push("Your %s reflected %d of the damage back!",
		            self->name,
		            reflected);
	} else {
		status_push("You got hit with %d reflected damage!",
		            reflected);
	}
}

/**
 * Equipment which sets the other on fire.
 * @param owner The owner of this item (one of attacker or defender)
 * @param self The item
 * @param attacker The attacker
 * @param defender The defender
 * @param damage The damage inflicted upon the defender
 */
void inflict_fire(Mob * owner, Item * self,
                  Mob * attacker, Mob * defender,
                  unsigned int damage) {
	(void) self;
	(void) damage;

	if(owner == attacker) {
		afflict(defender, effect_burn, 3);
	} else {
		afflict(attacker, effect_burn, 3);
	}
}
