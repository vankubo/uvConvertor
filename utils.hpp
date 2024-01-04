#pragma once
#include <regex>
#include <vector>
#include <list>
#include <string>

/*
* function
*/
void StringSplit(const std::string& str, const std::string& split, std::list<std::string>& res);
void StringReplace(std::string& str,const std::string& pat,const std::string& fmt);
void RemoveRedundancyPath(std::string&  str);
