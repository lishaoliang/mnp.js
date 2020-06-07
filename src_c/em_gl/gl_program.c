#include "gl_program.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"

typedef struct emgl_shader_t_
{
    const char* p_source;
    GLenum      type;
    GLuint      shader;
}emgl_shader_t;


emgl_program_t* emgl_program_create(const char* p_vertex, const char* p_fragment)
{
    assert(NULL != p_vertex);
    assert(NULL != p_fragment);

    emgl_shader_t shaders[2] = {
        { p_vertex, GL_VERTEX_SHADER, 0 },
        { p_fragment, GL_FRAGMENT_SHADER, 0 }
    };

    GLuint program = glCreateProgram();

    GLint compiled = 0, log_size = 0;
    GLint linked = 0;

    char* p_log_msg = NULL;
    emgl_program_t* p_prog = NULL;

    for (int i = 0; i < 2; i++)
    {
        emgl_shader_t* ptr = &shaders[i];

        ptr->shader = glCreateShader(ptr->type);
        glShaderSource(ptr->shader, 1, (const GLchar**)&ptr->p_source, NULL);
        glCompileShader(ptr->shader);

        glGetShaderiv(ptr->shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            glGetShaderiv(ptr->shader, GL_INFO_LOG_LENGTH, &log_size);

            p_log_msg = KLB_MALLOC(char, log_size, 0);
            glGetShaderInfoLog(ptr->shader, log_size, NULL, p_log_msg);

            LOG_E("[%s]compiled.msg:[%s]\n", __FUNCTION__, p_log_msg);

            assert(false);
            goto err_init;
        }

        glAttachShader(program, ptr->shader);
    }

    // link  and error check
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);

        p_log_msg = KLB_MALLOC(char, log_size, 0);
        glGetShaderInfoLog(program, log_size, NULL, p_log_msg);

        LOG_E("[%s]linked.msg:[%s]\n", __FUNCTION__, p_log_msg);

        assert(false);
        goto err_init;
    }

    p_prog = KLB_MALLOC(emgl_program_t, 1, 0);
    KLB_MEMSET(p_prog, 0, sizeof(emgl_program_t));

    p_prog->program = program;
    p_prog->v_shader = shaders[0].shader;
    p_prog->f_shader = shaders[1].shader;

    assert(NULL == p_log_msg);
    return p_prog;

err_init:
    if (0 != shaders[0].shader)
    {
        glDeleteShader(shaders[0].shader);
    }

    if (0 != shaders[1].shader)
    {
        glDeleteShader(shaders[1].shader);
    }

    if (0 != program)
    {
        glDeleteProgram(program);
    }

    KLB_FREE(p_log_msg);

    return NULL;
}

void emgl_program_destroy(emgl_program_t* p_prog)
{
    assert(NULL != p_prog);

    emgl_program_validate(p_prog);

    if (0 != p_prog->v_shader)
    {
        glDeleteShader(p_prog->v_shader);
    }

    if (0 != p_prog->f_shader)
    {
        glDeleteShader(p_prog->f_shader);
    }

    if (0 != p_prog->program)
    {
        glDeleteProgram(p_prog->program);
    }

    KLB_FREE(p_prog);
}

void emgl_program_use(emgl_program_t* p_prog)
{
    assert(NULL != p_prog);

    glUseProgram(p_prog->program);
    p_prog->use = 1;
}

void emgl_program_validate(emgl_program_t* p_prog)
{
    assert(NULL != p_prog);

    if (0 != p_prog->use)
    {
        glValidateProgram(p_prog->program);
        p_prog->use = 0;
    }
}
