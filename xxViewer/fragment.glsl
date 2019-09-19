#version 330 core
out vec4 color;

in vec2 TexCoord;

uniform sampler2D myTextureSampler;

void main(){
  color = texture(myTextureSampler, TexCoord);
  //color = texture(myTextureSampler, vec2(0.5,0.5));
  //color = vec4(TexCoord.x, TexCoord.y, 0, 1);
}