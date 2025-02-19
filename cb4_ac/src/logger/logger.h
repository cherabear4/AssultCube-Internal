#pragma once
#include "../../includes.h"

class Logger
{
public:
	inline void log(std::string text)
	{
		printf("[AC] ");
		printf(text.c_str());
		printf("\n");
	}
};