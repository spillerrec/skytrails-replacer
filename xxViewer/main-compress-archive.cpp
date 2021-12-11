#include "Falcom.hpp"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <filesystem>

std::vector<uint8_t> compress1(const uint8_t* buffer, int64_t size){
	std::vector<uint8_t> out;
	
	//Retained copy, if this is set we need to copy before doing anything else
	int64_t copy_start = -1;
	
	int64_t pos = 0;
	
	auto pushCopy = [&](){
		if( copy_start < 0 )
			return;
		
		int64_t length = pos - copy_start;
		if( length < (1 << 5) )
			out.push_back(0b00000000 | length);
		else if( length < (1 << 13) ){
			out.push_back(0b00100000 | (length >> 8));
			out.push_back(length & 0xFF);
		}
		else
			throw std::runtime_error("Very large copies not implemented");
		
		for( int64_t i=0; i<length; i++ )
			out.push_back( buffer[copy_start + i] );
		
		copy_start = -1;
	};
	
	while( pos < size ){
		//Check if byte repeats
		int next_byte_repeats = 1;
		while(buffer[pos] == buffer[pos + next_byte_repeats])
			next_byte_repeats++;
		
		//Check lookback
		int64_t best_run = 0;
		int64_t best_run_pos = -1;
		for( int64_t i=pos-1; i > std::max((int64_t)0, pos-0x1FFF); i-- ){
			if( buffer[i] == buffer[pos] ){
				//Possible run
				int64_t length = 1;
				while( pos+length < size && buffer[i+length] == buffer[pos+length] )
					length++;
				if( length > best_run ){
					best_run = length;
					best_run_pos = i;
				}
			}
		}
		
		bool commited = false;
		
		if( next_byte_repeats > best_run ){
			if( next_byte_repeats >= 4){
				next_byte_repeats -= 4;
				pushCopy();
				
				if( next_byte_repeats < (1 << 4) ){
					out.push_back( 0b01000000 | next_byte_repeats );
				}
				else{
					next_byte_repeats = std::min(next_byte_repeats, 0xFFF);
					out.push_back( 0b01010000 | (next_byte_repeats >> 8) );
					out.push_back( (next_byte_repeats & 0xFF) );
				}
				out.push_back( buffer[pos] );
				pos += next_byte_repeats + 4;
				commited = true;
			}
		}
		else{
			if( best_run >= 4 ){ //TODO: Check if 4 works!
				pushCopy();
				
				auto offset = pos - best_run_pos;
				
				int remaining = best_run - 4;
				
				int start = remaining & 3;
				remaining -= start;
				auto sub = 0b10000000 | (start << 5) | (offset >> 8);
				out.push_back( sub );
				out.push_back( offset & 0xFF );
				
				while( remaining > 0 ){
					int local = std::min( remaining, 0b00011111 );
					out.push_back( 0b01100000 | local );
					remaining -= local;
				}
				
				pos += best_run;
				commited = true;
			}
		}
		
		if (!commited){
			if( copy_start < 0 )
				copy_start = pos;
			
			if( pos + 1 == 0x1FFF )
				pushCopy();
			else
				pos++;
		}
	}
	pushCopy();
	
	return out;
}


int main( int argc, char* argv[] ){
	std::string dir_file( argv[1] );
	auto base_name = dir_file.substr(0, dir_file.size()-3);
	
	auto data = File( dir_file.c_str(), "rb" ).readAll();
	auto out = File( (base_name + "compressed").c_str(), "wb" );
	
	int64_t block_size = 64*1024;
	int64_t remaining_size = data.size();
	
	int counter = 0;
	while( remaining_size > 0 ){
		// Compress a block of data which is no larger than block_size
		auto current_block = std::min(block_size, remaining_size);
		auto res = compress1( data.data() + counter++ * block_size, current_block );
		remaining_size -= current_block;
		
		// Write this size
		uint16_t compressed_size = res.size() + 1;
		out.write(ArrayView{&compressed_size, 1});
		
		// Write out how many blocks remain
		auto remaining_blocks = (remaining_size + block_size-1) / block_size;
		if( remaining_blocks >= 0x100 )
			throw std::runtime_error("File too large to compress!");
		res.push_back( remaining_blocks );
		
		out.write(ArrayView{res.data(), res.size()});
	}
	
	
	return 0;
}