#include "uvconvertor.hpp"
#include "tinyxml2/tinyxml2.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <string>



namespace fs=std::filesystem;
using namespace tinyxml2;
using namespace nlohmann;

/***************************************************************
  *  @brief     parse uvprojx file to list
  *  @param        
  *  @note      
  *  @Sample usage:      
 **************************************************************/
uVConvertor::uVConvertor(std::string uvProjx, std::string target)
{
	fs::path p(uvProjx);
	ifPath=p.parent_path().string();
	//加载文件
	XMLDocument doc;
	if(doc.LoadFile( uvProjx.c_str() )!=0)
	{
	//cout<<doc.ErrorID()<<std::endl;
		return;
	}
	//2.找到根节点
	tinyxml2::XMLElement* xmlroot = doc.RootElement();
	if (xmlroot == NULL) {
    	return;
	}
	//cout<<"root:"<<xmlroot->Name()<<std::endl;

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
			std::cout<<"error: target <"<<target<<"> not found!"<<std::endl;
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
	//cout<<"IncludePath:"<<inc<<std::endl;
	StringSplit(inc,";" ,incList);

	///define
	tinyxml2::XMLElement* define = target_elm->FirstChildElement("TargetOption") \
										 ->FirstChildElement("TargetArmAds")->FirstChildElement("Cads")->FirstChildElement("VariousControls") \
										 ->FirstChildElement("Define");
	std::string def = define->GetText();
	//cout<<"Defines:"<<def<<std::endl;
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
		//cout<<"->:"<<f<<std::endl;
		tinyxml2::XMLElement* file=group->FirstChildElement("Files")->FirstChildElement("File");
		while(file!=nullptr)
		{
			tinyxml2::XMLElement* filePath=file->FirstChildElement("FilePath");
			//cout<<"----"<<filePath->GetText()<<std::endl;
			fileList.push_back(filePath->GetText());
			file=file->NextSiblingElement("File");
		}

		group=group->NextSiblingElement("Group");
	}
	
	//conv to abs path
	//remove redundancy path
	for (std::list<std::string>::iterator it = incList.begin(); it != incList.end(); ++it) {
       *it=ifPath+"\\"+*it;

	   StringReplace(*it,"\\\\","/");
	   RemoveRedundancyPath(*it);
    }
	for (std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
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
	std::cout<<"-------------------------------"<<std::endl;
	//使用迭代器输出list容器中的元素
    for (std::list<std::string>::iterator it = incList.begin(); it != incList.end(); ++it) {
        std::cout << *it << std::endl;
    }
	std::cout<<"-------------------------------"<<std::endl;
    for (std::list<std::string>::iterator it = defList.begin(); it != defList.end(); ++it) {
        std::cout << *it << std::endl;
    }
	std::cout<<"-------------------------------"<<std::endl;
    for (std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        std::cout << *it << std::endl;
    }
	std::cout<<"-------------------------------"<<std::endl;
}

//"D:\PROJECT\uVConvertor\audioRec\USER\FATFS.uvprojx"
/***************************************************************
  *  @brief     export list to json file
  *  @param        
  *  @note      
  *  @Sample usage:      
 **************************************************************/
void uVConvertor::toCompileJson(std::string outPath,std::string extOptions)
{

	//rebase file path in list
	
	//2.rebase 
	for (std::list<std::string>::iterator it = incList.begin(); it != incList.end(); ++it) {
        fs::path p(*it);
		*it=p.lexically_relative(outPath).string();
		StringReplace(*it,"\\\\","/");
    }
	for (std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        fs::path p(*it);
		*it=p.lexically_relative(outPath).string();
		StringReplace(*it,"\\\\","/");
    }
	
	//extern options
	std::list<std::string> extop;
	StringSplit(extOptions,",",extop);

	//export to json
	json	 j,j1;
	
	//files
	for (std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        j1.clear();
		j1["file"]=*it;
		j1["directory"]=outPath;
		//arguments
			//include
		for (std::list<std::string>::iterator inc = incList.begin(); inc != incList.end(); ++inc) {
        j1["arguments"].push_back("-I"+*inc);
    	}
			//def
		for (std::list<std::string>::iterator def = defList.begin(); def != defList.end(); ++def) {
        j1["arguments"].push_back("-D"+*def);
    	}
			//extern options
		if(extop.size()!=0)
		{
			for (std::list<std::string>::iterator e = extop.begin(); e != extop.end(); ++e) 
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
