#include <stddef.h>
#include <stdbool.h>

#include "mob.h"
#include "effect.h"

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
	damage_mob(mob, 1);
}