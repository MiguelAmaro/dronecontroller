#include "FlightControl_Platform.h"
#include "FlightControl_OpenGL.h"
#include "FlightControl_Shader.h"
#include "FlightControl_Helpers.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stdio.h>
#include "LAL.h"

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

global Entity Sprite = {0};

global u32 ufrm_sprite_model     ;
global u32 ufrm_sprite_color     ; 
global u32 ufrm_sprite_projection;

global BITMAPINFO BitmapInfo;
global void *BitmapMemory;
global int BitmapWidth;
global int BitmapHeight;
global int BytesPerPixel = 4;


void
App_Init(Platform *platform_)
{
    //~GRAPH-> SPRITE.INIT
    
    
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
        if(platform->key_down[KEY_c])
        {
            app_should_quit = 1;
        }
    }
    
    //glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    //~SPRITE RENDERER_01
    
    vec3 position = { 0.0f, 0.0f, 0.0f };
    static vec3 size = { 1.0f, 1.0f, 0.0f };
    static vec3 color = { 1.0f, 1.0f, 1.0f };
    static vec2 size2 = { 1.0f, 1.0f };
    static f32 rotate = 10;
    static vec3 scalefactor = { 0 };
    
    mat4 model = GLM_MAT4_IDENTITY_INIT; // constructor
    
    scalefactor[0] = size[0];
    scalefactor[1] = size[1];
    scalefactor[2] = 1.0f;
    
    
    GL_Call(glUseProgram(Sprite.shader));
    
    
    position[0] += ( 20) * platform->key_down[KEY_d];
    position[0] += (-20) * platform->key_down[KEY_a];
    position[1] += ( 20) * platform->key_down[KEY_w];
    position[1] += (-20) * platform->key_down[KEY_s];
    
    glm_translate(model, position );
    
    //glm_mat4_print(model, stdout);
    
    size[0] += 0.5f;
    size[1] += 1.0f;
    
    glm_translate(model, size); 
    //glm_rotate(model, glm_rad(rotate),(vec3){0.0f, 0.0f, 1.0f}); 
    rotate++;
    
    //glm_translate(model, size);
    //Helpers_Display_Matrix4(model, "Model Matrix");
    
    glm_scale(model, scalefactor); 
    
    GL_Call(glUniform3f(ufrm_sprite_color, color[0],color[1], color[2]));
    GL_Call(glUniformMatrix4fv(ufrm_sprite_model, 1, GL_FALSE, (f32 *)model));
    
    
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, Sprite.texture));
    
    // Enables the alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    GL_Call(glBindVertexArray(Sprite.vertex_Attributes));
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_Call(glBindVertexArray(0));
    
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

