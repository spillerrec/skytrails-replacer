#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

int error(const char* const msg)
{
	std::cout << msg << std::endl;
	return -1;
}

enum class Type{
	Frame,
	Bone,
	Mesh
};

const std::string type_ext[] = {std::string(".frame"), std::string(".bone"), std::string(".mesh") };

int main(int argc, const char* argv[]){
	if( argc != 2 )
		return error("dissector INPUT_FILE\n");
	
	auto file = std::fopen( argv[1], "rb" );
	if( !file )
		return error( "could not open file" );
	
	std::fseek( file, 0, SEEK_END );
	int64_t file_size = std::ftell( file );
	std::fseek( file, 0, SEEK_SET );
	std::cout << "File size: "<< file_size << std::endl;
	
	std::vector<char> data;
	data.resize(file_size+1);
	std::fread( data.data(), file_size, 1, file );
	data.back() = 0; //Make sure compare later will not run out of the array
	
	std::fclose(file);
	
	std::string look_for = "Frame";
	int64_t str_size = look_for.size();
	std::string look_for2 = "CD3DFile_Mesh";
	int64_t str_size2 = look_for.size();
	std::vector<int64_t> offsets;
	for( int64_t i=0; i<file_size; i++ )
	{
		if (look_for.compare(0, str_size, data.data() + i, std::min(str_size, file_size-i)) == 0)
			offsets.push_back( i );
		else
			if (look_for2.compare(0, str_size2, data.data() + i, std::min(str_size2, file_size-i)) == 0)
				offsets.push_back( i );
	}
	
	FILE* out = nullptr;
	for(size_t i=0; i<offsets.size(); i++)
	{
		auto start = offsets[i];
		auto end = (i+1 >= offsets.size()) ? file_size : offsets[i+1];
		
		Type type = Type::Frame;
		std::string name( data.data() + start, 260 );
		if( name.find("Bone") != std::string::npos )
			type = Type::Bone;
		if( name.find("Mesh") != std::string::npos )
			type = Type::Mesh;
		std::string ext = type_ext[static_cast<int>(type)];
		
		if( type == Type::Frame ){
			std::fclose( out );
			out = std::fopen( ("out/stream" + std::to_string(i) + ext).c_str(), "wb" );
		}
		fwrite(data.data()+start, end-start, 1, out);
		
		if( type != Type::Frame )
		{
			auto ofile = std::fopen( ("out/stream" + std::to_string(i) + ext).c_str(), "wb" );
			if( !ofile )
				return error( "Could not open output file, do the 'out'  directory exist?" );
			
			fwrite(data.data()+start, end-start, 1, ofile);
			
			std::fclose(ofile);
		}
		
	}
	std::fclose(out);
	std::cout << "Count: " << offsets.size() << "\n";
	
	return 0;
}