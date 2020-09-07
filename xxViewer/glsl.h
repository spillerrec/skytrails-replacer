const GLchar VERTEX_SRC[] = R"glsl(

#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec2 aWeights;
uniform mat4 MVP;
uniform mat4 projection;
uniform mat4 model;

out vec2 TexCoord;
out vec2 weights;

void main(){
  gl_Position = projection * MVP * model * vec4(vertexPosition_modelspace,1);
    TexCoord = aTexCoord;
	 weights = aWeights;
}


)glsl";


const GLchar FRAGMENT_SRC[] = R"glsl(

#version 330 core
out vec4 color;

in vec2 TexCoord;
in vec2 weights;

uniform sampler2D myTextureSampler;

void main(){
	//color = vec4(weights.x, weights.y, 0.0, 1.0);
	//color = vec4(weights.x, weights.x, weights.x, 1.0);
	//color = vec4(weights.y, weights.y, weights.y, 1.0);
	color = texture(myTextureSampler, TexCoord);
	if(color.a < 0.9)
		discard;
}

)glsl";


const size_t VERTEX_SRC_SIZE = sizeof(VERTEX_SRC);
const size_t FRAGMENT_SRC_SIZE = sizeof(FRAGMENT_SRC);

