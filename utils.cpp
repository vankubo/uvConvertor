#include "utils.hpp"

void StringSplit(const std::string& str, const std::string& split, std::list<std::string>& res)
{
	//std::std::regex ws_re("\\s+"); // 正则表达式,匹配空格 
	std::regex reg(split);		// 匹配split
	std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1);
	decltype(pos) end;              // 自动推导类型 
	for (; pos != end; ++pos)
	{
		res.push_back(pos->str());
	}
}

void StringReplace(std::string& str,const std::string& pat,const std::string& fmt)
{
	std::regex pattern(pat);
    str=regex_replace(str, pattern, fmt);
}


void RemoveRedundancyPath(std::string&  str)
{
	std::list<std::string> res;
	std::list<std::string> pathItem;
	StringSplit(str, "/", res);
	std::regex pattern("[.]{2}");
	std::regex pattern1("[.]{1}");
	for(std::list<std::string>::iterator it=res.begin();it!=res.end();it++)
	{
		if(regex_match(*it,pattern1))
		{
			continue;
		}

		if(!regex_match(*it,pattern))
		{
			pathItem.push_back(*it);
		}
		else 
		{
			pathItem.pop_back();
		}
	}

	str.clear();
	for(std::list<std::string>::iterator it=pathItem.begin();it!=pathItem.end();it++)
	{
		if(std::next(it) == pathItem.end())
		{
			str+=(*it);
		}
		else {
		str+=(*it)+"/";
		}
			
	}

}