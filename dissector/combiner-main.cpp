#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

int error(const char* const msg)
{
	std::cout << msg << std::endl;
	return -1;
}

int main(int argc, const char* argv[]){
	if( argc <= 2 )
		return error("dissector [INPUT_FILES]\n");
	
	auto ofile = std::fopen( (std::string(argv[1]) + ".combined").c_str(), "wb" );
	if( !ofile )
		return error( "could not open output file" );
	
	std::vector<char> data;
	for( int i=1; i<argc; i++ ){
		auto file = std::fopen( argv[i], "rb" );
		if( !file )
			return error( "could not open input file" );
		
		std::fseek( file, 0, SEEK_END );
		int64_t file_size = std::ftell( file );
		std::fseek( file, 0, SEEK_SET );
		
		std::cout << "Adding: " << file_size << " bytes\n";
		data.resize(file_size);
		std::fread( data.data(), file_size, 1, file );
		std::fwrite( data.data(), file_size, 1, ofile );
		
		std::fclose(file);
	}
	
	std::fclose(ofile);
	
	return 0;
}