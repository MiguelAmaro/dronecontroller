
//~VERT SHADER
#version 460 core
layout(location = 0) in vec2 vertex;

uniform mat4 Transform;


void main()
{
    
    vec4 NewPos = Transform * vec4(vertex.xy, 0.0f, 1.0f);
    // This is because my projection matix fuck with
    // z values for some reason.
    NewPos.z = 0.0f;
    
    gl_Position = NewPos;
}


//~FRAG SHADER
#version 460 core

out vec4 FragColor;
uniform float ThrottleValue;
uniform vec2 UISize;
uniform vec2 UIPos;
uniform vec2 WindowSize;

void main()
{
    float UITop    = UIPos.y + (UISize.y * 0.5f);
    float UIBottom = UIPos.y - (UISize.y * 0.5f);
    float UILeft   = UIPos.x - (UISize.x * 0.5f);
    float UIRight  = UIPos.x + (UISize.x * 0.5f);
    float ClipSpaceCoordsX = (UIRight - gl_FragCoord.x) / UISize.x;
    float ClipSpaceCoordsY = (UITop   - gl_FragCoord.y) / UISize.y;
    ClipSpaceCoordsX = (1.0f - ClipSpaceCoordsX);
    
    float ThrottleMask = float(ThrottleValue > ClipSpaceCoordsX);
    
    vec4 Color;
    Color += vec4(ThrottleValue, 1.0f - ClipSpaceCoordsX, 0.0f, 1.0f) * ThrottleMask;
    Color += vec4(0.3f, 0.0f, 0.3f, 0.08f) * (1.0 - ThrottleMask);
    
    FragColor = vec4(Color);
}