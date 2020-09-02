#include "Falcom.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>

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

