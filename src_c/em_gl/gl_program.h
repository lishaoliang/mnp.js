///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    gl_program.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __GL_PROGRAM_H__
#define __GL_PROGRAM_H__


#include "SDL/SDL_opengl.h"


typedef struct emgl_program_t_
{
    GLuint      program;    // GLSL编号
    GLuint      v_shader;
    GLuint      f_shader;

    int         use;        // 0.未使用; 非0.使用
}emgl_program_t;


emgl_program_t* emgl_program_create(const char* p_vertex, const char* p_fragment);
void emgl_program_destroy(emgl_program_t* p_prog);

void emgl_program_use(emgl_program_t* p_prog);
void emgl_program_validate(emgl_program_t* p_prog);


#endif // __GL_PROGRAM_H__
//end

