#include "gl_yuv.h"
#include "mem/klb_mem.h"
#include "gl_program.h"
#include "em_util/em_log.h"


// WebGL中必须设置浮点精度
static const char YUV_VertexShader[] = "precision mediump float;"
"attribute vec4 vertex_screen;"
"attribute vec2 vertex_texture;"
"varying vec2 texture_pos;"
"void main(void)"
"{"
"gl_Position = vertex_screen;"
"texture_pos = vertex_texture;"
"}";

static const char YUV_FragmentShader[] = "precision mediump float;"
"varying vec2 texture_pos;"
"uniform sampler2D texture_y;"
"uniform sampler2D texture_u;"
"uniform sampler2D texture_v;"
"void main(void)"
"{"
"vec3 yuv;"
"vec3 rgb;"
"yuv.x = texture2D(texture_y, texture_pos).x;"
"yuv.y = texture2D(texture_u, texture_pos).x-0.5;"
"yuv.z = texture2D(texture_v, texture_pos).y-0.5;"
"rgb = mat3(1,1,1,"
"0,-0.39465,2.03211,"
"1.13983,-0.58060,0) * yuv;"
"gl_FragColor = vec4(rgb, 1);"
"}";


// YUV纹理顶点坐标
static const GLfloat YUV_TextureVertices[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };


typedef struct emgl_yuv_t_
{
    emgl_program_t* p_prog;             ///< GLSL程序

    GLuint          texture_y;          ///< 在片段shader共享数据标记"uniform sampler2D tex_y"
    GLuint          texture_u;          ///< 在片段shader共享数据标记"uniform sampler2D tex_u"
    GLuint          texture_v;          ///< 在片段shader共享数据标记"uniform sampler2D tex_v"

    GLuint          vertex_screen;      ///< 
    GLuint          vertex_tex;         ///< 

    GLuint          tex_id_y;           ///< 创建Y对应的纹理
    GLuint          tex_id_u;           ///< 创建U对应的纹理
    GLuint          tex_id_v;           ///< 创建V对应的纹理

    GLuint          vbo;                ///< 创建VBO顶点缓存

}emgl_yuv_t;


// 按 X * Y分割 计算屏幕顶点坐标值
static void init_split_vertices(int split_x, int split_y, GLfloat* p_vertices)
{
    // opengl 窗口坐标, 坐标原点在屏幕中心(在初始化时指定的)
    //   (-1, 1)            (1, 1)
    //
    //            (0, 0)
    //
    //   (-1,-1)            (1, -1)
    GLfloat w = (GLfloat)2.0 / (GLfloat)split_x, h = (GLfloat)2.0 / (GLfloat)split_y;

    // OpenGL识别的是三角形, 矩形被分解为2个三角形
    GLfloat* ptr = p_vertices;
    for (int j = 0; j < split_y; j++)
    {
        for (int i = 0; i < split_x; i++)
        {
            GLfloat x = -1.0 + w * i;
            GLfloat y = -1.0 + h * j;

            // 左下点
            *ptr = x;     ptr++;
            *ptr = y;     ptr++;

            // 右下点
            *ptr = x + w; ptr++;
            *ptr = y;     ptr++;

            // 左上点
            *ptr = x;     ptr++;
            *ptr = y + h; ptr++;

            // 右上点
            *ptr = x + w; ptr++;
            *ptr = y + h; ptr++;
        }
    }

    GLfloat tmp[8 * 8] = { 0 };

    // 重新排序, 上面得出的是从(-1,-1)起始标记的坐标次序
    int w_num = split_x * 8;
    GLfloat* ptr_start = p_vertices;
    GLfloat* ptr_end = p_vertices + (split_y - 1) * w_num;
    for (int k = 0; k < split_y / 2; k++)
    {
        memcpy(tmp, ptr_end, w_num * sizeof(GLfloat));

        memcpy(ptr_end, ptr_start, w_num * sizeof(GLfloat));
        memcpy(ptr_start, tmp, w_num * sizeof(GLfloat));

        ptr_start += w_num;
        ptr_end -= w_num;
    }
}

emgl_yuv_t* emgl_yuv_create()
{
    emgl_yuv_t* p_yuv = KLB_MALLOC(emgl_yuv_t, 1, 0);
    KLB_MEMSET(p_yuv, 0, sizeof(emgl_yuv_t));

    p_yuv->p_prog = emgl_program_create(YUV_VertexShader, YUV_FragmentShader);
    emgl_program_use(p_yuv->p_prog);

    // 获取GLSL程序中共享的变量id, 纹理Y,U,V
    p_yuv->texture_y = glGetUniformLocation(p_yuv->p_prog->program, "texture_y");
    p_yuv->texture_u = glGetUniformLocation(p_yuv->p_prog->program, "texture_u");
    p_yuv->texture_v = glGetUniformLocation(p_yuv->p_prog->program, "texture_v");

    // 获取GLSL程序中的顶点变量
    p_yuv->vertex_screen = glGetAttribLocation(p_yuv->p_prog->program, "vertex_screen");
    p_yuv->vertex_tex = glGetAttribLocation(p_yuv->p_prog->program, "vertex_texture");


    // 创建2维纹理Y, 并设置相应属性
    glGenTextures(1, &p_yuv->tex_id_y);
    glBindTexture(GL_TEXTURE_2D, p_yuv->tex_id_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 创建2维纹理U, 并设置相应属性
    glGenTextures(1, &p_yuv->tex_id_u);
    glBindTexture(GL_TEXTURE_2D, p_yuv->tex_id_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 创建2维纹理V, 并设置相应属性
    glGenTextures(1, &p_yuv->tex_id_v);
    glBindTexture(GL_TEXTURE_2D, p_yuv->tex_id_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // 顶点坐标
    int f8_size = sizeof(GLfloat) * 8;

    GLfloat vertices[8] = { 0 };
    init_split_vertices(1, 1, vertices);

    glGenBuffers(1, &p_yuv->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, p_yuv->vbo);
    glBufferData(GL_ARRAY_BUFFER, f8_size * 2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, f8_size, (GLfloat*)YUV_TextureVertices);
    glBufferSubData(GL_ARRAY_BUFFER, f8_size, f8_size, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    emgl_program_validate(p_yuv->p_prog);

    //p_yuv->b_update = L_FALSE;

    return p_yuv;
}

void emgl_yuv_destroy(emgl_yuv_t* p_yuv)
{
    assert(NULL != p_yuv);

    if (0 != p_yuv->vbo)
    {
        glDeleteBuffers(1, &p_yuv->vbo);
        p_yuv->vbo = 0;
    }

    if (0 != p_yuv->tex_id_v)
    {
        glDeleteTextures(1, &p_yuv->tex_id_v);
        p_yuv->tex_id_v = 0;
    }

    if (0 != p_yuv->tex_id_u)
    {
        glDeleteTextures(1, &p_yuv->tex_id_u);
        p_yuv->tex_id_u = 0;
    }

    if (0 != p_yuv->tex_id_y)
    {
        glDeleteTextures(1, &p_yuv->tex_id_y);
        p_yuv->tex_id_y = 0;
    }

    KLB_FREE_BY(p_yuv->p_prog, emgl_program_destroy);
    KLB_FREE(p_yuv);
}


int emgl_yuv_draw(emgl_yuv_t* p_yuv, em_yuv_frame_t* p_frame)
{
    assert(NULL != p_yuv);

    int w = p_frame->w;
    int h = p_frame->h;

    //char* ptr = KLB_MALLOC(char, w * h * 2, 0);
    //KLB_MEMSET(ptr, 0x0, w * h * 2);

    emgl_program_use(p_yuv->p_prog);


    // 使用VBO来设置顶点,纹理映射坐标
    // Note. WebGL不支持直接使用CPU内存填充顶点数据; 需要使用GPU缓存来填充顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, p_yuv->vbo);

    glVertexAttribPointer(p_yuv->vertex_screen, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(sizeof(GLfloat) * 8));
    glEnableVertexAttribArray(p_yuv->vertex_screen);

    glVertexAttribPointer(p_yuv->vertex_tex, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(p_yuv->vertex_tex);


    // 更新数据
    int uv_w = w / 2, uv_h = h / 2;
    //GLvoid *p_y = ptr, *p_u = ptr + w * h, *p_v = ptr + w * h + w * h / 4;
    GLvoid *p_y = p_frame->p_y, *p_u = p_frame->p_u, *p_v = p_frame->p_v;

    //Y
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, p_yuv->tex_id_y);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, p_frame->pitch_y, p_frame->h_y, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p_y);
    glUniform1i(p_yuv->texture_y, 0);

    //U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, p_yuv->tex_id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, p_frame->pitch_u, p_frame->h_u, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p_u);
    glUniform1i(p_yuv->texture_u, 1);

    //V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, p_yuv->tex_id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, p_frame->pitch_v, p_frame->h_v, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p_v);
    glUniform1i(p_yuv->texture_v, 2);

    // 绘制
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    // 中间状态必须解除, 否则浏览器报警告
    glDisableVertexAttribArray(p_yuv->vertex_tex);
    glDisableVertexAttribArray(p_yuv->vertex_screen);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    emgl_program_validate(p_yuv->p_prog);

    //p_yuv->b_update = L_FALSE;

    //KLB_FREE(ptr);

    return 0;
}
