#include "FlightControl_Platform.h"
#include "FLIGHTCONTROL_OpenGL.h"
#include "FLIGHTCONTROL_Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "LAL.h"


//global u8 Running;
global BITMAPINFO BitmapInfo;
global void *BitmapMemory;
global int BitmapWidth;
global int BitmapHeight;
global int BytesPerPixel = 4;


typedef struct 
{
    f32 *vertices;
    u32 *indices;
    u32  shader;
    u32  vertex_Buffer;
    u32  element_Buffer;
    u32  vertex_Attributes;
    u32  texture;
} Entity;

typedef struct App {
    u32 stuff;
} App;

global App *app = 0;
global Platform *platform = 0;


global u32 ufrm_sprite_model     ;
global u32 ufrm_sprite_color     ; 
global u32 ufrm_sprite_projection;



void
App_Init(Platform *platform_)
{
    //~GRAPH-> SPRITE.INIT
    
    local_persist Entity Sprite = {0};
    
    f32 sprite_vertices[] = { 
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 
        
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    
    Sprite.vertices = sprite_vertices;
    
    GL_Call(glGenVertexArrays(1, &Sprite.vertex_Attributes));
    GL_Call(glGenBuffers(1, &Sprite.vertex_Buffer));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, Sprite.vertex_Attributes));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_DYNAMIC_DRAW));
    
    GL_Call(glBindVertexArray(Sprite.vertex_Attributes));
    GL_Call(glEnableVertexAttribArray(0));
    GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));
    
    
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    GL_Call(glGenTextures(1, &Sprite.texture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, Sprite.texture));
    // CONFIGUE OPENGL WRAPPING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // CONFIGURE OPENGL FILTERING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
    
    // LOAD TEXTURE
    s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
    stbi_set_flip_vertically_on_load(true);  
    u8 *sprite_tex_data = stbi_load("../res/images/geo.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 
    if(sprite_tex_data)
    {
        //printf("Tex Data: \n %d | %d | %s  \n", sprite_tex_width, sprite_tex_height, sprite_tex_data);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // NOTE(MIGUEL): NO AFFECT
        GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite_tex_width, sprite_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sprite_tex_data));
        GL_Call(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        printf("Failed to load texture");
    }
    
    stbi_image_free(sprite_tex_data);
    
    
    // THIS SHADER MAyBE FUCKED UP
    ReadAShaderFile(&Sprite.shader, "../res/shaders/SpriteShader.glsl");
    
    
    // NOTE(MIGUEL): ISSUES HERE !!!! 
    GL_Call(glUseProgram(Sprite.shader));
    GL_Call(glUniform1i(glGetUniformLocation(Sprite.shader, "sprite_texture"), 1));
    
    // NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown
    
    // FOR LATER
    mat4 projection;
    GL_Call(glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f, projection));
    GL_Call(glUniformMatrix4fv(glGetUniformLocation(Sprite.shader, "projection"), 1, GL_FALSE, (f32 *)projection));
    
    // UNBIND BUFFER
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    GL_Call(glBindVertexArray(0));
    
    ufrm_sprite_model      = glGetUniformLocation(Sprite.shader, "model");
    ufrm_sprite_color      = glGetUniformLocation(Sprite.shader, "spriteColor");
    ufrm_sprite_projection = glGetUniformLocation(Sprite.shader, "projection");
    
    return;
}

b32 App_Update(Platform *platform_)
{
    b32 app_should_quit = 0;
    platform = platform_;
    app = platform->permanent_storage;
    {
        if(platform->key_down[KEY_a])
        {
            app_should_quit = 1;
        }
    }
    
    return app_should_quit;
}


// NOTE(MIGUEL): Platform Independent
internal void RenderWeirdGradient(int XOffset, int YOffset) {
    int Width  = BitmapWidth;
    int Height = BitmapHeight;
    
    int Pitch = Width * BytesPerPixel;
    u8 *Row = (u8*)BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; Y++) {
        u8 *Pixel = (u8 *)Row;
        for(int X = 0; X < BitmapWidth; X++) {
            // Blue
            *Pixel = (u8)(X + XOffset);
            ++Pixel;
            // Green
            *Pixel = (u8)(Y + YOffset);
            ++Pixel;
            // Red
            *Pixel = 0;
            ++Pixel;
            // Padding
            *Pixel = 0;
            ++Pixel;
        }
        Row += Pitch;
    }
    
    return;
}

