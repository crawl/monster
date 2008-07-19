#include "mon-util.h"
#include "view.h"
#include <sstream>

static FixedVector < int, NUM_MONSTERS > mon_entry;

static monsterentry mondata[] = {
#include "mon-data.h"
};

#define MONDATASIZE ARRAYSIZE(mondata)

void init_my_monsters();
monsterentry *get_monster_data_by_id(int monsterid);
int get_monster_id_by_name(std::string monstername);


std::string &lowercase(std::string &s);
std::string uppercase_first(std::string s);

template <class T> inline std::string to_string (const T& t);

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

    printf(" - Speed: %i", me->speed);
	
	printf(" - Health: %i-%i",
	  me->hpdice[0] * me->hpdice[1] + me->hpdice[3],
	  me->hpdice[0] * (me->hpdice[1] + me->hpdice[2]) + me->hpdice[3]);

    printf(" - AC/EV: %i/%i", me->AC, me->ev);

    for (int x = 0; x < 4; x++)
	{
	  if (me->attack[x].type)
      {
	    if (monsterattacks.empty())
		  monsterattacks = " - Damage: ";
	    else
	      monsterattacks += ", ";
		monsterattacks += to_string((short int) me->attack[x].damage);

		if (me->name == "hydra")
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
		  monsterflags = " - Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "holy";
	  break;
	  case MH_UNDEAD:
	    if (monsterflags.empty())
		  monsterflags = " - Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "undead";
	  break;
	  case MH_DEMONIC:
	    if (monsterflags.empty())
		  monsterflags = " - Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "demonic";
	  break;
	  case MH_NONLIVING:
	    if (monsterflags.empty())
		  monsterflags = " - Flags: ";
	    else
	      monsterflags += ", ";
		monsterflags += "non-living";
	  break;
	  case MH_PLANT:
	    if (monsterflags.empty())
		  monsterflags = " - Flags: ";
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
	    monsterflags = " - Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "evil";
	}

	if (me->bitfields & M_SPELLCASTER)
	{
	  if (monsterflags.empty())
	    monsterflags = " - Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "spellcaster";
	}

	if (me->bitfields & M_COLD_BLOOD)
	{
	  if (monsterflags.empty())
	    monsterflags = " - Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "cold-blooded";
	}

	if (me->bitfields & M_SENSE_INVIS)
	{
	  if (monsterflags.empty())
	    monsterflags = " - Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "sense invisible";
	}

	if (me->bitfields & M_SEE_INVIS)
	{
	  if (monsterflags.empty())
	    monsterflags = " - Flags: ";
	  else
	    monsterflags += ", ";
	  monsterflags += "see invisible";
	}

	printf("%s", monsterflags.c_str());

	if (me->resist_magic == 5000)
	{
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "magic(immune)";
	}
	else if (me->resist_magic < 0)
	{
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "magic("
		  + to_string((short int) me->hpdice[0] * me->resist_magic * 4 / 3 * -1)
		  + ")";
	}
	else if (me->resist_magic > 0)
	{
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "magic("
		  + to_string((short int) me->resist_magic)
		  + ")";
	}

	if (me->resists)
	{
	  if (me->resists & MR_RES_HELLFIRE)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "hellfire";
	  }
	  else
	    if (me->resists & MR_RES_FIRE)
	    {
	      if (monsterresistances.empty())
	        monsterresistances = " - Res: ";
	      else
	        monsterresistances += ", ";
		  monsterresistances += "fire";
	    }
	  if (me->resists & MR_RES_COLD)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "cold";
	  }
	  if (me->resists & MR_RES_ELEC)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "electricity";
	  }
	  if (me->resists & MR_RES_POISON)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "poison";
	  }
	  if (me->resists & MR_RES_ACID)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "acid";
	  }
	  if (me->resists & MR_RES_ASPHYX)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "asphyx";
	  }
	  if (me->resists & MR_RES_PIERCE)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "pierce";
	  }
	  if (me->resists & MR_RES_SLICE)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "slice";
	  }
	  if (me->resists & MR_RES_BLUDGEON)
	  {
	    if (monsterresistances.empty())
	      monsterresistances = " - Res: ";
	    else
	      monsterresistances += ", ";
		monsterresistances += "bludgeon";
	  }

	  if (me->resists & MR_VUL_FIRE)
	  {
	    if (monstervulnerabilities.empty())
	      monstervulnerabilities = " - Vul: ";
	    else
	      monstervulnerabilities += ", ";
	    monstervulnerabilities += "fire";
	  }
	  if (me->resists & MR_VUL_COLD)
	  {
	    if (monstervulnerabilities.empty())
	      monstervulnerabilities = " - Vul: ";
	    else
	      monstervulnerabilities += ", ";
	    monstervulnerabilities += "cold";
	  }
	  if (me->resists & MR_VUL_ELEC)
	  {
	    if (monstervulnerabilities.empty())
	      monstervulnerabilities = " - Vul: ";
	    else
	      monstervulnerabilities += ", ";
	    monstervulnerabilities += "electricity";
	  }
	  if (me->resists & MR_VUL_POISON)
	  {
	    if (monstervulnerabilities.empty())
	      monstervulnerabilities = " - Vul: ";
	    else
	      monstervulnerabilities += ", ";
	    monstervulnerabilities += "poison";
	  }
	  if (me->resists & MR_VUL_PIERCE)
	  {
	    if (monstervulnerabilities.empty())
	      monstervulnerabilities = " - Vul: ";
	    else
	      monstervulnerabilities += ", ";
	    monstervulnerabilities += "pierce";
	  }
	  if (me->resists & MR_VUL_SLICE)
	  {
	    if (monstervulnerabilities.empty())
	      monstervulnerabilities = " - Vul: ";
	    else
	      monstervulnerabilities += ", ";
	    monstervulnerabilities += "slice";
	  }
	  if (me->resists & MR_VUL_BLUDGEON)
	  {
	    if (monstervulnerabilities.empty())
	      monstervulnerabilities = " - Vul: ";
	    else
	      monstervulnerabilities += ", ";
	    monstervulnerabilities += "bludgeon";
	  }

    }
	printf("%s", monsterresistances.c_str());
	printf("%s", monstervulnerabilities.c_str());

	if (me->weight != 0 && me->corpse_thingy != CE_NOCORPSE && me->corpse_thingy != CE_CLEAN)
	{
	  printf(" - Chunks: ");
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

std::string uppercase_first(std::string s)
{
    if (s.length())
        s[0] = toupper(s[0]);
    return (s);
}

template <class T> inline std::string to_string (const T& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

