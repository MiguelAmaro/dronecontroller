//~VERT SHADER
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * vec4(model[3][0] * vertex.x, model[3][1] * vertex.y, 0.0, 1.0);
}


//~FRAG SHADER
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite_texture;
uniform vec3 spriteColor;

void main()
{    
    color = vec4(spriteColor, 1.0) * texture(sprite_texture, TexCoords);
}

