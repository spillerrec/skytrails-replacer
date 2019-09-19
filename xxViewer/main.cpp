#include <stdio.h>
#include <stdlib.h>
extern "C"{
	#include <GL/glew.h>
}
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "utils/File.hpp"
#include "utils/ByteViewReader.hpp"
#include "nv_dds/nv_dds.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

namespace Falcom{
	struct Matrix{
		float values[4][4];
	};
	struct Texture{
		uint8_t unknown[72];
		char name[256];
		uint8_t unknown2[524];
	};
	struct TexRef{
		uint32_t unknown01; //mtlstart //Face start
		uint32_t unknown02; //mtlindex //Face amount
		uint32_t unknown03;
		uint32_t unknown04;
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
		uint32_t unknown29;
		float    unknown30;
	};
	struct Vertex{
		float x, y, z;
		float normal_x, normal_y, normal_z;
		float unknown1, unknown2;
		float u, v;
	};
	class Mesh{
		public:
			char name[256];
			uint32_t unknown1;
			uint32_t unknown2;
			uint32_t texture_count;
			
			std::vector<TexRef> texture_refferences;
			
			uint32_t vertices_count;
			std::vector<Vertex> vertices;
			
			uint32_t edge_count;
			std::vector<uint16_t> edges;
			
			float unknown3[11];
			
			void read( ByteViewReader& reader ){
				reader.readData<char>( name, 256 );
				unknown1 = reader.read32u();
				unknown2 = reader.read32u();
				
				texture_count = reader.read32u();
			//	std::cerr << "Texture_ref count: " << texture_count << '\n';
				reader.readVector( texture_refferences, texture_count );
			//	for( auto& ref : texture_refferences )
			//		std::cout << "TexRef:" << ref.unknown29 << "\n";
				
				vertices_count = reader.read32u();
			//	std::cerr << "Vertices count: " << vertices_count << '\n';
				reader.readVector( vertices, vertices_count );
				
				edge_count = reader.read32u();
			//	std::cerr << "Edge count: " << edge_count << '\n';
				reader.readVector( edges, edge_count );
				
				reader.readData<float>( unknown3, 11 );
			}
	};
	class Frame{
		public:
			char name[260];
			Matrix matrix;
			void read( ByteViewReader& reader ){
				reader.readStruct( *this );
			}
	};
	
	class SubModel{
		public:
			Frame frame;
			uint16_t texture_count;
			std::vector<Texture> textures;
			uint16_t mesh_count;
			std::vector<Mesh> meshes;
			
			void read( ByteViewReader& reader ){
				frame.read( reader );
				
				//Textures
				auto amount = reader.read16u();
				std::cout << "Texture count: " << amount << '\n';
				reader.readVector( textures, amount );
				for( auto texture : textures )
					std::cout << texture.name << '\n';
				
				//Meshes
				mesh_count = reader.read16u();
				std::cout << "Mesh count: " << mesh_count << '\n';
				meshes.resize(mesh_count);
				for( unsigned i=0; i<mesh_count; i++ )
					meshes[i].read( reader );
			}
	};
	class Model{
		public:
		Buffer data;
		ByteView unknown1; //bytes 48
		Frame root;
		ByteView unknown2; //bytes 55
		
		SubModel sub_model;
		
		Model( Buffer data );
	};
}

Falcom::Model::Model( Buffer data_1 ) : data( data_1 ){
	ByteViewReader reader( data );
	std::cout << "Starting :D \n";
	unknown1 = reader.read( 48 );
	root.read( reader );
	unknown2 = reader.read( 55 );
	/*
	Frame extra_frame;
	extra_frame.read( reader );
	reader.read( 211 );
	*/
	sub_model.read( reader );
}

class TextureHandler{
	public:	
		struct Texture{
			std::string filename;
			nv_dds::CDDSImage image;
			GLuint id;
		};
		std::vector<Texture> textures;
		
		
		TextureHandler( const Falcom::SubModel& m ) {
			for( auto& fal_tex : m.textures ){
				Texture t;
				t.filename = fal_tex.name;
				textures.push_back( t );
			}
			
			for( auto& tex : textures ){
			//	auto &tex = textures[0];
				auto path = "images/" + tex.filename;
				path = path.substr(0, path.size()-4) + ".dds";
				tex.image.load( path.c_str() );
				
				tex.id = -1;
				glGenTextures( 1, &tex.id );
				glBindTexture( GL_TEXTURE_2D, tex.id );
				
	// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Generate mipmaps, by the way.
				
				tex.image.upload_texture2D();
	glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
		
};

class Model{
	private:
		struct Vertex{
			GLfloat x, y, z;
			GLfloat u, v;
			//Weights??
			//Bones
			//Normals ??
			uint16_t id;
			
			bool operator<( const Vertex& other ) const{ return id < other.id; };
		};
		struct Face{
			uint16_t a, b, c;
			Face( uint16_t a, uint16_t b, uint16_t c )
				:	a(a), b(b), c(c) {}
		};
		
		std::vector<Vertex> vertices;
		std::vector<Face> faces;
		std::vector<Vertex*> vertex_lookup;
		uint32_t material_index;
		
		Vertex* getVertex( uint16_t id ) const{
			if( id >= vertex_lookup.size() ){
				std::cout << "ID too large: " << id << std::endl;
				return nullptr;
			}
			if( !vertex_lookup[id] ){
				std::cout << "Vertex not defined! " << id << std::endl;
			}
			return vertex_lookup[id];
		}
		
		void addMesh( const Falcom::Mesh& input );
		void createLookup();
		
	public:
		Model( const Falcom::Mesh& mesh ){
			addMesh( mesh );
			createLookup();
		}
		Model( const std::vector<Falcom::Mesh>& meshes ){
			for( auto& mesh : meshes )
				addMesh( mesh );
			createLookup();
		}
		
		void addFaces( std::vector<GLfloat>& points, std::vector<GLfloat>& uvs ) const;
};

void Model::addMesh( const Falcom::Mesh& input ){
	faces.reserve( faces.size() + input.edge_count/3 );
	for( unsigned i=0; i < input.edge_count/3; i++ )
		faces.emplace_back( input.edges[i*3+0], input.edges[i*3+1], input.edges[i*3+2] );
	
	vertices.reserve( vertices.size() + input.vertices_count );
	for( unsigned i=0; i < input.vertices_count; i++ ){
		Vertex v;
		v.id = i;
		v.x = input.vertices[i].x;
		v.y = input.vertices[i].y;
		v.z = input.vertices[i].z;
		v.u = input.vertices[i].u;
		v.v = input.vertices[i].v;
		vertices.emplace_back( v );
	}
};

void Model::createLookup(){
	//Construct lookup
	auto max_id = std::max_element( vertices.begin(), vertices.end() )->id;
	vertex_lookup.clear();
	vertex_lookup.resize( max_id+1, nullptr );
	for( auto& vertex : vertices ){
		if( vertex_lookup[vertex.id] )
			std::cout << "Warning, id already defined: " << vertex.id << std::endl;
		vertex_lookup[vertex.id] = &vertex;
	}
}

void Model::addFaces( std::vector<GLfloat>& points, std::vector<GLfloat>& uvs ) const{
	uvs.reserve( uvs.size() + faces.size() * 3 * 2 );
	points.reserve( points.size() + faces.size() * 3 * 3 );
	for( auto& face : faces ){
		/*
		auto addVertex = [&]( uint16_t id ){
				auto vertex = getVertex( id );
				if( !vertex )
					throw std::runtime_error( "Missing vertex lookup" );
				
				points.push_back( vertex->x );
				points.push_back( vertex->y );
				points.push_back( vertex->z );
				uvs.push_back( vertex->u );
				uvs.push_back( vertex->v );
			};
		addVertex( face.a );
		addVertex( face.b );
		addVertex( face.b );
		addVertex( face.c );
		addVertex( face.c );
		addVertex( face.a );
		/*/
		auto vertex_a = getVertex( face.a );
		auto vertex_b = getVertex( face.b );
		auto vertex_c = getVertex( face.c );
		if( vertex_a && vertex_b && vertex_c ){
			auto addVertex = [&]( const Vertex& v ){
					points.push_back( v.x );
					points.push_back( v.y );
					points.push_back( v.z );
					uvs.push_back( v.u );
					uvs.push_back( v.v );
				};
			addVertex( *vertex_a );
		//	addVertex( *vertex_b );
			addVertex( *vertex_b );
		//	addVertex( *vertex_c );
			addVertex( *vertex_c );
		//	addVertex( *vertex_a );
		}
		//*/
	}
}


void getMeshes( const Falcom::Model& model, std::vector<Model>& models ){
	for( auto& mesh : model.sub_model.meshes )
		models.emplace_back( mesh );
	//models.emplace_back( model.sub_model );
	/*
	if( frame.meshes.size() > 0 )	
	//	models.emplace_back( frame.meshes );
		for( auto& mesh : frame.meshes )
			models.emplace_back( mesh );
	
	for( auto& child : frame.children )
		getMeshes( child, models );
	*/
}

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            GLuint id, 
                            GLenum severity, 
                            GLsizei length, 
                            const GLchar *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

int main( int argc, char* argv[] ){
	if( argc != 2 ){
		std::cout << "Wrong amount of parameters\n";
		return -1;
	}
	Falcom::Model xx( File( argv[1], "rb" ).readAll() );
	std::vector<Model> models;
	getMeshes( xx, models );
	std::cout << "Amount of models: " << models.size() << std::endl;
	
	// Initialise GLFW
	if( !glfwInit() ){
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	} 
	
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	
	//Debug context
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	// Open a window and create its OpenGL context
	GLFWwindow* window; // (In the accompanying source code, this variable is global)
	window = glfwCreateWindow( 1024, 768, "Tutorial 01", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental=true; // Needed in core profile
	if( glewInit() != GLEW_OK ){
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	
	GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
		std::cout << "Could not start debug context!" << std::endl;
	
	
	GLuint programID = LoadShaders( "../vertex.glsl", "../fragment.glsl" );
	
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	
	// An array of 3 vectors which represents 3 vertices
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> uvs;
	
	for( auto& model : models ){
		model.addFaces( vertices, uvs );
	}
	for( auto& i : vertices ){
		i /= 10;
	}
	
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	
	GLuint uvsbuffer;
	glGenBuffers(1, &uvsbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvsbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(GLfloat), uvs.data(), GL_STATIC_DRAW);
	
	//Load textures
	TextureHandler tex_handler( xx.sub_model );	
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	//Set up a perspective view. This is needed for depth testing to work
	glm::perspective(glm::radians(45.0f), (float)1024/(float)768, 0.1f, 100.0f);

	//GLfloat i = 0;
	do{
		double xpos = 0, ypos = 0;
		glfwGetCursorPos(window, &xpos, &ypos);
		
		
		glm::mat4 MVP{ 1 };
		MVP = glm::rotate( MVP, (float)ypos / 300.f, glm::vec3(1,0,0) );
		MVP = glm::rotate( MVP, (float)xpos / 300.f, glm::vec3(0,1,0) );
	
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvsbuffer);
		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		// Use our shader
		glUseProgram(programID);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_handler.textures[0].id );
		glUniform1i(TextureID, 0);
		
		
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle
		//glDrawArrays(GL_LINES, 0, vertices.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		//i += 0.002;
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );
	
	return 0;
}