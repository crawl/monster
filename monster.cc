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

#include "mon-util.h"
#include "view.h"
#include <sstream>

static FixedVector < int, NUM_MONSTERS > mon_entry;

static monsterentry mondata[] = {
#include "mon-data.h"
};

#define MONDATASIZE ARRAYSZ(mondata)

void init_my_monsters();
monsterentry *get_monster_data_by_id(int monsterid);
int get_monster_id_by_name(std::string monstername);


std::string &lowercase(std::string &s);
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

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
	printf("Usage: @? <monster name>\n");
	exit(0);
  }

  std::string target = argv[1];

  if (argc > 2)
    for (int x = 2; x < argc; x++)
	{
	  target.append(" ");
	  target.append(argv[x]);
	}

  init_my_monsters();

  monsterentry *me = get_monster_data_by_id(get_monster_id_by_name(target));

  if (me)
  {
    std::string monstername = uppercase_first(me->name);
	std::string monsterflags;
	std::string monsterresistances;
	std::string monstervulnerabilities;
	std::string monsterattacks;

    printf("%s", monstername.c_str());

    printf(" | Speed: %i", me->speed);

    const int hd = me->hpdice[0];
    printf(" | HD: %d", hd);

    printf(" | Health: ");
    const int hplow = hd * me->hpdice[1] + me->hpdice[3];
    const int hphigh =
        hd * (me->hpdice[1] + me->hpdice[2]) + me->hpdice[3];
    if (hplow < hphigh)
        printf("%i-%i", hplow, hphigh);
    else
        printf("%i", hplow);

    printf(" | AC/EV: %i/%i", me->AC, me->ev);

    for (int x = 0; x < 4; x++)
	{
	  if (me->attack[x].type)
      {
	    if (monsterattacks.empty())
		  monsterattacks = " | Damage: ";
	    else
	      monsterattacks += ", ";
		monsterattacks += to_string((short int) me->attack[x].damage);

		if (!strcmp(me->name, "hydra"))
		  monsterattacks += " per head";

		switch (me->attack[x].flavour)
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
		  case AF_BAD_MUTATE:
		    monsterattacks += "(bad mutation)";
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
	  }
	}

	printf("%s", monsterattacks.c_str());

	switch (me->holiness)
	{
	  case MH_HOLY:
	    if (monsterflags.empty())
		  monsterflags = " | Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "holy";
	  break;
	  case MH_UNDEAD:
	    if (monsterflags.empty())
		  monsterflags = " | Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "undead";
	  break;
	  case MH_DEMONIC:
	    if (monsterflags.empty())
		  monsterflags = " | Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "demonic";
	  break;
	  case MH_NONLIVING:
	    if (monsterflags.empty())
		  monsterflags = " | Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "non-living";
	  break;
	  case MH_PLANT:
	    if (monsterflags.empty())
		  monsterflags = " | Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "plant";
	  break;
	  case MH_NATURAL:
	  default:
		break;
	}

	if (me->bitfields & M_EVIL)
	{
	  if (monsterflags.empty())
	    monsterflags = " | Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "evil";
	}

	if (me->bitfields & M_SPELLCASTER)
	{
	  if (monsterflags.empty())
	    monsterflags = " | Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "spellcaster";
	}

	if (me->bitfields & M_COLD_BLOOD)
	{
	  if (monsterflags.empty())
	    monsterflags = " | Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "cold-blooded";
	}

	if (me->bitfields & M_SENSE_INVIS)
	{
	  if (monsterflags.empty())
	    monsterflags = " | Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "sense invisible";
	}

	if (me->bitfields & M_SEE_INVIS)
	{
	  if (monsterflags.empty())
	    monsterflags = " | Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "see invisible";
	}

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

#define res(x) \
    do                                          \
    {                                           \
        record_resist(#x,                             \
                      monsterresistances,             \
                      monstervulnerabilities,         \
                      me->resists.x);                 \
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

    printf(".\n");

    return 0;
  }

  printf("No monster with such name \"%s\"...\n", target.c_str());

  return 1;
}

void init_my_monsters()
{
  unsigned int x;

  mon_entry.init(-1);

  for (x = 0; x < MONDATASIZE; x++)
    mon_entry[mondata[x].mc] = x;

  for (x = 0; x < NUM_MONSTERS; x++)
    if (mon_entry[x] == -1)
      mon_entry[x] = mon_entry[MONS_PROGRAM_BUG];
}

monsterentry *get_monster_data_by_id(int monsterid)
{
  const int me = monsterid != -1? mon_entry[monsterid] : -1;

  if (me >= 0)
    return (&mondata[me]);
  else
    return (NULL);
}

int get_monster_id_by_name(std::string monstername)
{
  lowercase(monstername);

  for (unsigned int x = 0; x < MONDATASIZE; x++)
  {
    std::string match = mondata[x].name;
	lowercase(match);

    if (monstername == match)
      return mondata[x].mc;

    continue;
  }

  return (-1);
}

std::string &lowercase(std::string &s)
{
    for (unsigned i = 0, sz = s.size(); i < sz; ++i)
        s[i] = tolower(s[i]);
    return (s);
}

template <class T> inline std::string to_string (const T& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

// Unfortunate duplication from mon-util.cc, but otherwise we'll have
// to link in the entire Crawl codebase:

mon_resist_def::mon_resist_def()
    : elec(0), poison(0), fire(0), steam(0), cold(0), hellfire(0),
      asphyx(0), acid(0), sticky_flame(false), pierce(0),
      slice(0), bludgeon(0)
{
}

short mon_resist_def::get_default_res_level(int resist, short level) const
{
    if (level != -100)
        return level;
    switch (resist)
    {
    case MR_RES_STEAM:
    case MR_RES_ACID:
        return 3;
    case MR_RES_ELEC:
        return 2;
    default:
        return 1;
    }
}

mon_resist_def::mon_resist_def(int flags, short level)
    : elec(0), poison(0), fire(0), steam(0), cold(0), hellfire(0),
      asphyx(0), acid(0), sticky_flame(false), pierce(0),
      slice(0), bludgeon(0)
{
    for (int i = 0; i < 32; ++i)
    {
        const short nl = get_default_res_level(1 << i, level);
        switch (flags & (1 << i))
        {
        // resistances
        case MR_RES_STEAM:    steam    =  3; break;
        case MR_RES_ELEC:     elec     = nl; break;
        case MR_RES_POISON:   poison   = nl; break;
        case MR_RES_FIRE:     fire     = nl; break;
        case MR_RES_HELLFIRE: hellfire = nl; break;
        case MR_RES_COLD:     cold     = nl; break;
        case MR_RES_ASPHYX:   asphyx   = nl; break;
        case MR_RES_ACID:     acid     = nl; break;

        // vulnerabilities
        case MR_VUL_ELEC:     elec     = -nl; break;
        case MR_VUL_POISON:   poison   = -nl; break;
        case MR_VUL_FIRE:     fire     = -nl; break;
        case MR_VUL_COLD:     cold     = -nl; break;

        // resistance to certain damage types
        case MR_RES_PIERCE:   pierce   = nl; break;
        case MR_RES_SLICE:    slice    = nl; break;
        case MR_RES_BLUDGEON: bludgeon = nl; break;

        // vulnerability to certain damage types
        case MR_VUL_PIERCE:   pierce   = -nl; break;
        case MR_VUL_SLICE:    slice    = -nl; break;
        case MR_VUL_BLUDGEON: bludgeon = -nl; break;

        case MR_RES_STICKY_FLAME: sticky_flame = true; break;

        default: break;
        }
    }
}

const mon_resist_def &mon_resist_def::operator |= (const mon_resist_def &o)
{
    elec        += o.elec;
    poison      += o.poison;
    fire        += o.fire;
    cold        += o.cold;
    hellfire    += o.hellfire;
    asphyx      += o.asphyx;
    acid        += o.acid;
    pierce      += o.pierce;
    slice       += o.slice;
    bludgeon    += o.bludgeon;
    sticky_flame = sticky_flame || o.sticky_flame;
    return (*this);
}

mon_resist_def mon_resist_def::operator | (const mon_resist_def &o) const
{
    mon_resist_def c(*this);
    return (c |= o);
}
