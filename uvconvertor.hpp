#pragma once
#include <string>
#include <vector>
#include <list>

/***************************************************************
  *  @brief 
  *  @param        
  *  @note      
  *  @Sample usage:      
 **************************************************************/
class uVConvertor
{
	public:
	uVConvertor(std::string uvprojx, std::string target);
	~uVConvertor();
	void toCompileJson(std::string outPath,std::string extOptions="");
	void printItems();
	private:
	std::list<std::string> m_fileList;
	std::list<std::list<std::string>> m_argumentsList;
	std::string ifPath;
};