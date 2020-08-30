#include "Falcom.hpp"

#include <filesystem>
#include <iostream>
#include <stdlib.h>
#include "termcolor/termcolor.hpp"
namespace fs = std::filesystem;

int main( int argc, char* argv[] ){
	int success = 0, failure = 0;
	for(auto& p : fs::directory_iterator(argv[1]))
	{
		auto filename = p.path();
		auto ext = filename.extension();
		if( ext != "._X3" && ext != ".X" )
			continue;
		
		Falcom::Model xx( File( filename.string().c_str(), "rb" ).readAll() );
		
		bool isValid = xx.validate();
		if (isValid)
			success++;
		else
			failure++;
		if (isValid)
			std::cout << termcolor::green << "Success" << termcolor::reset;
		else
			std::cout << termcolor::red   << "Failed " << termcolor::reset;
		std::cout << " - " << filename << std::endl;
	}
	
	std::cout << "Files ok: " << success << std::endl;
	std::cout << "Files failed: " << failure << std::endl;
	
	return 0;
}