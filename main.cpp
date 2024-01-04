#include <iostream>
#include "uvconvertor.hpp"
#include <filesystem>
#include <CLI/CLI.hpp>

using namespace std;
namespace fs=std::filesystem;

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
	
	cout << "Done." << endl;
	
	return 0;
}
