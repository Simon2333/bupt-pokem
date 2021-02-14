#pragma once
#include<cstdio>
#include<cstring>
#include<iostream>
#include"define.h"
using namespace std;
//exp needed when level up
const int expValue[15] = { 0, 100, 250, 500, 800, 1200, 1800, 2500,
							3300, 4500, 6000, 7000, 8000, 9000, 10000 };
// diferrent attribute cause different damage
const int attributeRelation[6][6] = { {2,3,1,2,2},{2,2,3,3,2},{1,1,2,1,1},{3,2,2,2,3},{3,1,1,1,2} };
//skill cost
const int energy[4] = { 1,10,15,20 };
class Pokemon
{
private:
	//basic
	string name;
	string skillName[4];
	string skillDisc[4];
	int race;
	int element;
	int type;
	int exp;
	int level;
	int baseAttack;
	int baseDefense;
	int baseHp;
	int baseAtkInterval;
	//during battle
	int curHp;
	int curAtk;
	int curDef;
	int curInte;
	int curEner;
public:
	//two construct
	Pokemon(int race, int element, int type, string name);
	Pokemon(int race, int element, int type, int atk, int hp, int def, int speed, int level, int exp, string name);
	//return basic information
	int GetRace()const { return race; }
	int GetType()const { return type; }
	int GetElement()const{ return element; }
	int GetLevel()const { return level; }
	int GetExp()const { return exp; }
	int GetBaseAtk()const { return baseAttack; }
	int GetBaseDef()const { return baseDefense; }
	int GetBaseHp()const { return baseHp; }
	int GetBaseInterval()const { return baseAtkInterval; }
	string GetName()const { return name; }
	virtual string GetSkillName(int n)const { return skillName[n]; }
	virtual string GetSkillDisc(int n)const { return skillDisc[n]; }
	//during battle
	int GetCurAtk()const { return curAtk; }
	int GetCurDef()const { return curDef; }
	int GetCurInte()const { return curInte; }
	int GetCurEner()const { return curEner; }
	int GetCurHp()const { return curHp; }

	//about level and exp
	void GainExp(int e);
	void LevelUp();

	//get random number
	//op=0:get 0~n
	//op=1:get -n~n
	int GetRandomNumber(int n, int op);

	//fight
	void RestoreAll();
	virtual bool attack(Pokemon& aim, string& msg, int skillIndex = 0, bool autoFight = true) { return true; };
	int ChooseSkill();
	//return current information
	bool TakeDamage(int damage);
	bool ChangeHp(int count);
	void ChangeDef(int count);
	void ChangeInte(int count);
	void ChangeAtk(int count);
	void ChangeEner(int count);
	float JudgeAttribute(int ele);

	//MISS 5*difference of attack interval/100
	bool dodge(int attacker, int aim, string &msg);

	//CIRTICAL 5*diffence of attack and defence/100
	int critical(int attacker, int aim);
};
//½ÜÄá¹ê
class Squirtle :public Pokemon
{
private:
	string raceName;
public:
	Squirtle(const string& name = "")
		:Pokemon(SQUIRTLE,WATER,DEF,name)
	{
		skillName[0] = "kick";
		skillName[1] = "iron defence";
		skillName[2] = "water pulse";
		skillName[3] = "hydro pump";
		skillDisc[0] = "simple attack";
		skillDisc[1] = "improve defence";
		skillDisc[2] = "improve attack then cause damage";
		skillDisc[3] = "cause huge damage";

	}
	Squirtle(int atk, int hp, int def, int speed, int level, int exp, const string& name = "")
		:Pokemon(SQUIRTLE, WATER, DEF, atk, hp, def, speed, level, exp, name)
	{
		skillName[0] = "kick";
		skillName[1] = "iron defence";
		skillName[2] = "water pulse";
		skillName[3] = "hydro pump";
		skillDisc[0] = "simple attack";
		skillDisc[1] = "improve defence";
		skillDisc[2] = "improve attack then cause damage";
		skillDisc[3] = "cause huge damage";

	}
	string skillName[4];
	string skillDisc[4];
	//rewrite virtual function
	string GetSkillName(int n)const { return skillName[n]; }
	string GetSkillDisc(int n)const { return skillDisc[n]; }
	bool attack(Pokemon& aim, string& msg, int skillIndex, bool autoFight);
};
//Ð¡»ðÁú
class Charmander :public Pokemon
{
private:
	string raceName;

public:
	Charmander(const string& name = "")
		:Pokemon(CHARMANDER, FIRE, ATK, name)
	{
		raceName = "Charmander";
		skillName[0] = "kick";
		skillName[1] = "spark";
		skillName[2] = "rage";
		skillName[3] = "fireball";
		skillDisc[0] = "simple attack";
		skillDisc[1] = "ignore opponent's half defence";
		skillDisc[2] = "increase attack";
		skillDisc[3] = "cause huge damage";

	}
	Charmander(int atk,int hp,int def,int speed, int level,int exp,const string& name = "")
		:Pokemon(CHARMANDER, FIRE, ATK, atk,hp,def,speed,level,exp,name)
	{
		raceName = "Charmander";
		skillName[0] = "kick";
		skillName[1] = "spark";
		skillName[2] = "rage";
		skillName[3] = "fireball";
		skillDisc[0] = "simple attack";
		skillDisc[1] = "ignore opponent's half defence";
		skillDisc[2] = "increase attack";
		skillDisc[3] = "cause huge damage";

	}
	string skillName[4];
	string skillDisc[4];
	//rewrite virtual function
	bool attack(Pokemon& aim, string& msg, int skillIndex, bool autoFight);
	string GetSkillName(int n)const { return skillName[n]; }
	string GetSkillDisc(int n)const { return skillDisc[n]; }
};
//ÃîÍÜÖÖ×Ó
class Bulbasaur :public Pokemon
{
private:
	string raceName;
public:
	Bulbasaur(const string& name = "")
		:Pokemon(BULBASAUR,GRASS,SPE,name)
	{
		skillName[0] = "kick";
		skillName[1] = "photosynthesis";
		skillName[2] = "life drain";
		skillName[3] = "razor leaf";
		skillDisc[0] = "simple attack";
		skillDisc[1] = "restore HP";
		skillDisc[2] = "cause damage and restore HP, ignore defence";
		skillDisc[3] = "cause huge damage";
		cout << "Skills:\n";
		for (int i = 0; i < 4; ++i)
		{
			cout << "	Name: " << skillName[i] << endl;
			cout << "	Description: " << skillDisc[i] << endl;
		}
	}
	Bulbasaur(int atk, int hp, int def, int speed, int level, int exp, const string& name = "")
		:Pokemon(BULBASAUR, GRASS, SPE, atk, hp, def, speed, level, exp, name)
	{
		skillName[0] = "kick";
		skillName[1] = "photosynthesis";
		skillName[2] = "life drain";
		skillName[3] = "razor leaf";
		skillDisc[0] = "simple attack";
		skillDisc[1] = "restore HP";
		skillDisc[2] = "cause damage and restore HP, ignore defence";
		skillDisc[3] = "cause huge damage";

	}
	string skillName[4];
	string skillDisc[4];
	//rewrite virtual function
	bool attack(Pokemon& aim, string& msg, int skillIndex, bool autoFight);
	string GetSkillName(int n)const { return skillName[n]; }
	string GetSkillDisc(int n)const { return skillDisc[n]; }
};