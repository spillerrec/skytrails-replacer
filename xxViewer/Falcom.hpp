#include <stdio.h>
#include <stdlib.h>

#include "utils/File.hpp"
#include "utils/ByteViewReader.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>

namespace Falcom{
	struct ArchiveEntry{
		char name[8];
		char dot;
		char extension[3];
		uint32_t unknown1; //Supposed to be a timestamp
		uint32_t compressed_size;
		uint32_t uncompressed_size;
		uint32_t unknown2;
		uint32_t timestamp;
		uint32_t offset;
	};
	struct Archive{
		char magic[10];
		uint16_t value; //0x001a
		std::vector<ArchiveEntry> entries;
		void read( ByteViewReader& reader ){
			reader.readData<char>(magic, 10);
			value = reader.read16u();
			//TODO: Check validity
			reader.readVector(entries, reader.read32u());
		}
	};
	
	class Bones{
	private:
		uint32_t count1 = 0;
		uint32_t count2 = 0;
		uint32_t count3 = 0;
		ByteView bones;
		ByteView names;
	public:
		void read( ByteViewReader& reader ){
			count1 = reader.read32u();
			count2 = reader.read32u();
			count3 = reader.read32u();
			bones = reader.read(count3 * 4 * 16);
			names  = reader.read(count3 * 256);
		}
		
	};
	
	struct Matrix{
		float values[4][4];
	};
	struct Texture{
		uint32_t unknown1;
		float matrix[16];
		uint32_t unknown2;
		char name[204];
		char bumpmap[204];
		uint32_t unknown3[93];
	};
	struct TextureV1{
		uint32_t edge_start;
		uint32_t edge_count;
		uint8_t unknown[176];
		char name[256];
		uint8_t unknown2[104];
	};
	struct TexRef{
		uint32_t edge_start; //mtlstart //Face start
		uint32_t edge_count; //mtlindex //Face amount
		uint32_t unknown03; //Same, but with vertices?
		uint32_t unknown04; // ... count
		uint32_t unknown05;
		uint32_t unknown06;
		uint32_t unknown07;
		uint32_t unknown08;
		uint32_t unknown09;
		uint32_t unknown10;
		uint32_t unknown11;
		uint32_t unknown12;
		uint32_t unknown13;
		uint32_t unknown14;
		uint32_t unknown15;
		uint32_t unknown16;
		uint32_t unknown17;
		uint32_t unknown18;
		uint32_t unknown19;
		uint32_t unknown20;
		uint32_t unknown21;
		uint32_t unknown22;
		uint32_t unknown23;
		uint32_t unknown24;
		uint32_t unknown25;
		uint32_t unknown26;
		uint32_t unknown27;
		uint32_t unknown28;
		uint32_t id; //Still unsure about this
		uint32_t unknown30; //Doesn't know the format of this
	};
	struct Vertex{
		float x, y, z;
		float normal_x, normal_y, normal_z;
		float unknown1, unknown2;
		float u, v;
	};
	struct Vertex48{
		float x, y, z;
		float normal_x, normal_y, normal_z;
		float unknown1, unknown2;
		float unknown3, unknown4;
		float u, v;
	};
	class Mesh{
		public:
			char name[256];
			uint32_t unknown1;
			uint32_t vertex_size;
			
			std::vector<TexRef> texture_refferences;
			std::vector<TextureV1> textures;
			
			std::vector<Vertex> vertices;
			std::vector<Vertex48> vertices48;
			std::vector<uint16_t> edges;
			Bones bones;
			
			float unknown3[10];
			uint32_t unknown11;
			
			void read( ByteViewReader& reader, int version ){
				reader.readData<char>( name, 256 );
				unknown1 = reader.read32u();
				vertex_size = reader.read32u();
				
				auto texture_count = reader.read32u();
				if( version >= 2 )
					reader.readVector( texture_refferences, texture_count );
				else
					reader.readVector( textures, texture_count );
				
				auto vertices_count = reader.read32u();
				if (vertex_size != 40 && vertex_size != 48)
				{
					std::cout << "Does not yet support files with vertex_size == " << vertex_size << "\n";
					throw std::runtime_error("Does not support vertex_size == " + std::to_string(vertex_size));
					//std::exit(-1);
				}
				
				if( vertex_size == 40 )
					reader.readVector( vertices, vertices_count );
				else
					reader.readVector( vertices48, vertices_count );
				
				auto edge_count = reader.read32u();
				reader.readVector( edges, edge_count );
				
				if( vertex_size == 48 )
					bones.read(reader);
				
				reader.readData<float>( unknown3, 10 );
				unknown11 = reader.read32u();
				if (unknown11 > 0){
					//std::cout << "Mesh unknown: " << unknown11 << std::endl;
					reader.read(4 * unknown11 * 26);
				}
			}
			
			void debug( int level ){
				if( unknown1 != 466 ){
					for( int i=0; i<level; i++ )
						std::cout << "\t";
					std::cout << "Mesh: " << name << " (" << unknown1 << ")\n";
				}
			}
	};
	
	class UnknownType{
	private:
		ByteView u1;
		uint32_t count = 0;
		ByteView u2;
		ByteView u3;
		ByteView u4;
		ByteView u5;
	public:
		void read( ByteViewReader& reader ){
			u1 = reader.read(5*4);
			count = reader.read32u();
			auto unknown6 = reader.read32u();
			auto unknown7 = reader.read32u();
			
			auto unknown8 = reader.read32u();
			u3 = reader.read(unknown8 * 4*4);
			
			auto count2 = reader.read32u();
			u4 = reader.read(count2 * 4*5);
			auto count3 = reader.read32u();
			u5 = reader.read(count3 * 4*4);
			auto unknown9 = reader.read32u();
			if (unknown9 != 0)
				std::cout << "Unknown9 is not 0\n";
		}
		
	};
	
	class SubModel{
		public:
			char name[260];
			Matrix matrix;
			std::vector<Texture> textures;
			std::vector<Mesh> meshes;
			uint8_t unknown1;
			UnknownType unknown_block1;
			ByteView random_padding; //48 bytes if null frame, I'm missing something here, but it fixes a bunch of files for now
			uint16_t children_count;
			std::vector<SubModel> children;
			
			
			void read( ByteViewReader& reader, int version ){
				reader.readData<char>( name, 260 );
				reader.readStruct( matrix );
				
				//Textures
				if( version >= 2 ){
					auto amount = reader.read16u();
					reader.readVector( textures, amount );
				}
				
				//Meshes
				auto mesh_count = reader.read16u();
				//std::cout << "Mesh count: " << mesh_count << '\n';
				meshes.resize(mesh_count);
				for( unsigned i=0; i<mesh_count; i++ )
					meshes[i].read( reader, version );
				
				unknown1 = reader.read8u();
				unknown_block1.read( reader );
				
				children_count = reader.read16u();
				children.resize( children_count );
				for( auto& child : children )
					child.read( reader, version );
			}
			
			int totalChildren() const{
				int sum = children_count;
				for( auto& child : children )
					sum += child.totalChildren();
				return sum;
			}
			
			void debug( int level ){
				for( int i=0; i<level-1; i++ )
					std::cout << "| ";
				if( level > 0 )
					std::cout << "|--";
				std::cout << /*"Frame: " <<*/ name << " (" << meshes.size() << " meshes)\n";
				for( auto& mesh : meshes )
					mesh.debug( level + 1 );
				for( auto& child : children )
					child.debug( level + 1 );
			}
		
			template<typename Func>
			void forEachFrame(Func f){
				f(*this);
				for(auto& frame : children)
					frame.forEachFrame(f);
			}
			
			template<typename Func>
			void forEachMesh(Func f){
				for(auto& mesh : meshes)
					f(mesh);
				for(auto& frame : children)
					frame.forEachMesh(f);
			}
	};
	class Model{
		public:
		Buffer data;
		
		uint8_t type;
		uint8_t version;
		ByteView unknown1; //bytes 46
		uint16_t children_count;
		ByteView remainder;
		
		std::vector<SubModel> frames;
		
		Model( Buffer data );
		
		std::vector<int> scanForFrames(){
			ByteViewReader readFrames( data );
			std::vector<int> positions;
			uint8_t key[5] = {'F', 'r', 'a', 'm', 'e'}; 
			while(readFrames.findNextMatch(ConstByteView(key, 5))){
				positions.push_back(readFrames.tell());
				readFrames.read(256);
			}
			return positions;
		}
		void dumpFrames(){
			auto positions = scanForFrames();
			for (int i=0; i<(int)positions.size(); i++){
				auto start = positions[i];
				auto end = (i != positions.size()-1) ? positions[i+1] : data.view().size();
				auto frame_data = data.view().subView(start, end-start);
				
				std::cout << "Dumping frame " << i << " at: " << start << std::endl;
				auto file = fopen(("dump" + std::to_string(i) + ".frame").c_str(), "wb");
				fwrite(frame_data.begin(), 1, frame_data.size(), file);
				fclose(file);
			}
		}
		
		int childrenTotal(){
			int childrenTotal = 0;
			for( auto& frame : frames )
				childrenTotal += 1 + frame.totalChildren();
			return childrenTotal;
		}
		
		void debug(){
			std::cout << "Model with " << frames.size() << " roots:\n";
			for( auto& frame : frames )
				frame.debug( 0 );
			
			std::cout << "Read frames " << childrenTotal() << " vs. " << scanForFrames().size() << " expected\n";
			
			std::cout << "Bytes left in file: " << remainder.size() << std::endl;
		}
		
		bool validate(){
			bool ok = true;
			ok &= (remainder.size() == 40);
			//ok &= (childrenTotal() == scanForFrames().size()); //Sometimes picks up unitilalized stuff in Meshes names
			return ok;
		}
		
		template<typename Func>
		void forEachFrame(Func f){
			for(auto& frame : frames)
				frame.forEachFrame(f);
		}
		
		template<typename Func>
		void forEachMesh(Func f){
			for(auto& frame : frames)
				frame.forEachMesh(f);
		}
	};
}

