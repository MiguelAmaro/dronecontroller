/* date = January 7th 2021 11:41 pm */

#ifndef FLIGHTCONTROL_RENDERER_H
#define FLIGHTCONTROL_RENDERER_H
#include "dc_platform.h"
#include "dc_opengl.h"

//
//void Renderer_draw_sprite(readonly platform *Platform, readonly Render_info *render_info, readonly f32 *color )
//{
// NOTE(MIGUEL): Include way to specify gl texture?
//
// NOTE(MIGUEL): Maybe take an array of uniforms 
//GL_Call(glUniformMatrix4fv(render_info->uniform_model, 1, GL_FALSE, (f32 *)render_info->matrix_model));
//GL_Call(glUniform3f       (render_info->uniform_color, color[0],color[1], color[2]));
//GL_Call(glUniform2f      (render_info->uniform_input, Platform->mouse_x / Platform->window_width, Platform->mouse_y / Platform->window_height));
//
//GL_Call(glActiveTexture(GL_TEXTURE1));
//GL_Call(glBindTexture(GL_TEXTURE_2D, render_info->texture));
//
// Enables the alpha channel
//glEnable(GL_BLEND);
//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
//
//GL_Call(glBindVertexArray(render_info->vertex_attributes_id));
//GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
//GL_Call(glBindVertexArray(0));
//
//
//return;
//}
//
//
#endif //FLIGHTCONTROL_RENDERER_H
