#ifndef EFFECT_H
#define EFFECT_H

#include "mob.h"

bool is_afflicted(Mob * mob);
void afflict(Mob * mob, void (*effect)(Mob *), int duration);
void effect_poison(Mob * mob);
void cure_poison(Mob * mob);
void corpse_effect(Mob * mob);

#endif
