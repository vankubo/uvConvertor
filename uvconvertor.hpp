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
	std::list<std::string> defList;
	std::list<std::string> fileList;
	std::list<std::string> incList;
	std::string ifPath;
};