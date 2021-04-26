//~VERT SHADER
#version 460 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

void main()
{
    gl_Position = vec4(vertex.x, vertex.y, 0.0f, 0.0f);
}


//~FRAG SHADER
#version 460 core
out vec4 FragColor;

uniform float throttle_value;
uniform uvec2 size;

float plot(vec2 st) 
{    
    return smoothstep(0.0001, 0.0, abs(st.y - st.x));
}


void main()
{
    vec2 st = gl_FragCoord.xy / size;
    
    float y = st.x;
    
    vec3 color = vec3(y);
    
    // Plot a line
    float pct = plot(st);
    color = (1.0-pct)*color+pct*vec3(0.0,throttle_value,0.0);
    
    FragColor = vec4(color, 1.0f);
}

