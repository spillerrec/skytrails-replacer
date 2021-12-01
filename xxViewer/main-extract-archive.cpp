#include "Falcom.hpp"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <filesystem>

struct Stream
{
	unsigned char* buffer = nullptr;
	int64_t pos = 0;
	int64_t max_pos = 0;
	
	Stream(unsigned char* buffer, int64_t size) : buffer(buffer), max_pos(size) {}
	
	auto posNow() const { return pos; }
	
	uint8_t valueAt(int64_t lookup_pos)
	{
		if (lookup_pos >= pos)
			return 0;
		if (lookup_pos >= max_pos )
			throw std::runtime_error("Trying to read past buffer end!");
		if (lookup_pos < 0 )
			throw std::runtime_error("Trying to read past buffer start!");
		return buffer[lookup_pos];
	}
	uint8_t peek() const{ return buffer[pos]; }
	
	uint8_t read8() {
		if( pos >= max_pos )
			throw std::runtime_error("Trying to read past buffer end!");
		return buffer[pos++];
	}
	
	uint16_t read16() {
		auto low = read8();
		return read8() * 256 + low;
	}
	
	void write8(uint8_t byte) {
		if( pos >= max_pos )
			throw std::runtime_error("Trying to write past buffer end!");
		buffer[pos++] = byte;
	}
	
	bool isEof() const { return pos >= max_pos; }
};

void decompress2(Stream& input, Stream& output, int64_t size){
	auto start_offset = input.pos;
	
	while( !input.isEof() && size > (input.pos - start_offset + 2) )
	{
		int action = input.read8();
		// f = flag
		// s/S = length / lookback-length
		// 0000.0000 No op
		// 00fs.ssss Copy from input
		// 010f.ssss Repeat byte
		// 011?.???? Unused?
		// 1ssS.SSSS lookback
		// 111s.ssss Lookback extension (following lookback)
		
		if( (action & 128) == 0 ){ // 0---.----
			if( (action & 64) == 0 ){ // 00--.---- Copy from input (i.e. no compression)
				int length = action & 31; // 00-X.XXXX
				if( (action & 32) != 0 )   // 001-.----
					length = length * 256 + input.read8();
				
				for( int i=0; i<length; i++ )
					output.write8(input.read8());
			}
			else{ // Repeat specified byte
				if( (action & 32) != 0 ) // 011-.---- Unused opcode?
					throw std::runtime_error("Unknown bit in decompression set to true!");
				
				int length = (action & 15); // ----.XXXX
				if( (action & 16) != 0 ){   // 0101.----
					auto p = input.read8();
					length = length * 256 + p;
				}
				length += 4;
				
				auto fillbyte = input.read8();
				for (int i=0; i<length; i++)
					output.write8(fillbyte);
			}
		}
		else{ // 1---.---- Copy from previous output (lookback)
			int lookbacklength = ((action & 31) << 8) + input.read8(); // 1--X.XXXX
			int length = ((action >> 5) & 3) + 4; // -XX-.---- Can never be less than 7?
			if (length != 7)
				std::cout << "Length: " << length << " " << (int)action << std::endl;
				
			while( (input.peek() & 0xE0) == 96 ) //All top3 bits are set, 111-.----
				length += input.read8() & 31;
				
			auto startPos = output.posNow() - lookbacklength;
			for (int i=0; i<length; i++)
				output.write8(output.valueAt(startPos + i));
		}
	}
}

template<typename StreamType>
void decompress1(StreamType& input, StreamType& output){
	int bitstream = input.read8();
	int bits = 8;
	
	auto getBit = [&](){
		if( bits == 0 ){
			bitstream = input.read16();
			bits = 16;
		}
		
		bool bit = (bitstream % 2 == 1);
		bitstream /= 2;
		bits--;
		return bit;
	};
	
	while( !input.isEof() ){
		if( getBit() ){
			bool extended = getBit();
			unsigned run = 0x00;
			if( extended ){
				// Read 5 bit number
				run += getBit() ? 16 : 0;
				run += getBit() ?  8 : 0;
				run += getBit() ?  4 : 0;
				run += getBit() ?  2 : 0;
				run += getBit() ?  1 : 0;
			}
			unsigned prev_u_buffer_pos = input.read8() + (run << 0x08);
			
			if( prev_u_buffer_pos == 0 && extended )
				return;
			
			else if( prev_u_buffer_pos == 1 && extended ){ // Repeat byte
				bool branch = getBit();

				// Read 4 bit number
				int amount = 0;
				amount += getBit() ? 8 : 0;
				amount += getBit() ? 4 : 0;
				amount += getBit() ? 2 : 0;
				amount += getBit() ? 1 : 0;

				if (branch)
					amount = (amount << 0x08) + input.read8();
				
				amount += 0x0E;
				
				unsigned char byte = input.read8();
				for(int i = 0; i < amount; i++)
					output.write8(byte);
			}
			else{ // Copy previous data
				int run = 0x02;
			   
				if( !getBit() ){
					run++;

					if( !getBit() ){
						run++;

						if( !getBit() ){
							run++;

							if( !getBit() ){
								if( !getBit() ){
									run = input.read8() + 0x0E;
								}
								else{
									// 03D69
									run++;
									run += getBit() ? 4 : 0;
									run += getBit() ? 2 : 0;
									run += getBit() ? 1 : 0;
								}
							}
						}
					}
				}
				
				for(int i = 0; i < run; i++)
					output.write8(output.valueAt(output.pos - prev_u_buffer_pos));
			}
		}
		else
			output.write8( input.read8() );
	}
}




int main( int argc, char* argv[] ){
	std::string dir_file( argv[1] );
	auto base_name = dir_file.substr(0, dir_file.size()-3);
	
	auto data = File( dir_file.c_str(), "rb" ).readAll();
	auto main = File( (base_name + "dat").c_str() );
	ByteViewReader reader{data};
	Falcom::Archive xx;
	xx.read( reader );
	std::cout << "Contains " << xx.entries.size() << " entries\n";
	std::cout << "Magic: " << xx.magic << "\n";
	std::cout << "Value: " << xx.value << "\n";
		
	std::filesystem::create_directory( base_name );
		
	auto buffer_output_size = 0x4000000; // 64 MiB, should be KiB?
	auto buffer_out = std::make_unique<uint8_t[]>( buffer_output_size );
	
	for( auto& entry : xx.entries ){
		std::cout << std::string(entry.name) << " " 
			<< entry.compressed_size << " " 
			<< entry.uncompressed_size  << " " 
			<< entry.offset  << " " 
			<< entry.unknown1  << " " 
			<< entry.unknown2  << " " 
			<< "\n";
			
		if( entry.compressed_size == 0 ){
			std::cout << "Skipping empty file: " << entry.fixedFilename() << std::endl;
			continue;
		}
			
		main.seek(entry.offset, 0);
		auto buf = main.read(entry.compressed_size);
		
		if( entry.compressed_size == entry.uncompressed_size ){
			//std::cout << "Extracting without decompressing" << std::endl;
			//File((base_name + "/" + entry.fixedFilename()).c_str(), "wb").write(buf);
			//continue;
		}
		
		Stream in(buf.data(), buf.size());
		
		
		File((base_name + "/" + entry.fixedFilename() + ".raw").c_str(), "wb").write(buf);
		File out_file((base_name + "/" + entry.fixedFilename()).c_str(), "wb");
		
		
		
		
		while( !in.isEof() ){
			Stream out(buffer_out.get(), buffer_output_size);
			
			auto unknown1 = in.read16(); // Size of first block?
			auto unknown2 = in.peek();
			
			bool other_method = unknown2 != 0;
			if (!other_method)
				in.read8(); //hmm
			
			try{
				if (other_method)
					decompress2(in, out, unknown1);
				else{
					decompress1(in, out);
				}
			}
			catch(std::exception& e)
			{
				out_file.write(ArrayView{buffer_out.get(), out.pos});
				
				std::cout << "Failing at :" << in.pos << std::endl;
				throw std::runtime_error(e.what());
			}
			
			out_file.write(ArrayView{buffer_out.get(), out.pos});
			out_file.flush();
			
			if( in.isEof() )
				break;
			
			auto remaining_blocks = in.read8();
			if( remaining_blocks == 0 )
				break;
		}
	}
	return 0;
}