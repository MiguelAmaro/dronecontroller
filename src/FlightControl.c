#include "FlightControl_Platform.h"
#include "FlightControl_OpenGL.h"
#include "FlightControl_Shader.h"
#include "FlightControl_Helpers.h"
#include "FlightControl_Entity.h"
#include "FlightControl_Renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stdio.h>
#include <math.h>
#include "LAL.h"

typedef struct App {
    f32 dela_time;
} App;


global App *app = 0;
global Platform *platform = 0;

global Entity Sprite = {0};
global Entity Nick = {0};


global u32 ufrm_sprite_model     ;
global u32 ufrm_sprite_color     ; 
global u32 ufrm_sprite_projection;

global vec4 color       = { 1.0f, 1.0f, 1.0f };
global mat4 translation = GLM_MAT4_ZERO_INIT;
global mat4 scale       = GLM_MAT4_ZERO_INIT;
global mat4 rotation    = GLM_MAT4_ZERO_INIT;
global vec3 scalefactor = { 100.0f, 100.0f, 0.0f };



global Render_Info nick_render_info;
//global u32 ufrm_nick_model     ;
//global u32 ufrm_nick_mouse     ;
//global u32 ufrm_nick_color     ; 
// NOTE(MIGUEL): Should there just be one projectin matrix for everthing
//global u32 ufrm_nick_projection;

global vec4 nick_color       = { 1.0f, 1.0f, 1.0f };
global mat4 nick_translation = GLM_MAT4_ZERO_INIT;
global mat4 nick_scale       = GLM_MAT4_ZERO_INIT;
global mat4 nick_rotation    = GLM_MAT4_ZERO_INIT;
global vec3 nick_scalefactor = { 100.0f, 100.0f, 0.0f };


global BITMAPINFO BitmapInfo;
global void *BitmapMemory;
global int BitmapWidth;
global int BitmapHeight;
global int BytesPerPixel = 4;


void
App_Init(Platform *platform_)
{
    //~ INIT SPRITE GEO
    
    
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
    //stbi_set_flip_vertically_on_load(true);  
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
    
    
    //~ INIT SPRITE NICK
    
    // NOTE(MIGUEL): Use GEO Sprite vertices
    
    Nick.vertices = sprite_vertices;
    
    GL_Call(glGenVertexArrays(1, &Nick.vertex_Attributes));
    GL_Call(glGenBuffers(1, &Nick.vertex_Buffer));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, Nick.vertex_Attributes));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_DYNAMIC_DRAW));
    
    GL_Call(glBindVertexArray(Nick.vertex_Attributes));
    GL_Call(glEnableVertexAttribArray(0));
    GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0x00));
    
    
    // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
    GL_Call(glGenTextures(1, &Nick.texture));
    GL_Call(glBindTexture(GL_TEXTURE_2D, Nick.texture));
    // CONFIGUE OPENGL WRAPPING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // CONFIGURE OPENGL FILTERING OPTIONS
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
    
    // LOAD TEXTURE
    //s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
    //stbi_set_flip_vertically_on_load(true);  
    /*u8 **/sprite_tex_data = stbi_load("../res/images/nick.png", &sprite_tex_width, &sprite_tex_height, &sprite_nrChannels, STBI_rgb_alpha); 
    
    if(sprite_tex_data)
    {
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
    ReadAShaderFile(&Nick.shader, "../res/shaders/Nick.glsl");
    
    
    // NOTE(MIGUEL): ISSUES HERE !!!! 
    GL_Call(glUseProgram(Nick.shader));
    GL_Call(glUniform1i(glGetUniformLocation(Nick.shader, "sprite_texture"), 1));
    
    // NOTE(MIGUEL): deleting the folowing 2 lines changes it from black to brown
    
    // FOR LATER
    mat4 nick_projection;
    GL_Call(glm_ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f, nick_projection));
    GL_Call(glUniformMatrix4fv(glGetUniformLocation(Nick.shader, "projection"), 1, GL_FALSE, (f32 *)nick_projection));
    
    // UNBIND BUFFER
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    GL_Call(glBindVertexArray(0));
    
    nick_render_info.uniform_model      = glGetUniformLocation(Nick.shader, "model");
    nick_render_info.uniform_color      = glGetUniformLocation(Nick.shader, "spriteColor");
    nick_render_info.uniform_input      = glGetUniformLocation(Nick.shader, "mousePos");
    nick_render_info.uniform_projection = glGetUniformLocation(Nick.shader, "projection");
    
    return;
}

b32 App_Update(Platform *platform_)
{
    b32 app_should_quit = 0;
    platform = platform_;
    
    app = platform->permanent_storage;
    {
        f32 move_speed = -200.0f *  app->dela_time;
        app->dela_time = platform->current_time - platform->last_time;
        
        if(platform->key_down[KEY_c])
        {
            app_should_quit = 1;
        }
        
        
        // NOTE(MIGUEL): Input only for SRITE AKE GEO(player)
        if(platform->key_down[KEY_w])
        {
            glm_translate(translation, (vec3){0.0f, move_speed , 0.0f} );
            //printf("w\n");
        }
        
        if(platform->key_down[KEY_s])
        {
            glm_translate(translation, (vec3){0.0f, -move_speed, 0.0f} );
            //printf("s\n");
        }
        
        if(platform->key_down[KEY_a])
        {
            glm_translate(translation, (vec3){move_speed, 0.0f, 0.0f} );
            //printf("a\n");
            
        }
        
        if(platform->key_down[KEY_d])
        {
            glm_translate(translation, (vec3){-move_speed, 0.0f, 0.0f} );
            //printf("d\n");
        }
        
        if(platform->key_down[KEY_q])
        {
            glm_rotate(rotation, glm_rad(10.0f),(vec3){0.0f, 1.0f, 0.0f}); 
            //printf("d\n");
        }
        
        
        if(platform->key_down[KEY_e])
        {
            glm_rotate(rotation, glm_rad(10.0f),(vec3){0.0f, 1.0f, 0.0f}); 
            //printf("d\n");
        }
        
    }
    
    // ************************************************
    // RENDERING
    //*************************************************
    local_persist b32 first_render = true;
    
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    //~SPRITE RENDERER_01
    
    mat4 model = GLM_MAT4_ZERO_INIT; // constructor
    mat4 nick_model = GLM_MAT4_ZERO_INIT; // constructor
    nick_render_info.matrix_model = (f32 *)&nick_model;
    
    GL_Call(glUseProgram(Sprite.shader));
    
    if(first_render)
    {
        glm_translate_make(translation, (vec3){200.0f, 200.0f, 0.0f});
        glm_scale_make(scale, (vec3){200.0f, 200.0f, 1.0f});
        glm_rotate_make(rotation, glm_rad(10.0f) ,(vec3){0.0f, 1.0f, 0.0f});
        
        glm_translate_make(nick_translation, (vec3){1.0f, 1.0f, 0.0f});
        glm_scale_make(nick_scale, (vec3){200.0f, 200.0f, 1.0f});
        glm_rotate_make(nick_rotation, glm_rad(10.0f) ,(vec3){0.0f, 1.0f, 0.0f});
        
        Helpers_Display_Matrix4(translation, "Translate Matrix");
        Helpers_Display_Matrix4(scale, "Scale Matrix");
        Helpers_Display_Matrix4(rotation, "Rotate Matrix");
        
        first_render = false;
    }
    //rotate++;
    
    
    glm_mat4_mulN((mat4 *[]){&translation, &scale }, 2, model);
    
    //glm_mat4_print(model, stdout);
    
    //Helpers_Display_Matrix4(model, "Model Matrix");
    
    GL_Call(glUniformMatrix4fv(ufrm_sprite_model, 1, GL_FALSE, (f32 *)model));
    GL_Call(glUniform3f(ufrm_sprite_color, color[0],color[1], color[2]));
    
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, Sprite.texture));
    
    // Enables the alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    
    GL_Call(glBindVertexArray(Sprite.vertex_Attributes));
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_Call(glBindVertexArray(0));
    
    //~ SPRITE RENDERER_01
    
    GL_Call(glUseProgram(Nick.shader));
    glm_translate(nick_translation, (vec3){ ( cos(platform->current_time) ), (  sin(platform->current_time) ), 0.0f} );
    glm_mat4_mulN((mat4 *[]){&nick_translation, &nick_scale }, 2, nick_model);
    
    Renderer_Draw_Sprite(platform, &Nick, &nick_render_info, (f32 *)&nick_color );
    
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

