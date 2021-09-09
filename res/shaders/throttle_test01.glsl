
//~VERT SHADER
#version 460 core
layout (location = 0) in vec2 vertex;

uniform mat3 Transform;


void main()
{
    
    gl_Position = vec4((Transform * vec3(vertex, 1.0f)), 1.0f);
}


//~FRAG SHADER
#version 460 core

out vec4 FragColor;
uniform float ThrottleValue;
uniform vec2 UISize;
uniform vec2 UIPos;
uniform vec2 WindowSize;

float box(in vec2 _st, in vec2 _size){
    _size = vec2(0.5) - _size*0.5;
    vec2 uv = smoothstep(_size,
                         _size+vec2(0.001),
                         _st);
    uv *= smoothstep(_size,
                     _size+vec2(0.001),
                     vec2(1.0)-_st);
    return uv.x*uv.y;
}

float cross(in vec2 _st, float _size){
    return  box(_st, vec2(_size,_size/4.)) +
        box(_st, vec2(_size/4.,_size));
}

vec3 DrawGuage(vec2 st, vec2 BoxPos, vec2 BoxDim, vec2 BarPos, vec2 BarDim, float Throttle)
{
    /// BOX
    vec2  Box_BottomLeft = step(BoxPos - (BoxDim / 2.0f), st);
    float Box_Percentage = Box_BottomLeft.x * Box_BottomLeft.y;
    
    vec2 Box_TopRight = step(1.0f - (BoxPos + (BoxDim / 2.0f)),1.0f-st);
    Box_Percentage   *= Box_TopRight.x * Box_TopRight.y;
    
    vec3 Box_Color = vec3(Box_Percentage) * vec3(0.4f, 0.4f, 0.4f);
    
    /// BAR
    vec2 Bar_BottomLeft  = step(BarPos - (BarDim / 2.0f),st);
    float Bar_Percentage = Bar_BottomLeft.x * Bar_BottomLeft.y;
    
    vec2 Bar_TopRight = step(1.0f - (BarPos + (BarDim / 2.0f)),1.0f-st);
    Bar_Percentage   *= Bar_TopRight.x * Bar_TopRight.y;
    
    vec3 Bar_Color = vec3(Bar_Percentage) * vec3(0.2f, 0.8f, 0.0f);
    
    return Bar_Color + Box_Color;
}

void main()
{
    vec2 st = gl_FragCoord.xy / WindowSize.xy;
    vec3 color = vec3(0.0);
    /*
    // To move the cross we move the space
    vec2 translate = vec2(0.5f);
    st += translate*0.35f;*/
    
    // Show the coordinates of the space on the background
    color = vec3(st.x,st.y,0.0);
    
    // TODO(MIGUEL): what transfrom to do to use the DrawGuage
    
    vec2 BoxPos = vec2(UIPos.x / WindowSize.x, UIPos.y / WindowSize.y);
    vec2 BoxDim = (UISize / WindowSize) * 1.0f;
    vec2 BarPos = vec2(0.5f, ThrottleValue);
    vec2 BarDim = vec2(BoxDim.x, BoxDim.y / 4.0f);
    
    // Add the shape on the foreground
    color += vec3(cross(st,0.4f));
    color += DrawGuage(st, BoxPos, BoxDim, BarPos, BarDim, ThrottleValue);
    FragColor = vec4(color, 1.0f);
}