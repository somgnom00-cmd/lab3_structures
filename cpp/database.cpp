#pragma once
#include "database.h"

void help();
bool execute(Database& database, const std::string& query);
