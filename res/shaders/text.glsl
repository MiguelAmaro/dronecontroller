
//~VERT SHADER
#version 460 core
layout(location = 0) in vec4 vertex; // (vec2: pos | vec2: tex)
layout(location = 2)uniform mat4 Transform;

out vec2 TexCoords;

void main()
{
    
    vec4 NewPos = Transform * vec4(vertex.xy, 0.0f, 1.0f);
    // This is because my projection matix fuck with
    // z values for some reason.
    NewPos.z = 0.0f;
    TexCoords = vertex.zw;
    
    gl_Position = NewPos;
}


//~FRAG SHADER
#version 460 core

out vec4 FragColor;
uniform float ThrottleValue;
uniform vec2  UISize;
uniform vec2  UIPos;
uniform vec2  WindowSize;

in  vec2 TexCoords;

uniform sampler2D Text;
uniform vec3 TextColor;

void main()
{    
    vec4 Sampled = vec4(1.0f, 1.0f, 1.0f, texture(Text, TexCoords).r);
    FragColor = vec4(TextColor, 1.0f) * Sampled;
}  
