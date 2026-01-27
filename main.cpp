#include <cstdio>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

bool game_running = false;
bool fire_pressed = false;
int move_dir = 0;

struct Buffer
{
    size_t width, height;
    uint32_t* data;
};

struct Sprite
{
    size_t width, height;
    uint8_t* data;
};

struct SpriteAnimation
{
    bool loop;
    size_t num_frames;
    float frame_duration;
    float time;
    Sprite** frames;
};

struct Alien
{
    float x, y;
    uint8_t type;
    int hp;
};

struct Player
{
    float x, y;
    size_t life;
};

struct Projectile
{
    size_t x, y;
    int dir;
};

enum AlienType: uint8_t
{
    ALIEN_DEAD = 0,
    ALIEN_1    = 1
};

#define GAME_MAX_PROJECTILES 128

struct Game
{
    size_t width, height;
    size_t num_aliens;
    size_t num_projectiles;
    Alien* aliens;
    Player player;
    Projectile projectiles[GAME_MAX_PROJECTILES];
};

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS) game_running = false;
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS) move_dir += 1;
            else if (action == GLFW_RELEASE) move_dir -= 1;
            break;
        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS) move_dir -= 1;
            else if (action == GLFW_RELEASE) move_dir += 1;
            break;
        case GLFW_KEY_SPACE:
            if (action == GLFW_RELEASE) fire_pressed = true;
            break;
        default:
            break;
    }
}

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 24) | (g << 16) | (b << 8) | 255;
}

void buffer_clear(Buffer* buffer, uint32_t color)
{
    for (size_t i = 0; i < buffer->width * buffer-> height; ++i)
    {
        buffer->data[i] = color;
    }
}

void validate_shader(GLuint shader, const char* file = 0)
{
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;
    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);

    if (length > 0)
    {
        printf("Shader %d(%s) compile error: %s\n",
            shader, (file ? file : ""), buffer);
    }
}

bool validate_program(GLuint program)
{
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

    if(length > 0)
    {
        printf("Program %d link error: %s\n", program, buffer);
        return false;
    }

    return true; 
}

void draw_sprite_buffer(
    Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t color
){
    for(size_t xi = 0; xi < sprite.width; ++xi)
    {
        for(size_t yi = 0; yi < sprite.height; ++yi)
        {
            size_t sy = sprite.height - 1 + y - yi;
            size_t sx = x + xi;
            if(sprite.data[yi * sprite.width + xi] &&
               sy < buffer->height && sx < buffer->width) 
            {
                buffer->data[sy * buffer->width + sx] = color;
            }
        }
    }
}

bool sprite_overlap_check(
    const Sprite& sp_a, size_t x_a, size_t y_a,
    const Sprite& sp_b, size_t x_b, size_t y_b
)
{
    if(x_a < x_b + sp_b.width && x_a + sp_a.width > x_b &&
       y_a < y_b + sp_b.height && y_a + sp_a.height > y_b)
    {
        return true;
    }

    return false;
}

int main()
{
    const size_t buffer_width = 224;
    const size_t buffer_height = 256;
    /*
    ################################################
    ##           PACKAGE INITIALIZATION           ##
    ################################################
    */

    // GLFW Initialization
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // GLEW Initialization
    GLenum err = glewInit();

    if(err != GLEW_OK)
    {
        fprintf(stderr, "Error initializing GLEW.\n");
        glfwTerminate();
        return -1;
    }

    // OpenGL Initialization
    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    /*
    ################################################
    ##                BUFFER SETUP                ##
    ################################################
    */

    Buffer buffer;
    buffer.width = buffer_width;
    buffer.height = buffer_height;
    buffer.data = new uint32_t[buffer_width * buffer_height];

    // Shaders
    const char* vertex_shader =
        "\n"
        "#version 330\n"
        "\n"
        "noperspective out vec2 TexCoord;\n"
        "\n"
        "void main(void){\n"
        "\n"
        "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
        "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
        "    \n"
        "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
        "}\n";

    const char* fragment_shader =
        "\n"
        "#version 330\n"
        "\n"
        "uniform sampler2D buffer;\n"
        "noperspective in vec2 TexCoord;\n"
        "\n"
        "out vec3 outColor;\n"
        "\n"
        "void main(void){\n"
        "    outColor = texture(buffer, TexCoord).rgb;\n"
        "}\n";
    
    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);

    GLuint shader_id = glCreateProgram();
    // Vertex Shader
    {
        GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shader_vp, 1, &vertex_shader, 0);
        glCompileShader(shader_vp);
        validate_shader(shader_vp, vertex_shader);
        glAttachShader(shader_id, shader_vp);

        glDeleteShader(shader_vp);
    }

    // Fragment Shader
    {
        GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(shader_fp, 1, &fragment_shader, 0);
        glCompileShader(shader_fp);
        validate_shader(shader_fp, fragment_shader);
        glAttachShader(shader_id, shader_fp);

        glDeleteShader(shader_fp);
    }

    glLinkProgram(shader_id);

    if(!validate_program(shader_id))
    {
        fprintf(stderr, "Error while validating shader.\n");
        glfwTerminate();
        glDeleteVertexArrays(1, &fullscreen_triangle_vao);
        delete[] buffer.data;
        return -1;
    }

    // Texture
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);

    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB8,
        buffer.width, buffer.height, 0,
        GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(shader_id);

    GLint location = glGetUniformLocation(shader_id, "buffer");
    glUniform1i(location, 0);

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(fullscreen_triangle_vao);

    /*
    ################################################
    ##                  SPRITES                   ##
    ################################################
    */

    Sprite alien_sprite;
    alien_sprite.width = 11;
    alien_sprite.height = 8;
    alien_sprite.data = new uint8_t[11 * 8]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
        0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
        0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
        0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
    };

    Sprite alien_sprite1;
    alien_sprite1.width = 11;
    alien_sprite1.height = 8;
    alien_sprite1.data = new uint8_t[88]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
    };

    Sprite alien_death_sprite;
    alien_death_sprite.width = 13;
    alien_death_sprite.height = 7;
    alien_death_sprite.data = new uint8_t[91]
    {
        0,1,0,0,1,0,0,0,1,0,0,1,0, // .@..@...@..@.
        0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
        0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
        1,1,0,0,0,0,0,0,0,0,0,1,1, // @@.........@@
        0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
        0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
        0,1,0,0,1,0,0,0,1,0,0,1,0  // .@..@...@..@.
    };

    Sprite player_sprite;
    player_sprite.width = 11;
    player_sprite.height = 7;
    player_sprite.data = new uint8_t[77]
    {
        0,0,0,0,0,1,0,0,0,0,0, // .....@.....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    };

    Sprite projectile_sprite;
    projectile_sprite.width = 1;
    projectile_sprite.height = 3;
    projectile_sprite.data = new uint8_t[3]
    {
        1, // @
        1, // @
        1  // @
    };

    SpriteAnimation* alien_animation = new SpriteAnimation;

    alien_animation->loop = true;
    alien_animation->num_frames = 2;
    alien_animation->frame_duration = 0.5f;
    alien_animation->time = 0.0f;

    alien_animation->frames = new Sprite*[2];
    alien_animation->frames[0] = &alien_sprite;
    alien_animation->frames[1] = &alien_sprite1;

    float player_speed = 60.0f;

    /*
    ################################################
    ##               GAME SETTINGS                ##
    ################################################
    */

    Game game;
    game.width = buffer_width;
    game.height = buffer_height;
    game.num_aliens = 55;
    game.num_projectiles = 0;
    game.aliens = new Alien[game.num_aliens];

    game.player.x = 112 - 5;
    game.player.y = 32;
    game.player.life = 3;

    for (size_t yi = 0; yi < 5; ++yi)
    {
        for (size_t xi = 0; xi < 11; ++xi)
        {
            game.aliens[yi * 11 + xi].x = 16 * xi + 20;
            game.aliens[yi * 11 + xi].y = 17 * yi + 128;
            game.aliens[yi * 11 + xi].type = ALIEN_1;
            game.aliens[yi * 11 + xi].hp = 3;
        }
    }

    uint8_t* death_counters = new uint8_t[game.num_aliens];
    for(size_t i = 0; i < game.num_aliens; ++i)
    {
        death_counters[i] = 10;
    }

    /*
    ################################################
    ##                 GAME LOOP                  ##
    ################################################
    */

    uint32_t clear_color = rgb_to_uint32(255, 192, 203);
    
    game_running = true;
    int player_move_dir = 1;
    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window) && game_running)
    {
        /*
        ### DISPLAY CURRENT FRAME
        */ 
        double current_time = glfwGetTime();
        double dt = current_time - last_time;
        last_time = current_time;

        buffer_clear(&buffer, clear_color);

        for (size_t ai = 0; ai < game.num_aliens; ++ai)
        {
            if(!death_counters[ai]) continue;

            const Alien& alien = game.aliens[ai];
            if (alien.type == ALIEN_DEAD && death_counters[ai])
            {
                draw_sprite_buffer(&buffer, alien_death_sprite, (size_t)alien.x, (size_t)alien.y, rgb_to_uint32(128, 0, 0));
                --death_counters[ai];
            }
            else
            {
                size_t current_frame = (size_t)(alien_animation->time / alien_animation->frame_duration);
                if(current_frame >= alien_animation->num_frames) current_frame = 0;
                const Sprite& sprite = *alien_animation->frames[current_frame];
                draw_sprite_buffer(&buffer, sprite, (size_t)alien.x, (size_t)alien.y, rgb_to_uint32(128, 0, 0));
            }           
        }

        for (size_t bi = 0; bi < game.num_projectiles; ++bi)
        {
            const Projectile& projectile = game.projectiles[bi];
            const Sprite& sprite = projectile_sprite;
            draw_sprite_buffer(&buffer, sprite, projectile.x, projectile.y, rgb_to_uint32(128, 0, 0));
        }

        draw_sprite_buffer(&buffer, player_sprite, (size_t)game.player.x, (size_t)game.player.y, rgb_to_uint32(128, 0, 0));

        alien_animation->time += (float)dt;
        if(alien_animation->time >= alien_animation->num_frames * alien_animation->frame_duration)
        {
            if(alien_animation->loop) 
            {
                alien_animation->time -= alien_animation->num_frames * alien_animation->frame_duration;
            }
            else
            {
                alien_animation->time = 0;
                delete alien_animation;
                alien_animation = nullptr;
            }
        }

        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            buffer.width, buffer.height,
            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
            buffer.data
        );

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glfwSwapBuffers(window);

        /*
        ### PROCESS MOVEMENT FOR NEXT FRAME
        */ 

        for (size_t bi = 0; bi < game.num_projectiles;)
        {
            game.projectiles[bi].y += game.projectiles[bi].dir;
            if (game.projectiles[bi].y >= game.height || 
                game.projectiles[bi].y < projectile_sprite.height)
            {
                game.projectiles[bi] = game.projectiles[game.num_projectiles - 1];
                --game.num_projectiles;
                continue;
            }

            bool projectile_active = true;
            for(size_t ai = 0; ai < game.num_aliens; ++ai)
            {
                const Alien& alien = game.aliens[ai];
                if(alien.type == ALIEN_DEAD) continue;

                const SpriteAnimation& animation = alien_animation[alien.type - 1];
                size_t current_frame = (size_t)(alien_animation->time / alien_animation->frame_duration);
                const Sprite& alien_sprite = *animation.frames[current_frame];
                bool overlap = sprite_overlap_check(
                    projectile_sprite, game.projectiles[bi].x, game.projectiles[bi].y,
                    alien_sprite, (size_t)alien.x, (size_t)alien.y
                );
                if(overlap)
                {
                    if (game.aliens[ai].hp <= 1)
                    {
                        game.aliens[ai].type = ALIEN_DEAD;
                        game.aliens[ai].x -= (alien_death_sprite.width - alien_sprite.width)/2;
                    }
                    else
                    {
                        --game.aliens[ai].hp;
                    }

                    game.projectiles[bi] = game.projectiles[game.num_projectiles - 1];
                    --game.num_projectiles;

                    projectile_active = false;
                    break;
                }
            }

            ++bi;
        }

        player_move_dir = 2 * move_dir;

        if (player_move_dir != 0)
        {
            if(game.player.x + player_sprite.width + (player_move_dir * player_speed * dt) >= game.width - 10)
            {
                game.player.x = game.width - player_sprite.width - player_move_dir - 10;
                player_move_dir *= -1;
            }
            else if(game.player.x + (player_move_dir * player_speed * dt) <= 10)
            {
                game.player.x = 10;
                player_move_dir *= -1;
            }
            else game.player.x += player_move_dir * player_speed * dt;
        }

        if(fire_pressed && game.num_projectiles < GAME_MAX_PROJECTILES)
        {
            game.projectiles[game.num_projectiles].x = (size_t)game.player.x + (size_t)player_sprite.width / 2;
            game.projectiles[game.num_projectiles].y = (size_t)game.player.y + (size_t)player_sprite.height;
            game.projectiles[game.num_projectiles].dir = 2;
            ++game.num_projectiles;
        }
        fire_pressed = false;

        glfwPollEvents();
    }

    delete[] alien_death_sprite.data;

    for(size_t i = 0; i < 3; ++i)
    {
        delete[] alien_animation[i].frames;
    }
    delete[] buffer.data;
    delete[] game.aliens;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}