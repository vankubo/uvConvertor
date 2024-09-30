#include "uvconvertor.hpp"
#include "tinyxml2/tinyxml2.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include "utils.hpp"
#include <string>
#include <regex>
#include <algorithm>

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

	/*Debug*/
	//std::cout<<"[Debug]:Input file:"<<uvProjx<<std::endl;
	replace(uvProjx.begin(),uvProjx.end(),'\\','/');
	//std::cout<<"[Debug]:Input file:"<<uvProjx<<std::endl;

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

	tinyxml2::XMLElement* outputDirectory = target_elm->FirstChildElement("TargetOption") \
											->FirstChildElement("TargetCommonOption")->FirstChildElement("OutputDirectory");

	tinyxml2::XMLElement* outputName = target_elm->FirstChildElement("TargetOption") \
											->FirstChildElement("TargetCommonOption")->FirstChildElement("OutputName");

	// 获取编译生成的build.log.htm文件位置
	std::string buildLogHtmPath;
	buildLogHtmPath.append(ifPath + "/").append(outputDirectory->GetText()).append(outputName->GetText()).append(".build_log.htm");
	
	/*Debug*/
	//std::cout<<"[Debug] log file:"<<buildLogHtmPath<<std::endl;
	replace(buildLogHtmPath.begin(),buildLogHtmPath.end(),'\\','/');
	//std::cout<<"[Debug] log file:"<<buildLogHtmPath<<std::endl;


	// 获取编译生成的dep文件位置
	std::string depFilePath;
	depFilePath.append(ifPath + "/").append(outputDirectory->GetText())
		.append(p.filename().replace_extension().string()).append("_")
		.append(target_elm->FirstChildElement("TargetName")->GetText()).append(".dep");

	/*Debug*/
	//std::cout<<"[Debug] Dep file:"<<depFilePath<<std::endl;
	replace(depFilePath.begin(),depFilePath.end(),'\\','/');
	//std::cout<<"[Debug] Dep file:"<<depFilePath<<std::endl;

	// 提示编译获取编译日志文件
	if (!fs::exists(depFilePath))
	{
		std::cout << "[Failed] File:\"" << fs::absolute(depFilePath).string() << "\" not exist!" << std::endl;
		std::cout << "\n\nPlease compile first!!!\n" << std::endl;
	}
	else
	{
		// 解析编译日志文件，提取编译依赖
		std::ifstream depFile(depFilePath, std::ifstream::in);
		std::vector<std::string> buildDepStringList;
		//std::cout << "Load : " << fs::absolute(depFilePath).string() << " Successful!" << std::endl;
		while (depFile.good())
		{
			std::string lineStr;
			getline(depFile, lineStr);

			if (lineStr.find("F") == 0)
			{
				StringReplace(lineStr, "F \\(", "");		// 去除开头的F (
				StringReplace(lineStr, "\r", " ");		// 去除行尾的回车，巨坑，dep文件 F开头的行中，明明换行了，但是只有回车
				StringReplace(lineStr, " -", "  -");		// 为每个参数选项之间的空格增加一个空格，防止路径中含有空格导致参数分割错误。
				StringReplace(lineStr, "-I \"{0,1}", "-I");	// 去除-I后面的空格和"字符
				StringReplace(lineStr, "\\(0x[[:xdigit:]]+\\)", "");	// 去除不需要的地址
				StringReplace(lineStr, "\\\\", "/");					// 所有的\\替换为/
				lineStr.erase(lineStr.end() - 1);

				buildDepStringList.push_back(lineStr);
			}
		}
		depFile.close();

		// 获取编译器所在路径
		std::ifstream buildLogHtmFile(buildLogHtmPath, std::ifstream::in);
		std::regex toolchainReg("Toolchain Path:.*");
		std::string toolchainPath;
		while (buildLogHtmFile.good())
		{
			std::string lineStr;
			getline(buildLogHtmFile, lineStr);
			if (std::regex_match(lineStr, toolchainReg))
			{
				StringReplace(lineStr, "(Bin|bin)", "Include");
				StringReplace(lineStr, "Toolchain Path:  ", "-I");
				StringReplace(lineStr, "\\\\", "/");
				toolchainPath = lineStr;
				break;
			}
		}
		buildLogHtmFile.close();

		// 提取所有参数
		for (const auto& buildDepString : buildDepStringList)
		{
			std::list<std::string> fileAndArguments, arguments;
			StringSplit(buildDepString, "\\)\\(", fileAndArguments);

			m_fileList.push_back(fileAndArguments.front());
			fileAndArguments.pop_front();
			if (!fileAndArguments.empty())
			{
				StringSplit(fileAndArguments.back(), " {2,}", arguments);		// 按照两个空格分割参数
				for (auto& argumentStr : arguments)
				{
					if (argumentStr.find("-I") == 0 && argumentStr.rfind("\"") == (argumentStr.size() - 1))		// 为-I开头的行去除行尾"字符
					{
						argumentStr.erase(argumentStr.end() - 1);
					}
				}
				arguments.push_back(toolchainPath);
				
			}
			m_argumentsList.push_back(arguments);
		}
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
	for (std::list<std::list<std::string>>::iterator args = m_argumentsList.begin(); args != m_argumentsList.end(); ++args) {
		for (std::list<std::string>::iterator arg = args->begin(); arg != args->end(); ++arg)
		{
			std::cout << *arg << std::endl;
		}
		std::cout << "==============================" << std::endl;
    }

	std::cout<<"-------------------------------"<<std::endl;
    for (std::list<std::string>::iterator it = m_fileList.begin(); it != m_fileList.end(); ++it) {
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
	//extern options
	std::list<std::string> extop;
	StringSplit(extOptions, ",", extop);

	//export to json
	json	 j,j1;
	
	//files
	for (std::list<std::string>::iterator it = m_fileList.begin(); it != m_fileList.end(); ++it) {
        j1.clear();
		j1["file"]=*it;
		j1["directory"]=ifPath;
		std::list<std::list<std::string>>::iterator args = m_argumentsList.begin();
		//arguments
		for (std::list<std::string>::iterator arg = args->begin(); arg != args->end(); ++arg)
		{
			std::string arg_str = *arg;

			//clangd can't handle -imacros argument with space
			std::size_t found = arg_str.find("--imacros ");
			if (found != std::string::npos)
			{
				j1["arguments"].push_back("--imacros");
				arg_str.replace(found, 10, "");
				j1["arguments"].push_back(arg_str);
				continue;
			}
			
			found = arg_str.find("-imacros ");
			if (found != std::string::npos)
			{
				j1["arguments"].push_back("-imacros");
				arg_str.replace(found, 9, "");
				j1["arguments"].push_back(arg_str);
				continue;
			}

			found = arg_str.find("-preinclude=");
			if (found != std::string::npos)
			{
				j1["arguments"].push_back("-imacros");
				arg_str.replace(found, 12, "");
				j1["arguments"].push_back(arg_str);
				continue;
			}

			// Remove quotes from -D arguments
            if (arg_str.find("-D") == 0) {
                std::size_t quote_start = arg_str.find('"');
                std::size_t quote_end = arg_str.rfind('"');
                if (quote_start != std::string::npos && quote_end != std::string::npos && quote_end > quote_start) {
                    arg_str.erase(quote_start, 1); // Remove starting quote
                    arg_str.erase(quote_end - 1, 1); // Remove ending quote
                }
				j1["arguments"].push_back(arg_str);
				continue;
            }

			found = arg_str.find("--diag_suppress=");
			if (found != std::string::npos)
			{
				continue;
			}

			found = arg_str.find("-o ");
			if (found != std::string::npos)
			{
				continue;
			}

			found = arg_str.find("--depend ");
			if (found != std::string::npos)
			{
				continue;
			}

			found = arg_str.find("--apcs=");
			if (found != std::string::npos)
			{
				continue;
			}

			found = arg_str.find("--split_sections");
			if (found != std::string::npos)
			{
				continue;
			}

			found = arg_str.find("--cpu ");
			if (found != std::string::npos)
			{
				continue;
			}

			j1["arguments"].push_back(*arg);
		}

		//extern options
		if(extop.size()!=0)
		{
			for (std::list<std::string>::iterator e = extop.begin(); e != extop.end(); ++e) 
			{
				std::string arg_str = *e;
				if (arg_str.empty()) {
					continue;
				}
        		j1["arguments"].push_back(*e);
    		}
		}
		m_argumentsList.pop_front();

		j.push_back(j1);
    }
	
	
	// write prettified JSON to another file
	outPath.append("/compile_commands.json");

	/*debug*/
	//std::cout<<"[Debug] output file:"<<outPath<<std::endl;
	replace(outPath.begin(),outPath.end(),'\\','/');
	//std::cout<<"[Debug] output file:"<<outPath<<std::endl;

	std::ofstream o(outPath);
	o << std::setw(4) << j << std::endl;

}
