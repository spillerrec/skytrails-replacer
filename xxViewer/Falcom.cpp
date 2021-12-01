#include "Falcom.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>

static inline void rtrim( std::string &s ){
	auto not_space = []( auto ch ){ return !std::isspace(ch); };
	s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
}

std::string Falcom::ArchiveEntry::fixedFilename(){
	std::string a(std::begin(name), std::end(name));
	std::string b(std::begin(extension), std::end(extension));
	rtrim(a);
	rtrim(b);
	if( b == "_DS" )
		b[0] = 'D';
	else if( b == "_HD" )
		b[0] = 'S';
	return a + '.' + b;
}

Falcom::Model::Model( Buffer data_1 ) : data( data_1 ){
	ByteViewReader reader( data );
	
	try{
		type     = reader.read8u();
		version  = reader.read8u();
		if( version >= 2 )
			unknown1 = reader.read( 11*sizeof(float) );
		if( version == 0 )
			std::cout << "Version 0 is known not to work\n";
	
		children_count = reader.read16u();
		frames.resize( children_count );
		for( auto& frame : frames )
			frame.read( reader, version );
		remainder = reader.read(reader.left());
	}
	catch( ... )
	{
		std::cout << "Warning, file reading stopped with errors!!!" << std::endl;
	}
}

