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


void StringSplit(const string& str, const string& split, list<string>& res)
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

void RemoveRedundancyPath(string&  str)
{
	list<string> res;
	list<string> pathItem;
	StringSplit(str, "/", res);
	regex pattern("[.]{2}");
	regex pattern1("[.]{1}");
	for(list<string>::iterator it=res.begin();it!=res.end();it++)
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
	for(list<string>::iterator it=pathItem.begin();it!=pathItem.end();it++)
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

class uVConvertor
{
	public:
	uVConvertor(string uvprojx, string target);
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
uVConvertor::uVConvertor(string uvProjx, string target)
{
	fs::path p(uvProjx);
	ifPath=p.parent_path().string();
	//加载文件
	XMLDocument doc;
	if(doc.LoadFile( uvProjx.c_str() )!=0)
	{
	//cout<<doc.ErrorID()<<endl;
		return;
	}
	//2.找到根节点
	tinyxml2::XMLElement* xmlroot = doc.RootElement();
	if (xmlroot == NULL) {
    	return;
	}
	//cout<<"root:"<<xmlroot->Name()<<endl;

	tinyxml2::XMLElement* target_elm;
	if (target != "") {
		while (true) {
			target_elm = xmlroot->FirstChildElement("Targets")->FirstChildElement("Target");
			if (!target_elm) {
				break;
			}
			if (target_elm->FirstChildElement("TargetName")->GetText() == target) {
				break;
			}
			xmlroot->FirstChildElement("Targets")->DeleteChild(target_elm);
		};
		if (!target_elm) {
			cout<<"error: target <"<<target<<"> not found!"<<endl;
			return;
		}

	} else {
		target_elm = xmlroot->FirstChildElement("Targets")->FirstChildElement("Target");
	}

	//3.获取子节点信息
	//include
	tinyxml2::XMLElement* includePath = target_elm->FirstChildElement("TargetOption") \
										 ->FirstChildElement("TargetArmAds")->FirstChildElement("Cads")->FirstChildElement("VariousControls") \
										 ->FirstChildElement("IncludePath");
	std::string inc = includePath->GetText();
	//cout<<"IncludePath:"<<inc<<endl;
	StringSplit(inc,";" ,incList);

	///define
	tinyxml2::XMLElement* define = target_elm->FirstChildElement("TargetOption") \
										 ->FirstChildElement("TargetArmAds")->FirstChildElement("Cads")->FirstChildElement("VariousControls") \
										 ->FirstChildElement("Define");
	std::string def = define->GetText();
	//cout<<"Defines:"<<def<<endl;
	StringSplit(def,"," ,defList);
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
		//cout<<"->:"<<f<<endl;
		tinyxml2::XMLElement* file=group->FirstChildElement("Files")->FirstChildElement("File");
		while(file!=nullptr)
		{
			tinyxml2::XMLElement* filePath=file->FirstChildElement("FilePath");
			//cout<<"----"<<filePath->GetText()<<endl;
			fileList.push_back(filePath->GetText());
			file=file->NextSiblingElement("File");
		}

		group=group->NextSiblingElement("Group");
	}
	
	//conv to abs path
	//remove redundancy path
	for (std::list<string>::iterator it = incList.begin(); it != incList.end(); ++it) {
       *it=ifPath+"\\"+*it;

	   StringReplace(*it,"\\\\","/");
	   RemoveRedundancyPath(*it);
    }
	for (std::list<string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        *it=ifPath+"\\"+*it;
		 StringReplace(*it,"\\\\","/");
	   	RemoveRedundancyPath(*it);
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
	
	//extern options
	list<string> extop;
	StringSplit(extOptions,",",extop);

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
			//def
		for (std::list<string>::iterator def = defList.begin(); def != defList.end(); ++def) {
        j1["arguments"].push_back("-D"+*def);
    	}
			//extern options
		if(extop.size()!=0)
		{
			for (std::list<string>::iterator e = extop.begin(); e != extop.end(); ++e) 
			{
        		j1["arguments"].push_back(*e);
    		}
		}
		

		j.push_back(j1);
    }
	
	
	// write prettified JSON to another file
	std::ofstream o(outPath+"\\compile_commands.json");
	o << std::setw(4) << j << std::endl;

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

	std::string target;
    CLI::Option *topt = app.add_option("-t,--target", target, "uvProject target name");

	std::string outputFile;
    CLI::Option *copt = app.add_option("-o,--output",outputFile, "Output path")->check(CLI::ExistingDirectory);

	std::string extOptions;
	app.add_option("-e,--extoptions",extOptions,"External Options");
	
    CLI11_PARSE(app, argc, argv);

	// convertor to absolute path
	fs::path in_path(inputFile);
	inputFile = std::filesystem::absolute(in_path).string();

	fs::path out_path(outputFile);
	outputFile = std::filesystem::absolute(outputFile).string();

#if DEBUG
	cout<<"-------------------------------"<<endl;
    cout<<"input file:"<<inputFile<<endl;
	cout<<"output file:"<<outputFile<<endl;
	
		cout<<"ext options:"<<extOptions<<endl;
	
	cout<<"-------------------------------"<<endl;
#endif
	uVConvertor uvc(inputFile, target);
	//uvc.printItems();
	uvc.toCompileJson(outputFile,extOptions);
	
	
	return 0;
}
