#include "dc_opengl.h"
#include "dc_memory.h"

GLenum OpenGLCheckError_(readonly u8 *file, u32 line)
{
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR)
  {
    u8 error[100];
    switch (errorCode)
    {
      case GL_INVALID_ENUM:                  strcpy(error, "INVALID_ENUM\0" ); break;
      case GL_INVALID_VALUE:                 strcpy(error, "INVALID_VALUE\0"); break; 
      case GL_INVALID_OPERATION:             strcpy(error, "INVALID_OPERATION\0"); break;
      case GL_STACK_OVERFLOW:                strcpy(error, "STACK_OVERFLOW\0"); break;
      case GL_STACK_UNDERFLOW:               strcpy(error, "STACK_UNDERFLOW\0"); break;
      case GL_OUT_OF_MEMORY:                 strcpy(error, "OUT_OF_MEMORY\0"); break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION\0"); break;
    }
    printf("%s | %s ( %d ) \r\n", error, file, line);
  }
  return errorCode;
}

b32 OpenGLLog(readonly u8 *file, readonly u32 line, readonly u8* function)
{
  GLenum errorCode;
  while (errorCode = glGetError())
  {
    u8 error[100];
    switch (errorCode)
    {
      case GL_INVALID_ENUM:                  strcpy(error, "INVALID_ENUM" ); break;
      case GL_INVALID_VALUE:                 strcpy(error, "INVALID_VALUE"); break; 
      case GL_INVALID_OPERATION:             strcpy(error, "INVALID_OPERATION"); break;
      case GL_STACK_OVERFLOW:                strcpy(error, "STACK_OVERFLOW"); break;
      case GL_STACK_UNDERFLOW:               strcpy(error, "STACK_UNDERFLOW"); break;
      case GL_OUT_OF_MEMORY:                 strcpy(error, "OUT_OF_MEMORY"); break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: strcpy(error, "INVALID_FRAMEBUFFER_OPERATION"); break;
    }
    printf("%s | %s ( %d ) | %s \r\n", error, file, line, function);
    
    return false;
  }
  
  return true;
}

void OpenGLClearError(void)
{
  while(glGetError() != GL_NO_ERROR);
  
  return;
}

void OpenGLCreateVertexBuffer(u32 *VertexBufferID, f32 *Vertices, u32 Size, GLenum DrawType)
{
  OPENGL_DBG(glGenBuffers(1, VertexBufferID));
  OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, *VertexBufferID));
  OPENGL_DBG(glBufferData(GL_ARRAY_BUFFER, Size, Vertices, DrawType));
  
  return;
}

void OpenGLVertexBuffer_Destroy(u32 vertex_buffer_id)
{
  OPENGL_DBG(glDeleteBuffers(1, &vertex_buffer_id));
  
  return;
}

void OpenGLVertexBuffer_Bind(u32 vertex_buffer_id)
{
  OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id));
  
  return;
}

void OpenGLVertexBuffer_Unbind(void)
{
  OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, 0));
  
  return;
}

// NOTE(MIGUEL): Mimic VertexBuffer functions for index buffers

void OpenGLIndexBuffer_Create(u32 *index_buffer_id, f32 *index_buffer, u32 count)
{
  OPENGL_DBG(glGenBuffers(1, index_buffer_id));
  OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *index_buffer_id));
  OPENGL_DBG(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), index_buffer, GL_STATIC_DRAW));
  
  return;
}

void OpenGLIndexBuffer_Destroy(u32 index_buffer_id)
{
  OPENGL_DBG(glDeleteBuffers(1, &index_buffer_id));
  
  return;
}

void OpenGLIndexBuffer_Bind(u32 index_buffer_id)
{
  OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id));
  
  return;
}

void OpenGLIndexBuffer_Unbind(void)
{
  OPENGL_DBG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
  
  return;
}

#if 0
void OpenGLLoadTexture(u32 *ShaderID, opengl_render_info *Info, b32 should_flip )
{
  // THE AFFECTS OF THIS MIGHT NOT BE APPARENT UNSLESS THERE ARE CERTAIN CONDITIONS
  GL_Call(glGenTextures(1, &Info->TextureID));
  GL_Call(glBindTexture(GL_TEXTURE_2D, Info->TextureID));
  // CONFIGUE OPENGL WRAPPING OPTIONS
  GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
  GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
  // CONFIGURE OPENGL FILTERING OPTIONS
  GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  
  //GL_Call(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texture_border_color)); 
  
  // LOAD TEXTURE
  s32 sprite_tex_width, sprite_tex_height, sprite_nrChannels;
  stbi_set_flip_vertically_on_load(should_flip);  
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
  
  
  return;
}
#endif