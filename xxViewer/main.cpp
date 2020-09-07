#include "Falcom.hpp"

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

#include <png++/png.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>

#include "glsl.h"

static int error( const char* msg, int code = -1 ){
	std::cout << "Error: " << msg << '\n';
	return code;
}

GLuint LoadShaders(){
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	auto compile_shader = [&]( GLuint& id, auto* src ){
		glShaderSource(id, 1, &src, NULL);
		glCompileShader(id);

		// Check Vertex Shader
		glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
			glGetShaderInfoLog(id, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("%s\n", &VertexShaderErrorMessage[0]);
		}
	};

	// Compile Vertex Shader
	printf("Compiling vertex shader\n");
	compile_shader( VertexShaderID, VERTEX_SRC );

	// Compile Fragment Shader
	printf("Compiling fragment shader\n");
	compile_shader( FragmentShaderID, FRAGMENT_SRC );

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

png::image< png::rgba_pixel > loadImage(std::string path){
	/* Try loading AI upscaled images
	try{
		return png::image< png::rgba_pixel >{"images-upscaled/" + path};
	}
	catch(...)
	{
		std::cout << "Error, could not load upscaled texture: " << path.c_str() << std::endl;
	}//*/
	
	return png::image< png::rgba_pixel >{"images/" + path};
}

class TextureHandler{
	public:	
		struct Texture{
			std::string filename;
			GLuint id = -1;
			Texture(std::string filename) : filename(filename) {
				auto pos = filename.find_last_of("/\\");
				if (pos != std::string::npos)
				{
					std::cout << "Warning, removing garbage from filename: " << filename << std::endl;
					this->filename = filename.substr(pos);
				}
			}
		};
		std::vector<Texture> textures;
		
		void loadAll(){
			for( auto& tex : textures ){
				tex.id = -1;
				auto path = tex.filename;
				path = path.substr(0, path.size()-4) + ".png";
				try{
					auto image = loadImage(path);
					
					auto buffer = std::make_unique<uint8_t[]>(image.get_height()*image.get_width()*4);
					for (png::uint_32 y = 0; y < image.get_height(); ++y)
						for (png::uint_32 x = 0; x < image.get_width(); ++x)
						{
							auto pix = image[y][x];
							auto out = buffer.get() + 4*(x + y*image.get_width());
							if(pix.red == 255 && pix.green==0 && pix.blue==0)
								pix.red = pix.alpha = 0;
							out[0] = pix.red;
							out[1] = pix.green;
							out[2] = pix.blue;
							out[3] = pix.alpha;
						}

					glGenTextures( 1, &tex.id );
					glBindTexture( GL_TEXTURE_2D, tex.id );
					
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.f);
					
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.get_width(), image.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get());
					
					glGenerateMipmap(GL_TEXTURE_2D);
				}
				catch(...)
				{
					std::cout << "Error, could not load texture: " << path.c_str() << std::endl;
				}
			}
		}
		
		TextureHandler( const Falcom::SubModel& m ) {
			for( auto& fal_tex : m.textures )
				textures.emplace_back( fal_tex.name );
			loadAll();
		}
		
		TextureHandler( const Falcom::TextureV1& tex ) {
			textures.emplace_back( tex.name );
			loadAll();
		}
		
};

class Model{
	private:
		struct Vertex{
			GLfloat x, y, z;
			GLfloat u, v;
			GLfloat w1, w2;
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
		
		uint32_t edge_start;
		uint32_t edge_count;
		
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
		GLuint material_index;
		Model( const Falcom::Mesh& mesh, const Falcom::TexRef& ref, TextureHandler& handler ){
			addMesh( mesh );
			material_index = handler.textures.at(ref.id).id;
			edge_start = ref.edge_start;
			edge_count = ref.edge_count;
			createLookup();
		}
		Model( const Falcom::Mesh& mesh, const Falcom::TextureV1& texture ){
			TextureHandler handler(texture);
			addMesh( mesh );
			material_index = handler.textures.at(0).id;
			edge_start = texture.edge_start;
			edge_count = texture.edge_count;
			createLookup();
		}
		
		void addFaces( std::vector<GLfloat>& points, std::vector<GLfloat>& uvs, std::vector<GLfloat>& weights ) const;

		
		void find_boundaries(float& x_min, float& y_min, float& z_min, float& x_max, float& y_max, float& z_max){
			for(auto v : vertices){
				x_min = std::min(x_min, v.x);
				y_min = std::min(y_min, v.y);
				z_min = std::min(z_min, v.z);
				x_max = std::max(x_max, v.x);
				y_max = std::max(y_max, v.y);
				z_max = std::max(z_max, v.z);
			}
		}
		void offset(float x, float y, float z, float scale){
			for(auto& v : vertices){
				v.x += x;
				v.y += y;
				v.z += z;
				v.x /= scale;
				v.y /= scale;
				v.z /= scale;
			}
		}
};

void Model::addMesh( const Falcom::Mesh& input ){
	faces.reserve( faces.size() + input.edges.size()/3 );
	for( unsigned i=0; i < input.edges.size()/3; i++ )
		faces.emplace_back( input.edges[i*3+0], input.edges[i*3+1], input.edges[i*3+2] );
	
	if( input.vertices.size() > input.vertices48.size() ){
		
		vertices.reserve( vertices.size() + input.vertices.size() );
		for( unsigned i=0; i < input.vertices.size(); i++ ){
			auto& in = input.vertices[i];
			Vertex v;
			v.id = i;
			v.x = in.x;
			v.y = in.y;
			v.z = in.z;
			v.u = in.u;
			v.v = in.v;
			v.w1 = 0.f;
			v.w2 = 0.f;
			vertices.emplace_back( v );
		}
	}
	else{
		vertices.reserve( vertices.size() + input.vertices.size() );
		for( unsigned i=0; i < input.vertices48.size(); i++ ){
			Vertex v;
			v.id = i;
			v.x = input.vertices48[i].x;
			v.y = input.vertices48[i].y;
			v.z = input.vertices48[i].z;
			v.u = input.vertices48[i].u;
			v.v = input.vertices48[i].v;
			v.w1 = input.vertices48[i].unknown3;
			v.w2 = input.vertices48[i].unknown4;
			//std::cout << v.w1 << "  " << v.w2 << std::endl;
			vertices.emplace_back( v );
		}
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

void Model::addFaces( std::vector<GLfloat>& points, std::vector<GLfloat>& uvs, std::vector<GLfloat>& weights ) const{
	uvs.reserve( uvs.size() + faces.size() * 3 * 2 );
	weights.reserve( weights.size() + faces.size() * 3 * 2 );
	points.reserve( points.size() + faces.size() * 3 * 3 );
	for( uint32_t i=0; i<edge_count; i++ ){
		auto& face = faces[i + edge_start];
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
					weights.push_back( v.w1 );
					weights.push_back( v.w2 );
				};
			addVertex( *vertex_a );
			addVertex( *vertex_b );
			addVertex( *vertex_c );
		}
	}
}

void getMeshes( const Falcom::SubModel& submodel, std::vector<Model>& models ){
	
	TextureHandler tex_handler( submodel );	//TODO: This is the wrong place to have this, as we can't clean up properly
	for( auto& mesh : submodel.meshes )
	{
		for( auto& texref : mesh.texture_refferences )
			models.emplace_back( mesh, texref, tex_handler );
		for( auto& texture : mesh.textures )
			models.emplace_back( mesh, texture );
	}
	for( auto& child : submodel.children )
		getMeshes( child, models );
}

void getMeshes( const Falcom::Model& model, std::vector<Model>& models ){
	for( auto& frame : model.frames )
		getMeshes( frame, models );
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

class MtlHandler{
private:
	FILE* file = nullptr;
	std::string path;
	
public:
	MtlHandler(std::string path) : path(path){
		file = fopen(path.c_str(), "a");
	}
	~MtlHandler(){ fclose(file); }
	
	std::string pathToId( std::string name ){
		std::replace(name.begin(), name.end(), '.', '_' );
		return name;
	}
	
	void writeRef( FILE* parent ){
		fprintf(parent, "mtllib %s\n", path.c_str());
	}
	
	void useTexture( FILE* parent, Falcom::Texture& tex ){
		fprintf(parent, "usemtl %s\n", pathToId(tex.name).c_str());
	}
	
	void writeTexture( std::string name ){
		auto path = name.substr(0, name.size()-4) + ".png";
		fprintf(file, "newmtl %s\n", pathToId( name ).c_str() );
		fprintf(file, "d 1.0\n");
		fprintf(file, "illum 2\n");
		fprintf(file, "map_Kd images\\\\%s\n\n", path.c_str());
	}
};

void writeMeshToObj( std::string path, int count, MtlHandler& handler, Falcom::SubModel& parent, Falcom::Mesh& mesh, int& vertex_offset ){
	auto* file = fopen(path.c_str(), "a");
	if( !file )
		return;

	static bool has_written = false;
	if (!has_written)
		handler.writeRef( file );
	has_written = true;
	
	fprintf(file, "o %s_%d\n", mesh.name, count);
	
	for( auto v : mesh.vertices ){
		fprintf(file, "v %f %f %f\n", v.x, v.y, v.z);
	}
	for( auto v : mesh.vertices ){
		fprintf(file, "vt %f %f\n", v.u, 1.0-v.v);
	}

	for( auto& texref : mesh.texture_refferences ){
		handler.useTexture( file, parent.textures[texref.id] );
		for( auto i=0; i<texref.edge_count; i++ ){
			auto* edge = mesh.edges.data() + (i+texref.edge_start)*3;
			fprintf(file, "f %d/%d %d/%d %d/%d\n", vertex_offset+edge[0]+1, vertex_offset+edge[0]+1, vertex_offset+edge[1]+1, vertex_offset+edge[1]+1, vertex_offset+edge[2]+1, vertex_offset+edge[2]+1);
		}
	}
	
	vertex_offset += mesh.vertices.size();
	
	
	fclose( file );
}
void writeMeshToObj( Falcom::SubModel& model, MtlHandler& handler, int& count, int& vertex_offset ){
	for( auto& tex : model.textures )
		handler.writeTexture( tex.name );
	
	for( auto& mesh : model.meshes ){
		writeMeshToObj( std::string("mesh") /*+ std::to_string(count++)*/ + ".obj", count++, handler, model, mesh, vertex_offset );
	}
	for( auto& child : model.children )
		writeMeshToObj( child, handler, count, vertex_offset );
}

template<typename T>
class Statistics{
private:
	std::set<T> items;
	
public:
	void add(T val){ items.insert( val ); }
	
	void evaluate(std::string msg){
		if( items.size() == 1 && *items.begin() == T() )
			return;
		
		std::cout << msg << ": ";
		for(auto val : items)
			std::cout << val << ", ";
		std::cout << std::endl;
	}
};

constexpr int window_width = 1920;
constexpr int window_height = 1080;
int main( int argc, char* argv[] ){
	if( argc != 2 )
		return error( "Wrong amount of parameters" );
	Falcom::Model xx( File( argv[1], "rb" ).readAll() );
	xx.debug();
	/*
	constexpr size_t stat_amount = 93;
	for(size_t i=0; i<stat_amount; i++){
		Statistics<float> stats;
		xx.forEachFrame([&](auto& frame){
			for(auto& tex : frame.textures)
				stats.add(tex.unknown3[i]);
		});
		stats.evaluate("Texture.unknown" + std::to_string(i));
	}
	{
		Statistics<uint32_t> stats1;
		Statistics<float> stats2;
		xx.forEachFrame([&](auto& frame){
			for(auto& tex : frame.textures){
				stats1.add(tex.unknown1);
				stats2.add(tex.unknown2);
			}
		});
		stats1.evaluate("Texture.first");
		stats2.evaluate("Texture.second");
	}
	*/
//	int count = 0, vertex_offset = 0;
//	MtlHandler mtl_handler( "textures.mtl");
//	for( auto& frame : xx.frames )
//		writeMeshToObj( frame, mtl_handler, count, vertex_offset );
	
	
	// Initialise GLFW
	if( !glfwInit() )
		return error( "Failed to initialize GLFW" );
	
	glEnable( GL_MULTISAMPLE );
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGl 4.3 needed for debug context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	
	//Debug context
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	// Open a window and create its OpenGL context
	auto* window = glfwCreateWindow( window_width, window_height, "TrailsViewer", NULL, NULL);
	if( !window ){
		glfwTerminate();
		return error( "Couldn't open GLFW window" );
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if( glewInit() != GLEW_OK )
		return error( "Failed to initialize GLEW" );
	
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
	
	
	GLuint programID = LoadShaders();
	
	
	struct GlModel{
		GLuint vertexbuffer;
		GLuint uvsbuffer;
		GLuint weightsbuffer;
		GLuint texture_id;
		
		// An array of 3 vectors which represents 3 vertices
		std::vector<GLfloat> vertices;
		std::vector<GLfloat> uvs;
		std::vector<GLfloat> weights;
	};
	std::vector<GlModel> gl_models;
	
	
	std::vector<Model> models;
	getMeshes( xx, models );
	struct MinMax{
		float min {std::numeric_limits<float>::max()};
		float max {std::numeric_limits<float>::min()};
		auto range() { return (max-min); }
		auto offset() { return -(min + range()/2); }
	} x, y, z;
	for( auto& model : models )
		model.find_boundaries(x.min, y.min, z.min, x.max, y.max, z.max);
	auto scale = 1.0;//std::max(x.range(), std::max(y.range(), z.range()));
	for( auto& model : models ) //TODO: Move this to modelMat
		model.offset(x.offset(), y.offset(), z.offset(), scale);
	for( auto& model : models ){
		GlModel m;
		model.addFaces( m.vertices, m.uvs, m.weights );
		
		glGenBuffers(1, &m.vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m.vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, m.vertices.size()*sizeof(GLfloat), m.vertices.data(), GL_STATIC_DRAW);
		
		glGenBuffers(1, &m.uvsbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m.uvsbuffer);
		glBufferData(GL_ARRAY_BUFFER, m.uvs.size()*sizeof(GLfloat), m.uvs.data(), GL_STATIC_DRAW);
		
		glGenBuffers(2, &m.weightsbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m.weightsbuffer);
		glBufferData(GL_ARRAY_BUFFER, m.weights.size()*sizeof(GLfloat), m.weights.data(), GL_STATIC_DRAW);
		
		m.texture_id = model.material_index;
		
		gl_models.push_back(std::move(m));
	}
	
	// Get uniforms
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ProjectionID = glGetUniformLocation(programID, "projection");
	GLuint ModelMatID = glGetUniformLocation(programID, "model");
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	//glEnable(GL_DEPTH_CLAMP);
	//Set up a perspective view. This is needed for depth testing to work

	bool exit_program = false;
	double old_xpos = 0, old_ypos = 0;
	glfwGetCursorPos(window, &old_xpos, &old_ypos);
	float rot_x = 0.3, rot_y = -0.1, rot_z = 0;
	float mov_x = 0, mov_y = 0, mov_z = 0;
	float view_scale = 1.0;
	float yaw = 0, pitch = 0;
	glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  0.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	{
			glm::vec3 front;
			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			cameraFront = glm::normalize(front);
	}
	do{
		auto projection = glm::perspective(glm::radians(45.0f), (float)window_width/(float)window_height, 0.1f, 1000.f);
		glm::mat4 modelMat = glm::scale( glm::mat4(1.0f), glm::vec3(-1, 1, 1) ); //Fix view getting mirroed for unknown reasons
	
		//Get mouse movement
		double xpos = 0, ypos = 0;
		double dx, dy;
		glfwGetCursorPos(window, &xpos, &ypos);
		dx = xpos - old_xpos;
		dy = ypos - old_ypos;
		old_xpos = xpos;
		old_ypos = ypos;
		
		
		if( glfwGetKey(window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS )
		{
			cameraPos += cameraFront * (float(dy) / 100.f);
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * (float(dx) / 100.f);
			mov_x += dx / 100.f;
			mov_y += dy / 100.f;
		}
		else if( glfwGetKey(window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS )
		{
			yaw += dx / 10.f;
			pitch += dy / 10.f;
			pitch = std::max( -89.f, std::min(pitch, 89.f) );

			glm::vec3 front;
			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			cameraFront = glm::normalize(front);
			
			auto dist = dx / 10.f;
			view_scale *= 1.0 - dist;
		}
		else if( glfwGetKey(window, GLFW_KEY_LEFT_ALT ) == GLFW_PRESS )
		{
			rot_x += dx / 300.f;
			rot_y += dy / 300.f;
		}
		modelMat = glm::translate( modelMat, glm::vec3( rot_x, 0.f, rot_y ) );
		
		//Close the program when pressing Esc
		if( glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
			exit_program = true;
		
		/*
		glm::mat4 MVP{ 1 };
		MVP = glm::translate( MVP, glm::vec3(mov_x, mov_y, mov_z) );
		MVP = glm::scale( MVP, glm::vec3(view_scale, view_scale, view_scale) );
		MVP = glm::rotate( MVP, rot_x, glm::vec3(0,1,0) );
		MVP = glm::rotate( MVP, rot_y, glm::vec3(1,0,1) );
		MVP = glm::rotate( MVP, rot_z, glm::vec3(0,0,1) );
		/*/
		auto MVP = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		//*/
	
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//for( auto& m : gl_models ){
		for(size_t i=0; i<gl_models.size(); i++){
			auto& m = gl_models[i];
			if( m.texture_id == -1 )
				continue;
			
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, m.vertexbuffer);
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
			glBindBuffer(GL_ARRAY_BUFFER, m.uvsbuffer);
			glVertexAttribPointer(
				1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				2,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, m.weightsbuffer);
			glVertexAttribPointer(
				2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				2,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			
			// Use our shader
			glUseProgram(programID);
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &projection[0][0]);
			glUniformMatrix4fv(ModelMatID, 1, GL_FALSE, &modelMat[0][0]);
			
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m.texture_id );
			glUniform1i(TextureID, 0);
			
			
			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, m.vertices.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle
			//glDrawArrays(GL_LINES, 0, m.vertices.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while( !exit_program && glfwWindowShouldClose(window) == 0 );
	
	return 0;
}