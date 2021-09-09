
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
uniform float DeltaTime;
uniform vec2 UISize;
uniform vec2 UIPos;
uniform vec2 WindowSize;


// Helper functions:
float slopeFromT (float t, float A, float B, float C){
    float dtdx = 1.0/(3.0*A*t*t + 2.0*B*t + C); 
    return dtdx;
}

float xFromT (float t, float A, float B, float C, float D){
    float x = A*(t*t*t) + B*(t*t) + C*t + D;
    return x;
}

float yFromT (float t, float E, float F, float G, float H){
    float y = E*(t*t*t) + F*(t*t) + G*t + H;
    return y;
}
float lineSegment(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return smoothstep(0.0, 1.0 / WindowSize.x, length(pa - ba*h));
}

float cubicBezier(float x, vec2 a, vec2 b){
    
    float y0a = 0.0; // initial y
    float x0a = 0.0; // initial x 
    float y1a = a.y;    // 1st influence y   
    float x1a = a.x;    // 1st influence x 
    float y2a = b.y;    // 2nd influence y
    float x2a = b.x;    // 2nd influence x
    float y3a = 1.0; // final y 
    float x3a = 1.0; // final x 
    
    float A =   x3a - 3.0*x2a + 3.0*x1a - x0a;
    float B = 3.0*x2a - 6.0*x1a + 3.0*x0a;
    float C = 3.0*x1a - 3.0*x0a;   
    float D =   x0a;
    
    float E =   y3a - 3.0*y2a + 3.0*y1a - y0a;    
    float F = 3.0*y2a - 6.0*y1a + 3.0*y0a;             
    float G = 3.0*y1a - 3.0*y0a;             
    float H =   y0a;
    
    // Solve for t given x (using Newton-Raphelson), then solve for y given t.
    // Assume for the first guess that t = x.
    float currentt = x;
    for (int i=0; i < 5; i++){
        float currentx = xFromT (currentt, A,B,C,D); 
        float currentslope = slopeFromT (currentt, A,B,C);
        currentt -= (currentx - x)*(currentslope);
        currentt = clamp(currentt,0.0,1.0); 
    } 
    
    float y = yFromT (currentt,  E,F,G,H);
    return y;
}

void main() {
    vec2 st = gl_FragCoord.xy/WindowSize.xy;
	float px = 1.0 / WindowSize.y;
    
    // control point
    vec2 cp0 = vec2(0.25, sin(DeltaTime) * 0.25 + 0.5);
    vec2 cp1 = vec2(0.75, cos(DeltaTime) * 0.25 + 0.5);
    float l = cubicBezier(st.x, cp0, cp1);
    vec3 color = vec3(smoothstep(l, l+px, st.y));
    
    // draw control points
    color = mix(vec3(0.5), color, lineSegment(st, vec2(0.0), cp0));
    color = mix(vec3(0.5), color, lineSegment(st, vec2(1.0), cp1));
    color = mix(vec3(0.5), color, lineSegment(st, cp0, cp1));
    color = mix(vec3(1.0,0.0,0.0), color, smoothstep(0.01,0.01+px,distance(cp0, st)));
    color = mix(vec3(1.0,0.0,0.0), color, smoothstep(0.01,0.01+px,distance(cp1, st)));
    
    gl_FragColor = vec4(color, 1.0);
}