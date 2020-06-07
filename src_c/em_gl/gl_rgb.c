#include "gl_rgb.h"
#include "mem/klb_mem.h"
#include "gl_program.h"
#include "em_util/em_log.h"

typedef struct emgl_rgb_t_
{
    emgl_program_t* p_prog;

    GLuint          texture_argb;       ///< 在片段shader共享数据标记"uniform sampler2D tex_rgba"

    GLuint          tex_id_argb;        ///< tex_rgba对应的纹理
    GLuint          vbo;                ///< 创建VBO顶点缓存

    int             w;
    int             h;
    char*           p_pixel;
}emgl_rgb_t;


// precision mediump float;
static const char RGB_VertexShader[] = "precision mediump float;"
"attribute vec4 vertex_screen;"
"attribute vec2 vertex_texture;"
"varying vec2 texture_pos;"
"void main(void)"
"{"
"gl_Position = vertex_screen;"
"texture_pos = vertex_texture;"
"}";

static const char RGB_FragmentShader[] = "precision mediump float;"
"varying vec2 texture_pos;"
"uniform sampler2D texture_argb;"
"void main(void)"
"{"
"gl_FragColor = texture2D(texture_argb, texture_pos).bgra;"
"}";

// 纹理顶点坐标
static const GLfloat RGB_TextureVertices[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };

// 屏幕顶点坐标
static const GLfloat RGB_ScreenVertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };


emgl_rgb_t* emgl_rgb_create(int w, int h)
{
    emgl_rgb_t* p_rgb = KLB_MALLOC(emgl_rgb_t, 1, 0);
    KLB_MEMSET(p_rgb, 0, sizeof(emgl_rgb_t));

    p_rgb->p_prog = emgl_program_create(RGB_VertexShader, RGB_FragmentShader);

    emgl_program_use(p_rgb->p_prog);

    // 获取GLSL程序中共享的变量id
    p_rgb->texture_argb = glGetUniformLocation(p_rgb->p_prog->program, "texture_argb");


    // 创建2维纹理Y, 并设置相应属性
    glGenTextures(1, &p_rgb->tex_id_argb);
    glBindTexture(GL_TEXTURE_2D, p_rgb->tex_id_argb);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // 顶点坐标
    int f8_size = sizeof(GLfloat) * 8;

    glGenBuffers(1, &p_rgb->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, p_rgb->vbo);
    glBufferData(GL_ARRAY_BUFFER, f8_size * 2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, f8_size, (GLfloat*)RGB_TextureVertices);
    glBufferSubData(GL_ARRAY_BUFFER, f8_size, f8_size, RGB_ScreenVertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    emgl_program_validate(p_rgb->p_prog);


    p_rgb->p_pixel = KLB_MALLOC(char, w * h * 4, 0);
    KLB_MEMSET(p_rgb->p_pixel, 0, w * h * 4);

    p_rgb->w = w;
    p_rgb->h = h;

    return p_rgb;
}

void emgl_rgb_estroy(emgl_rgb_t* p_rgb)
{
    assert(NULL != p_rgb);

    KLB_FREE(p_rgb->p_pixel);

    if (0 != p_rgb->vbo)
    {
        glDeleteBuffers(1, &p_rgb->vbo);
        p_rgb->vbo = 0;
    }

    if (0 != p_rgb->tex_id_argb)
    {
        glDeleteTextures(1, &p_rgb->tex_id_argb);
        p_rgb->tex_id_argb = 0;
    }

    KLB_FREE_BY(p_rgb->p_prog, emgl_program_destroy);
    KLB_FREE(p_rgb)
}

static void fill_rect(char* ptr, int width, int bpp, int left, int top, int right, int bottom, uint32_t color)
{
    int src_pitch = bpp * width;
    char *p_data = ptr + (src_pitch * top) + (bpp * left);
    char *p_point = p_data;
    int cx = (right - left), cy = (bottom - top), pitch = cx * bpp;

    //画出一行后复制
    for (int w = 0; w < cx; w++)
    {
        memcpy(p_point, &color, bpp);
        p_point += bpp;
    }

    p_point = p_data + src_pitch;

    for (int h = 1; h < cy; ++h)
    {
        memcpy(p_point, p_data, pitch);
        p_point += src_pitch;
    }
}

#define ARGB(a, r, g, b) \
            ((uint32_t) \
            (((a) & 0xFF) << 24) | \
            (((r) & 0xFF) << 16) | \
            (((g) & 0xFF) << 8) | \
            ((b) & 0xFF))

int emgl_rgb_draw(emgl_rgb_t* p_rgb)
{
    assert(NULL != p_rgb);

    unsigned char* p_bmp = (unsigned char*)p_rgb->p_pixel;
    memset(p_bmp, 0x0, p_rgb->w * p_rgb->h * 4);
    fill_rect((char*)p_bmp, p_rgb->w, 4, 0, 0, p_rgb->w / 2, p_rgb->h / 2, ARGB(0xFF, 0xFF, 0x0, 0x0));


    emgl_program_use(p_rgb->p_prog);

    // 使用VBO来设置顶点,纹理映射坐标
    // Note. WebGL不支持直接使用CPU内存填充顶点数据; 需要使用GPU缓存来填充顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, p_rgb->vbo);

    // 设置屏幕顶点映射坐标 
    GLuint vertex_screen = glGetAttribLocation(p_rgb->p_prog->program, "vertex_screen");
    glVertexAttribPointer(vertex_screen, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(sizeof(GLfloat) * 8));
    glEnableVertexAttribArray(vertex_screen);

    // 设置纹理映射坐标
    GLuint vertex_texture = glGetAttribLocation(p_rgb->p_prog->program, "vertex_texture");
    glVertexAttribPointer(vertex_texture, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(vertex_texture);

    //rgba
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, p_rgb->tex_id_argb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p_rgb->w, p_rgb->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, p_rgb->p_pixel);
    glUniform1i(p_rgb->texture_argb, 0);

    // 绘制
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // 中间状态必须解除, 否则浏览器报警告
    glDisableVertexAttribArray(vertex_texture);
    glDisableVertexAttribArray(vertex_screen);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    emgl_program_validate(p_rgb->p_prog);

    return 0;
}
