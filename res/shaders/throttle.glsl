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

float plot(vec2 st, float throttle_value) 
{   
	float high = 0.0001f + throttle_value;
	float low  = 0.0f + throttle_value;
    return smoothstep(high, low, st.y) - smoothstep(high + 0.4f, low + 0.4f, st.y);
}


void main()
{
    vec2 st = gl_FragCoord.xy / size;
    
    float y = st.x;
    
    vec3 color = vec3(y);
    
    // Plot a line
    float pct = plot(st, throttle_value);
    color = (1.0-pct)*color+pct*vec3(0.0,throttle_value,0.0);
    
    FragColor = vec4(color, 1.0f);
}

