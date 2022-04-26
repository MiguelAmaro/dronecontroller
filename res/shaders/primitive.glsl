
//~VERT SHADER
#version 460 core
layout(location = 0) in vec2 VertexIn;

uniform vec4 ColorIn ;
uniform mat4 Transform;

out vec4 Color;

void main()
{
  vec4 NewVertPos = Transform * vec4(VertexIn.xy, 0.0f, 1.0f);
  // This is because my projection matix fuck with
  // z values for some reason.
  NewVertPos.z = 0.0f;
  
  gl_Position = NewVertPos;
  Color = ColorIn;
}


//~FRAG SHADER
#version 460 core

in  vec4 Color;
out vec4 FragColor;

void main()
{    
  FragColor = Color; //vec4(1.0f, 0.0f, 1.0f, 1.0f);
}  
