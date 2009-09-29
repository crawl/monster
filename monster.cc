/*
 * ===========================================================================
 * Copyright (C) 2007 Marc H. Thoben
 * Copyright (C) 2008 Darshan Shaligram
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ===========================================================================
 */

#include "externs.h"
#include "dungeon.h"
#include "mon-util.h"
#include "view.h"
#include "maps.h"
#include "initfile.h"
#include "itemname.h"
#include "spl-util.h"
#include "state.h"
#include <sstream>
#include <set>

// Clockwise, around the compass from north (same order as enum RUN_DIR)
const coord_def Compass[8] =
{
    coord_def(0, -1), coord_def(1, -1), coord_def(1, 0), coord_def(1, 1),
    coord_def(0, 1), coord_def(-1, 1), coord_def(-1, 0), coord_def(-1, -1),
};


std::string uppercase_first(std::string s);

template <class T> inline std::string to_string (const T& t);

static void record_resvul(const char *name, const char *caption,
                          std::string &str, int rval)
{
  if (str.empty())
    str = " | " + std::string(caption) + ": ";
  else
    str += ", ";

  str += name;
  if (rval > 1 && rval <= 3) {
    while (rval-- > 0)
      str += "+";
  }
}

static void record_resist(const char *name,
                          std::string &res, std::string &vul,
                          int rval)
{
  if (rval > 0)
    record_resvul(name, "Res", res, rval);
  else if (rval < 0)
    record_resvul(name, "Vul", vul, -rval);
}

static void monster_action_cost(std::string &qual, int cost, const char *desc) {
  char buf[80];
  if (cost != 10) {
    snprintf(buf, sizeof buf, "%s: %d%%", desc, cost * 10);
    if (!qual.empty())
      qual += "; ";
    qual += buf;
  }
}

static std::string monster_speed(const monsters &mon,
                                 const monsterentry *me,
                                 int speed_min,
                                 int speed_max)
{
  std::string speed;

  char buf[50];
  if (speed_max != speed_min)
    snprintf(buf, sizeof buf, "%i-%i", speed_min, speed_max);
  else
    snprintf(buf, sizeof buf, "%i", speed_max);

  speed += buf;

  const mon_energy_usage &cost(me->energy_usage);
  std::string qualifiers;
  if (cost.attack != 10
      && cost.attack == cost.missile && cost.attack == cost.spell
      && cost.attack == cost.special && cost.attack == cost.item)
    monster_action_cost(qualifiers, cost.attack, "act");
  else {
    monster_action_cost(qualifiers, cost.move, "move");
    if (cost.swim != cost.move)
      monster_action_cost(qualifiers, cost.swim, "swim");
    monster_action_cost(qualifiers, cost.attack, "atk");
    monster_action_cost(qualifiers, cost.missile, "msl");
    monster_action_cost(qualifiers, cost.spell, "spell");
    monster_action_cost(qualifiers, cost.special, "special");
    monster_action_cost(qualifiers, cost.item, "item");
  }

  if (!qualifiers.empty())
    speed += " (" + qualifiers + ")";

  return speed;
}

static void mons_flag(std::string &flag, const std::string &newflag) {
  if (flag.empty())
    flag = " | Flags: ";
  else
    flag += ", ";
  flag += newflag;
}

static void mons_check_flag(bool set, std::string &flag,
                            const std::string &newflag)
{
  if (set)
    mons_flag(flag, newflag);
}

static void initialize_crawl() {
  init_monsters();
  init_properties();
  init_item_name_cache();

  init_spell_descs();
  init_feature_table();
  init_monster_symbols();
  init_mon_name_cache();

  dgn_reset_level();
}

static std::string mons_spell_set(const monsters *mp) {
  std::string spells;
  std::set<spell_type> seen;
  for (int i = 0; i < NUM_MONSTER_SPELL_SLOTS; ++i) {
    const spell_type sp = mp->spells[i];
    if (sp != SPELL_NO_SPELL && seen.find(sp) == seen.end()) {
      seen.insert(sp);
      std::string name = spell_title(sp);
      lowercase(name);
      std::string::size_type pos = name.find('\'');
      if (pos != std::string::npos) {
        pos = name.find(' ', pos);
        if (pos != std::string::npos)
          name = name.substr(pos + 1);
      }
      if ((pos = name.find(" of ")) != std::string::npos)
        name = name.substr(pos + 4) + " " + name.substr(0, pos);
      if (name.find("summon ") == 0 && name != "summon undead")
        name = name.substr(7);
      if (name.find("bolt") == name.length() - 4)
        name = "b." + name.substr(0, name.length() - 5);
      if (!spells.empty())
        spells += ", ";
      spells += name;
    }
  }
  return spells;
}

static void record_spell_set(const monsters *mp,
                             std::set<std::string> &sets)
{
  std::string spell_set = mons_spell_set(mp);
  if (!spell_set.empty())
    sets.insert(spell_set);
}

static inline void set_min_max(int num, int &min, int &max) {
  if (!min || num < min)
    min = num;
  if (!max || num > max)
    max = num;
}

static std::string colour_codes[] = {
    "",
    "02",
    "03",
    "10",
    "05",
    "06",
    "07",
    "15",
    "14",
    "12",
    "09",
    "11",
    "04",
    "13",
    "08",
    "16"
};

#ifdef CONTROL
#undef CONTROL
#endif
#define CONTROL(x) char(x - 'A' + 1)
static std::string monster_colour(const monsters &mon) {
    int colour(mon.colour);
    if (is_element_colour(colour))
        colour = element_colour(colour, true);
    const std::string code(colour_codes[colour]);
    std::string base =
        (code.empty() ? code :
         code[0] == 'b' ?
         std::string() + CONTROL('B') + CONTROL('C') + code.substr(1)
         : std::string() + CONTROL('C') + code);
    if (!base.empty())
        base += ",01";
    return (base);
}

static std::string monster_symbol(const monsters &mon) {
    std::string symbol;
    const monsterentry *me = mon.find_monsterentry();
    if (me) {
        symbol += me->showchar;
        const std::string colour = monster_colour(mon);
        if (!colour.empty())
            symbol = colour + symbol + CONTROL('O');
    }
    return (symbol);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
	printf("Usage: @? <monster name>\n");
    return 0;
  }

  initialize_crawl();

  mons_list mons;
  std::string target = argv[1];

  if (argc > 2)
    for (int x = 2; x < argc; x++)
	{
	  target.append(" ");
	  target.append(argv[x]);
	}

  std::string err = mons.add_mons(target, false);
  if (!err.empty()) {
    const std::string test = mons.add_mons("the " + target, false);
    if (test.empty())
      err = test;
  }

  mons_spec spec = mons.get_monster(0);

  if ((spec.mid < 0 || spec.mid >= NUM_MONSTERS
       || spec.mid == MONS_PLAYER_GHOST)
      || !err.empty())
  {
    if (err.empty())
      printf("No Habla Espanol: '%s'\n", target.c_str());
    else
      printf("No Habla Espanol (%s)\n", err.c_str());
    return 1;
  }

  int index = dgn_place_monster(spec, 10, coord_def(GXM / 2, GYM / 2),
                                true, false, false);
  if (index < 0 || index >= MAX_MONSTERS) {
    printf("Failed to create test monster for %s\n", target.c_str());
    return 1;
  }

  const int ntrials = 150;

  std::set<std::string> spell_sets;

  long exper = 0L;
  int hp_min = 0;
  int hp_max = 0;
  int mac = 0;
  int mev = 0;
  int speed_min = 0, speed_max = 0;
  // Calculate averages.
  for (int i = 0; i < ntrials; ++i) {
    if (i == ntrials)
      break;
    monsters *mp = &menv[index];
    if (!mons_class_is_zombified(mp->type))
      record_spell_set(mp, spell_sets);
    exper += exper_value(mp);
    mac += mp->ac;
    mev += mp->ev;
    set_min_max(mp->speed, speed_min, speed_max);
    set_min_max(mp->hit_points, hp_min, hp_max);

    // Destroy the monster.
    mp->reset();
    you.unique_creatures[spec.mid] = false;
    index = dgn_place_monster(spec, 10, coord_def(GXM / 2, GYM / 2),
                              true, false, false);
    if (index == -1) {
      printf("Unexpected failure generating monster for %s\n",
             target.c_str());
      return 1;
    }
  }
  exper /= ntrials;
  mac /= ntrials;
  mev /= ntrials;

  monsters &mon(menv[index]);

  const std::string symbol(monster_symbol(mon));

  const bool generated =
    mons_class_is_zombified(mon.type)
    || mon.type == MONS_BEAST || mon.type == MONS_PANDEMONIUM_DEMON;

  const bool shapeshifter =
      mons_is_shapeshifter(&mon)
      || spec.mid == MONS_SHAPESHIFTER
      || spec.mid == MONS_GLOWING_SHAPESHIFTER;

  const monsterentry *me =
      shapeshifter ? get_monster_data(spec.mid) : mon.find_monsterentry();

  if (me)
  {
	std::string monsterflags;
	std::string monsterresistances;
	std::string monstervulnerabilities;
	std::string monsterattacks;

    lowercase(target);

    const bool changing_name =
      mon.has_hydra_multi_attack() || mon.type == MONS_PANDEMONIUM_DEMON
        || mons_is_mimic(mon.type) || shapeshifter
        || mon.type == MONS_DANCING_WEAPON;

    printf("%s (%s)",
           changing_name ? me->name : mon.name(DESC_PLAIN, true).c_str(),
           symbol.c_str());

    printf(" | Speed: %s",
           monster_speed(mon, me, speed_min, speed_max).c_str());

    const int hd = generated? mon.hit_dice : me->hpdice[0];
    printf(" | HD: %d", hd);

    printf(" | Health: ");
    const int hplow = generated? hp_min : hd * me->hpdice[1] + me->hpdice[3];
    const int hphigh = generated? hp_max :
        hd * (me->hpdice[1] + me->hpdice[2]) + me->hpdice[3];
    if (hplow < hphigh)
        printf("%i-%i", hplow, hphigh);
    else
        printf("%i", hplow);

    const int ac = generated? mac : me->AC;
    const int ev = generated? mev : me->ev;
    printf(" | AC/EV: %i/%i", ac, ev);

    mon.wield_melee_weapon();
    for (int x = 0; x < 4; x++)
	{
      mon_attack_def orig_attk(me->attack[x]);
      mon_attack_def attk = mons_attack_spec(&mon, x);
	  if (attk.type)
      {
	    if (monsterattacks.empty())
		  monsterattacks = " | Damage: ";
	    else
	      monsterattacks += ", ";
		monsterattacks += to_string((short int) attk.damage);

        const mon_attack_flavour flavour(
          orig_attk.flavour == AF_KLOWN ? AF_KLOWN : attk.flavour);
		switch (flavour)
		{
        case AF_ACID:
		    monsterattacks += "(acid)";
			break;
        case AF_BLINK:
		    monsterattacks += "(blink)";
			break;
        case AF_COLD:
		    monsterattacks += "(cold)";
			break;
        case AF_CONFUSE:
		    monsterattacks += "(confuse)";
			break;
        case AF_DISEASE:
		    monsterattacks += "(disease)";
			break;
        case AF_DRAIN_DEX:
		    monsterattacks += "(drain dexterity)";
			break;
        case AF_DRAIN_STR:
		    monsterattacks += "(drain strength)";
			break;
        case AF_DRAIN_XP:
		    monsterattacks += "(drain)";
			break;
        case AF_CHAOS:
            monsterattacks += "(chaos)";
        case AF_ELEC:
		    monsterattacks += "(elec)";
			break;
        case AF_FIRE:
		    monsterattacks += "(fire)";
			break;
        case AF_HUNGER:
		    monsterattacks += "(hunger)";
			break;
        case AF_MUTATE:
		    monsterattacks += "(mutation)";
			break;
        case AF_PARALYSE:
		    monsterattacks += "(paralyse)";
			break;
        case AF_POISON:
		    monsterattacks += "(poison)";
			break;
        case AF_POISON_NASTY:
		    monsterattacks += "(nasty poison)";
			break;
        case AF_POISON_MEDIUM:
		    monsterattacks += "(medium poison)";
			break;
        case AF_POISON_STRONG:
		    monsterattacks += "(strong poison)";
			break;
        case AF_POISON_STR:
		    monsterattacks += "(poison, drain strength)";
			break;
        case AF_ROT:
		    monsterattacks += "(rot)";
			break;
        case AF_VAMPIRIC:
		    monsterattacks += "(vampiric)";
			break;
        case AF_KLOWN:
		    monsterattacks += "(klown)";
			break;
        case AF_DISTORT:
		    monsterattacks += "(distort)";
			break;
        case AF_RAGE:
		    monsterattacks += "(rage)";
			break;
        case AF_PLAIN:
        default:
			break;
		}

		if (mon.has_hydra_multi_attack())
		  monsterattacks += " per head";
	  }
      if (mon.has_hydra_multi_attack())
        break;
	}

	printf("%s", monsterattacks.c_str());

	switch (me->holiness)
	{
    case MH_HOLY:
      mons_flag(monsterflags, "holy");
      break;
    case MH_UNDEAD:
      mons_flag(monsterflags, "undead");
      break;
    case MH_DEMONIC:
      mons_flag(monsterflags, "demonic");
      break;
    case MH_NONLIVING:
      mons_flag(monsterflags, "non-living");
      break;
    case MH_PLANT:
      mons_flag(monsterflags, "plant");
      break;
    case MH_NATURAL:
    default:
      break;
	}

    mons_check_flag(me->habitat == HT_AMPHIBIOUS_WATER ||
                    me->habitat == HT_AMPHIBIOUS_LAND,
                    monsterflags, "amphibious");

    mons_check_flag(me->bitfields & M_EVIL, monsterflags, "evil");
    mons_check_flag((me->bitfields & M_SPELLCASTER)
                    && (me->bitfields & M_ACTUAL_SPELLS),
                    monsterflags, "spellcaster");
    mons_check_flag(me->bitfields & M_COLD_BLOOD, monsterflags, "cold-blooded");
    mons_check_flag(me->bitfields & M_SENSE_INVIS, monsterflags,
                    "sense invisible");
    mons_check_flag(me->bitfields & M_SEE_INVIS, monsterflags, "see invisible");
    mons_check_flag(me->bitfields & M_LEVITATE, monsterflags, "lev");
    mons_check_flag(me->bitfields & M_FLIES, monsterflags, "fly");

	printf("%s", monsterflags.c_str());

	if (me->resist_magic == 5000)
	{
	    if (monsterresistances.empty())
	      monsterresistances = " | Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "magic(immune)";
	}
	else if (me->resist_magic < 0)
	{
	    if (monsterresistances.empty())
	      monsterresistances = " | Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "magic("
		  + to_string((short int) me->hpdice[0] * me->resist_magic * 4 / 3 * -1)
		  + ")";
	}
	else if (me->resist_magic > 0)
	{
	    if (monsterresistances.empty())
	      monsterresistances = " | Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "magic("
		  + to_string((short int) me->resist_magic)
		  + ")";
	}

    const mon_resist_def res(
        shapeshifter? me->resists : get_mons_resists(&mon));
#define res(x) \
    do                                          \
    {                                           \
        record_resist(#x,                             \
                      monsterresistances,             \
                      monstervulnerabilities,         \
                      res.x);                         \
    } while (false)                                   \


    res(hellfire);
    if (me->resists.hellfire <= 0)
        res(fire);
    res(cold);
    res(elec);
    res(poison);
    res(acid);
    res(asphyx);
    res(pierce);
    res(slice);
    res(bludgeon);

	printf("%s", monsterresistances.c_str());
	printf("%s", monstervulnerabilities.c_str());

	if (me->weight != 0 && me->corpse_thingy != CE_NOCORPSE && me->corpse_thingy != CE_CLEAN)
	{
	  printf(" | Chunks: ");
	  switch (me->corpse_thingy)
	  {
		case CE_CONTAMINATED:
		  printf("contaminated");
		  break;
		case CE_POISONOUS:
		  printf("poisonous");
		  break;
		case CE_HCL:
		  printf("hydrochloric acid");
		  break;
		case CE_MUTAGEN_RANDOM:
		  printf("mutagenic");
		break;
		default:
		  printf("clean/none/unknown");
		  break;
	  }
	}

    printf(" | XP: %ld", exper);

    if (!spell_sets.empty()) {
      printf(" | Sp: ");

      if (shapeshifter || mon.type == MONS_PANDEMONIUM_DEMON)
        printf("(random)");
      else {
        bool first = true;
        for (std::set<std::string>::const_iterator i = spell_sets.begin();
             i != spell_sets.end(); ++i)
        {
          if (!first)
            printf(" / ");
          first = false;
          printf("%s", i->c_str());
        }
      }
    }

    printf(".\n");

    return 0;
  }
  return 1;
}

template <class T> inline std::string to_string (const T& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

//////////////////////////////////////////////////////////////////////////
// acr.cc stuff

CLua clua(true);
CLua dlua(false);      // Lua interpreter for the dungeon builder.
crawl_environment env; // Requires dlua.
player you;
system_environment SysEnv;
game_state crawl_state;

FILE *yyin;
int yylineno;

std::string init_file_error;    // externed in newgame.cc

char info[ INFO_SIZE ];         // messaging queue extern'd everywhere {dlb}

int stealth;                    // externed in view.cc

void process_command(command_type) {
}

int yyparse() {
  return 0;
}

void world_reacts() {
}
