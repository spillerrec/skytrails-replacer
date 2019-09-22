#version 330 core
out vec4 color;

in vec2 TexCoord;

uniform sampler2D myTextureSampler;

void main(){
	color = texture(myTextureSampler, TexCoord);
	if(color.a < 0.5)
		discard;
}