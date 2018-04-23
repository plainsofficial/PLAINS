#pragma once

#include <iostream>
#include <Windows.h>
#include <string>
#include "Hooks.h"

class CheatUserStruct
{
public:
	std::string name;
	std::vector<int> addr;
	int mac_int;

	CheatUserStruct(std::string _name, std::vector<int> _addr, int _mac_int)
	{
		this->name = _name;
		this->addr = _addr;
		this->mac_int = _mac_int;
	}
};

class SecurityVaults
{
public:
	std::vector<CheatUserStruct> users;
	void setup( )
	{
		
	}
};
extern SecurityVaults vaults;