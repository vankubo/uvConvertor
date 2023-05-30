#include <iostream>
#include <filesystem>
#include "tinyxml2/tinyxml2.h"
#include <list>
#include <regex>
#include <fstream>
#include "nlohmann/json.hpp"
#include <CLI/CLI.hpp>
#include <vector>

namespace fs=std::filesystem;
using namespace tinyxml2;
using namespace nlohmann;
using namespace std;


void Stringsplit(const string& str, const string& split, list<string>& res)
{
	//std::regex ws_re("\\s+"); // 正则表达式,匹配空格 
	std::regex reg(split);		// 匹配split
	std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1);
	decltype(pos) end;              // 自动推导类型 
	for (; pos != end; ++pos)
	{
		res.push_back(pos->str());
	}
}

void StringReplace(string& str,const string& pat,const string& fmt)
{
	regex pattern(pat);
    str=regex_replace(str, pattern, fmt);
}

class uVConvertor
{
	public:
	uVConvertor(string uvprojx);
	~uVConvertor();
	void toCompileJson(string outPath,string extOptions="");
	void printItems();
	private:
	list<string> defList;
	list<string> fileList;
	list<string> incList;
	string ifPath;
};

/***************************************************************
  *  @brief     parse uvprojx file to list
  *  @param        
  *  @note      
  *  @Sample usage:      
 **************************************************************/
uVConvertor::uVConvertor(string uvProjx)
{
	fs::path p(uvProjx);
	ifPath=p.parent_path().string();
	//加载文件
	XMLDocument doc;
	if(doc.LoadFile( uvProjx.c_str() )!=0)
	{
	cout<<doc.ErrorID()<<endl;
		return;
	}
	//2.找到根节点
	tinyxml2::XMLElement* xmlroot = doc.RootElement();
	if (xmlroot == NULL) {
    	return;
	}
	cout<<"root:"<<xmlroot->Name()<<endl; 
	//3.获取子节点信息
	//include
	tinyxml2::XMLElement* includePath = xmlroot->FirstChildElement("Targets")->FirstChildElement("Target")->FirstChildElement("TargetOption") \
										 ->FirstChildElement("TargetArmAds")->FirstChildElement("Cads")->FirstChildElement("VariousControls") \
										 ->FirstChildElement("IncludePath");
	std::string inc = includePath->GetText();
	cout<<"IncludePath:"<<inc<<endl;
	Stringsplit(inc,";" ,incList);

	///define
	tinyxml2::XMLElement* define = xmlroot->FirstChildElement("Targets")->FirstChildElement("Target")->FirstChildElement("TargetOption") \
										 ->FirstChildElement("TargetArmAds")->FirstChildElement("Cads")->FirstChildElement("VariousControls") \
										 ->FirstChildElement("Define");
	std::string def = define->GetText();
	cout<<"Defines:"<<def<<endl;
	Stringsplit(def,"," ,defList);
	///src
	tinyxml2::XMLElement* groups = xmlroot->FirstChildElement("Targets")->FirstChildElement("Target")->FirstChildElement("Groups");
	tinyxml2::XMLElement* group;
	tinyxml2::XMLElement* groupname;
	group=groups->FirstChildElement("Group");
	std::string f;
	while(group!=nullptr)
	{
		groupname=group->FirstChildElement("GroupName");
		f=groupname->GetText();
		cout<<"->:"<<f<<endl;
		tinyxml2::XMLElement* file=group->FirstChildElement("Files")->FirstChildElement("File");
		while(file!=nullptr)
		{
			tinyxml2::XMLElement* filePath=file->FirstChildElement("FilePath");
			cout<<"----"<<filePath->GetText()<<endl;
			fileList.push_back(filePath->GetText());
			file=file->NextSiblingElement("File");
		}

		group=group->NextSiblingElement("Group");
	}
	
	//conv to abs path
	for (std::list<string>::iterator it = incList.begin(); it != incList.end(); ++it) {
       *it=ifPath+"\\"+*it;

	   StringReplace(*it,"\\\\","/");
	   
    }
	for (std::list<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        *it=ifPath+"\\"+*it;
		 StringReplace(*it,"\\\\","/");
	   
    }
	
}

uVConvertor::~uVConvertor()
{

}

/***************************************************************
  *  @brief     print out list
  *  @param        
  *  @note      
  *  @Sample usage:      
 **************************************************************/
void uVConvertor::printItems()
{
	//
	cout<<"-------------------------------"<<endl;
	//使用迭代器输出list容器中的元素
    for (std::list<string>::iterator it = incList.begin(); it != incList.end(); ++it) {
        std::cout << *it << endl;
    }
	cout<<"-------------------------------"<<endl;
    for (std::list<string>::iterator it = defList.begin(); it != defList.end(); ++it) {
        std::cout << *it << endl;
    }
	cout<<"-------------------------------"<<endl;
    for (std::list<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        std::cout << *it << endl;
    }
	cout<<"-------------------------------"<<endl;
}
//"D:\PROJECT\uVConvertor\audioRec\USER\FATFS.uvprojx"
/***************************************************************
  *  @brief     export list to json file
  *  @param        
  *  @note      
  *  @Sample usage:      
 **************************************************************/
void uVConvertor::toCompileJson(string outPath,string extOptions)
{

	//rebase file path in list
	
	//2.rebase 
	for (std::list<string>::iterator it = incList.begin(); it != incList.end(); ++it) {
        fs::path p(*it);
		*it=p.lexically_relative(outPath).string();
		StringReplace(*it,"\\\\","/");
    }
	for (std::list<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        fs::path p(*it);
		*it=p.lexically_relative(outPath).string();
		StringReplace(*it,"\\\\","/");
    }
	//remove redundancy path

	//export to json
	json	 j,j1;
	
	//files
	for (std::list<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        j1.clear();
		j1["file"]=*it;
		j1["directory"]=outPath;
		//arguments
			//include
		for (std::list<string>::iterator inc = incList.begin(); inc != incList.end(); ++inc) {
        j1["arguments"].push_back("-I"+*inc);
    	}
		for (std::list<string>::iterator def = defList.begin(); def != defList.end(); ++def) {
        j1["arguments"].push_back("-D"+*def);
    	}

		j.push_back(j1);
    }
	
	
	// write prettified JSON to another file
	std::ofstream o(outPath+"\\pretty.json");
	o << std::setw(4) << j << std::endl;

	cout<<"-------------------------------"<<endl;
#if 0
	fs::path p=fs::current_path(); 
    std::cout << "The current path " << p << " decomposes into:\n"
              << "root-path " << p.root_path() << '\n'
              << "relative path " << p.relative_path() << '\n'
			  << "relative to:"<<p.lexically_relative("D:\\PROJECT\\C++\\freetype\\FontMap\\build");
#endif
}


#define DEBUG 0

int main(int argc, char **argv)
{
	

	//CLI11
	CLI::App app("uVConvertor");
    // add version output
    app.set_version_flag("--version", std::string(CLI11_VERSION));
    std::string inputFile;
    CLI::Option *opt = app.add_option("-f,--file", inputFile, "uvProject File name")->check(CLI::ExistingFile)->required();

	std::string outputFile;
    CLI::Option *copt = app.add_option("-o,--output",outputFile, "Output path")->check(CLI::ExistingDirectory);

	std::string extOptions;
	app.add_option("-e,--extoptions",extOptions,"External Options");
	
    CLI11_PARSE(app, argc, argv);
#if DEBUG
	cout<<"-------------------------------"<<endl;
    cout<<"input file:"<<inputFile<<endl;
	cout<<"output file:"<<outputFile<<endl;
	
		cout<<"ext options"<<extOptions<<endl;
	
	cout<<"-------------------------------"<<endl;
#endif
	uVConvertor uvc(inputFile);
	uvc.printItems();
	uvc.toCompileJson(outputFile);

	
	
	
	return 0;
}
