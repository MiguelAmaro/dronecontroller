
//~VERT SHADER
#version 460 core
layout (location = 0) in vec2 vertex;

uniform vec2 MousePos;
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
uniform vec2 WindowSize;


float plot(vec2 st, float pct)
{
    return (smoothstep( pct, pct, st.y + 0.004f) -
            smoothstep( pct, pct, st.y + 0.000f));
}


void main()
{
    vec2 st = gl_FragCoord.xy / UISize;
    
    float y = ThrottleValue;
    
    vec3 color = vec3(0.4f);
    
    //Plot a line
    float pct = plot(st, y);
    color += pct * vec3(1.0f, 0.0f, 0.4f);
    
    FragColor = vec4(color, 1.0f);
}