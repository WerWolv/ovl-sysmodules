#pragma once

#define NONE 0

#include <vector>

// Side Taunt

// DI
/*
 0, 0.785398, 1.570796, 2.356194, -3.14159, -2.356194,  -1.570796, -0.785398
 0, pi/4,     pi/2,     3pi/4,    pi,       5pi/4,      3pi/2,     7pi/4
*/

/* DI */
#define DI_RANDOM_IN_AWAY 9
const std::vector<std::string> di_items{"None", "Out", "Up Out", "Up", "Up In", "In", "Down In", "Down", "Down Out", "Random"};

// Attack Option
#define MASH_NAIR 0
#define MASH_FAIR 1
#define MASH_BAIR 2
#define MASH_UPAIR 3
#define MASH_DAIR 4
#define MASH_NEUTRAL_B 5
#define MASH_SIDE_B 6
#define MASH_UP_B 7
#define MASH_DOWN_B 8
#define MASH_UP_SMASH 9
#define MASH_GRAB 10
const std::vector<std::string> attack_items{"Neutral Air", "Forward Air", "Back Air", "Up Air", "Down Air", "Neutral B", "Side B", "Up B", "Down B", "Up Smash", "Grab"};

// Ledge Option
#define RANDOM_LEDGE 1
#define NEUTRAL_LEDGE 2
#define ROLL_LEDGE 3
#define JUMP_LEDGE 4
#define ATTACK_LEDGE 5
const std::vector<std::string> ledge_items{"None", "Random", "Ntrl. Getup", "Roll", "Jump", "Attack"};

// Tech Option
#define RANDOM_TECH 1
#define TECH_IN_PLACE 2
#define TECH_ROLL 3
#define TECH_MISS 4
const std::vector<std::string> tech_items{"None", "Random", "In-Place", "Roll", "Miss Tech"};

// Mash States
#define MASH_AIRDODGE 1
#define MASH_JUMP 2
#define MASH_ATTACK 3
#define MASH_SPOTDODGE 4
#define MASH_RANDOM 5
const std::vector<std::string> mash_items{"None", "Airdodge", "Jump", "Attack", "Spotdodge", "Random"};

// Shield States
#define SHIELD_INFINITE 1
#define SHIELD_HOLD 2
const std::vector<std::string> shield_items{"None", "Infinite", "Hold"};

// Defensive States
#define RANDOM_DEFENSIVE 1
#define DEFENSIVE_SPOTDODGE 2
#define DEFENSIVE_ROLL 3
#define DEFENSIVE_JAB 4
#define DEFENSIVE_SHIELD 5
const std::vector<std::string> defensive_items{"None", "Random", "Spotdodge", "Roll", "Jab", "Flash Shield"};

// Hitbox visualization
const std::vector<std::string> hitbox_items{"Off", "On"};