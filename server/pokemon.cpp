#include"pokemon.h"
#include"define.h"
#include"string"
using namespace std;
//construct
Pokemon::Pokemon(int race, int element, int type, string name)
	:type(type),
	element(element),
	race(race),
	name(name),
	baseAttack(10),
	baseDefense(5),
	baseHp(25),
	baseAtkInterval(10),
	level(1),
	exp(0)
{	
	//diffrent type get different value
	switch (type)
	{
	case ATK:
		baseAttack += 5;
		break;
	case DEF:
		baseDefense += 3;
		break;
	case HP:
		baseHp += 10;
		break;
	case SPE:
		baseAtkInterval += 3;
		break;
	default:
		break;
	}
	//make some random change
	baseAttack += GetRandomNumber(3,1);
	baseDefense += GetRandomNumber(2,1);
	baseHp += GetRandomNumber(5,1);
	baseAtkInterval += GetRandomNumber(1,1);
	//output info
	cout << "Init " << GetName() << " from " << GetRace() << endl
		<< "Type: " << GetType() << endl
		<< "Atk: " << GetBaseAtk() << endl
		<< "Def: " << GetBaseDef() << endl
		<< "MaxHp: " << GetBaseHp() << endl
		<< "Speed: " << GetBaseInterval() << endl
		<< "LV: " << GetLevel() << endl
		<< "Exp: " << GetExp() << endl;
}
//construct during battle
Pokemon::Pokemon(int race, int element, int type, int atk, int hp, int def, int speed, int level, int exp, string name)
	:type(type),
	element(element),
	race(race),
	name(name),
	baseAttack(atk),
	baseDefense(def),
	baseHp(hp),
	baseAtkInterval(speed),
	level(level),
	exp(exp)
{

}
#pragma region change current value during battle
//take damage
bool Pokemon::TakeDamage(int damage)
{
	return ChangeHp(-damage);
}
//change current attack
void Pokemon::ChangeAtk(int count)
{
	curAtk += count;
	if (curAtk < 1)
	{
		curAtk = 1;
	}
	if (count > 0)
	{
		cout << name << "'s Attack +" << count << endl;
	}
	else
	{
		cout << name << "'s Attack " << count << endl;
	}

	cout << name << "'s Attack becomes " << curAtk << endl;
}
//change current defence
void Pokemon::ChangeDef(int count)
{
	curDef += count;
	if (curDef < 1)
	{
		curDef = 1;
	}
	if (count > 0)
	{
		cout << name << "'s Defence +" << count << endl;
	}
	else
	{
		cout << name << "'s Defence " << count << endl;
	}
	cout << name << "'s Defence becomes " << curDef << endl;
}
//change current attack interval
void Pokemon::ChangeInte(int count)
{
	curInte += count;
	if (curInte < 1)
	{
		curInte = 1;
	}
	if (count > 0)
	{
		cout << name << "'s Speed +" << count << endl;
	}
	else
	{
		cout << name << "'s Speed " << count << endl;
	}
	cout << name << "'s Speed becomes " << curInte << endl;
}
//change current engergy
void Pokemon::ChangeEner(int count)
{
	curEner += count;
	/*if (count > 0)
	{
		cout << name << "'s Energy +" << count << endl;
	}
	else
	{
		cout << name << "'s Energy " << count << endl;
	}
	cout << name << "'s Energy becomes " << curEner << endl;*/
}
//change current hp and judge whether die
bool Pokemon::ChangeHp(int count)
{
	curHp += count;

	if (curHp > baseHp)//current hp cant bigger than base hp
	{
		curHp = baseHp;
	}
	if (curHp < 0)
	{
		curHp = 0;
	}
	if (count > 0)
	{
		cout << name << " restores " << count << "HP!\n";
	}
	else
	{
		cout << name << " takes " << -count << " damage!\n";
	}
	if (!curHp)//die
	{
		cout << name << " is defeated!\n\n";
		return true;
	}
	else//change hp
	{
		cout << name << "'s HP becomes " << curHp << endl
			<< endl;
	}
	return false;
}
#pragma endregion
//get exp and judge level up
void Pokemon::GainExp(int e)
{
	if (level == 15)// if level==15 its full
	{
		return;
	}
	exp += e;
	cout << name << " get " << e << " exp" << endl;
	cout << name << " has " << exp << " exp" << endl;
	while (level<15 && exp>= expValue[level])
	{
		level++;
		cout << "Level UP" << endl;
		cout << name << " is now Level " << level << endl;
		LevelUp();
	}
	return;
}
//level up
void Pokemon::LevelUp()
{
	int currAtk = 4 + GetRandomNumber(1,1);
	int currDef = 2 + GetRandomNumber(1,1);
	int currHp = 8 + GetRandomNumber(2,1);
	int currInte = 1 + GetRandomNumber(1,1);
	switch (type)
	{
	case ATK:
		currAtk += 5;
		break;
	case HP:
		currHp += 5;
		break;
	case DEF:
		currDef += 2;
		break;
	case SPE:
		currInte += 1;
		break;
	default:
		break;
	}
	baseAttack += currAtk;
	baseDefense += currDef;
	baseAtkInterval += currInte;
	baseHp += currHp;
	cout << "Atk: " << baseAttack - currAtk << "->" << baseAttack << endl;
	cout << "Def: " << baseDefense - currDef << "->" << baseDefense << endl;
	cout << "MaxHP: " << baseHp - currHp << "->" << baseHp << endl;
	cout << "Speed: " << baseAtkInterval - currInte << "->" << baseAtkInterval << endl;
}
//get a random number 
//op == 0 get a number between 0~n
//op == 1 get a number between -n~n
int Pokemon::GetRandomNumber(int n, int op)
{
	if (!op)
	{
		return rand() % n;
	}
	else
	{
		return rand() % (2 * n + 1) - n;
	}
}
//get a random skill
int Pokemon::ChooseSkill()
{
	int skillIndex = 0;
	bool usable[3];
	int usableCount = 1;
	//make skil which energy less than current energy usable
	for (int i = 1; i <= 3; i++)
	{
		if (level >= i * 3 && curEner >= energy[i])
		{
			usable[i] = true;
			usableCount++;
		}
	}
	//choose a random skill
	int use = rand() % usableCount;
	ChangeEner(-energy[use]);
	return use;
}
//initialize the pokemon during battle
void Pokemon::RestoreAll()
{
	curHp = baseHp;
	curAtk = baseAttack;
	curDef = baseDefense;
	curInte = baseAtkInterval;
	curEner = 0;
	cout << "NOW RESTORE ALL" << endl;
}
//judge dodge
bool Pokemon::dodge(int attacker, int aim,string &msg)
{
	if (attacker > aim)
	{
		msg += "0 "; return false;
	}
	int seed = 5 * (aim-attacker);
	if (GetRandomNumber(100,0)<seed)
	{
		cout << "Miss!\n";
		msg += "1 ";
		return true;
	}
	msg += "0 ";
	return false;
}
//judge critical
int Pokemon::critical(int attacker, int aim)
{
	if (attacker < aim)return 1;
	int seed = 5 * (attacker - aim);
	if (GetRandomNumber(100, 0) < seed)
	{
		cout << "CRITICAL!\n";
		return 2;
	}
	return 1;
}
//judge damage by two pokemon's attribute
float Pokemon::JudgeAttribute(int ele)
{
	switch (attributeRelation[element][ele])
	{
	case 1:
		return 1;
		break;
	case 2:
		return 0.5;
		break;
	case 3:
		return 2;
		break;
	default:
		return 1;
		break;
	}
}
/**
 *return attack information and let client draw it
 *format:<playerRound: 0 | 1> 0:offline 1:online
 *<skillName> 
 *<dodge: 0 | 1>0:not dodge 1:dodge
 *<defenderHP> <defenderAtk: 0 | 1 | 2> <defenderDef: 0 | 1 | 2> <defenderSpeed: 0 | 1 | 2> 0:down 1:not change 2:up
 *<attackerHP> <attackerAtk: 0 | 1 | 2> <attackerDef: 0 | 1 | 2> <attackerSpeed: 0 | 1 | 2> 0:down 1:not change 2:up
 *<yourpower>
*/
#pragma region attack
bool Charmander::attack(Pokemon& aim, string& msg, int skillIndex = 0, bool autoFight=true)
{
	if (autoFight)
	{
		skillIndex = ChooseSkill();
		cout << GetName() << " uses " << GetSkillName(skillIndex) << endl;
	}
	ChangeEner(-1 * energy[skillIndex]);
	switch (skillIndex)
	{
	case 1: //spark
	{
		if (dodge(GetCurInte(), aim.GetCurInte(),msg)) return false;
		int dmg = GetCurAtk() + GetLevel() * 2 - aim.GetCurDef() / 2 + GetRandomNumber(4,0) * critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;
		break;
	}
	case 2: //rage
		msg += "0 ";
		ChangeAtk(GetCurAtk() / 8);
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 2 1 1 ";
		break;
	case 3: //fireball
	{
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		int dmg = GetCurAtk() * 1.5 - aim.GetCurDef() + 8 + GetRandomNumber(4 + GetLevel(),0) * (int)critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;
		break;
	}
	default:
	{
		//simple attack
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		int dmg = GetCurAtk() - aim.GetCurDef() + GetRandomNumber(4,1) * critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;
		break;
	}
	} //switch
	return false;
}
bool Bulbasaur::attack(Pokemon& aim, string& msg, int skillIndex = 0, bool autoFight = true)
{
	if (autoFight)
	{
		skillIndex = ChooseSkill();
		cout << GetName() << " uses " << GetSkillName(skillIndex) << endl;
	}
	ChangeEner(-1 * energy[skillIndex]);
	switch (skillIndex)
	{
	case 1: //photosynthesis
	{
		msg += "0 ";
		ChangeHp(GetCurAtk() / 2 + GetCurDef() + GetRandomNumber(4,1));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";

		break;
	}
	case 2: //life drain
	{
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		int dmg = GetCurAtk() + GetRandomNumber(4 + GetLevel(),0) * critical(GetCurAtk(), aim.GetCurDef());
		ChangeHp(dmg);
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;
		break;
	}
	case 3: //razor leaf
	{
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		int dmg = GetCurAtk() * 2 - aim.GetCurDef() + GetRandomNumber(3 + GetLevel(),0) * critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;

		break;
	}
	default:
	{
		//simple attack
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		int dmg = GetCurAtk() - aim.GetCurDef() + GetRandomNumber(4,0) * critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;
		break;
	}
	} //switch
	return false;
}
bool Squirtle::attack(Pokemon& aim, string& msg, int skillIndex = 0, bool autoFight = true)
{
	if (autoFight)
	{
		skillIndex = ChooseSkill();
		cout << GetName() << " uses " << GetSkillName(skillIndex) << endl;
	}
	ChangeEner(-1 * energy[skillIndex]);
	switch (skillIndex)
	{
	case 1: //iron defence
	{
		msg += "0 ";
		ChangeDef(2);
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 2 1 ";
		break;
	}
	case 2: //water pulse
	{
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		ChangeAtk(2);
		int dmg = GetCurAtk() - aim.GetCurDef() + GetRandomNumber(4 + GetLevel(),0) * critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 2 1 1 ";
		return result;
		break;
	}
	case 3: //hydro pump
	{
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		int dmg = GetCurAtk() * 2 - aim.GetCurDef() + GetRandomNumber(3 + GetLevel(),0) * critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;
		break;
	}
	default:
	{
		//simple attack
		if (dodge(GetCurInte(), aim.GetCurInte(), msg)) return false;
		int dmg = GetCurAtk() - aim.GetCurDef() + GetRandomNumber(4,0) * critical(GetCurAtk(), aim.GetCurDef());
		bool result = aim.TakeDamage(dmg * JudgeAttribute(aim.GetElement()));
		msg += to_string(aim.GetCurHp()) + " 1 1 1 ";
		msg += to_string(GetCurHp()) + " 1 1 1 ";
		return result;
		break;
	}
	} //switch
	return false;
}
#pragma endregion