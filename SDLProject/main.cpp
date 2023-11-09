#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 13

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* end_scenes;
};

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.86f,
            BG_BLUE    = 0.93f,
            BG_GREEN   = 0.95f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char EAGLE_FILEPATH[] = "assets/eagle.png";
const char CLOUD_FILEPATH[] = "assets/cloud.png";
const char PIGEON_FILEPATH[] = "assets/dove_of_peace.png";

const char WIN_FILEPATH[] ="assets/win.png";
const char LOSE_FILEPATH[] ="assets/lose.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

const float GRAVITY = -0.4f;


// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Project3!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ––––– VIDEO ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    
    // ––––– PLATFORMS ––––– //
    GLuint platform_texture_id = load_texture(CLOUD_FILEPATH);
    
    g_state.platforms = new Entity[PLATFORM_COUNT];
    
    // Set the type of every platform entity to PLATFORM
    for (int i = 0; i < 2; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 5.0f, -1.0f, 0.0f));
        g_state.platforms[i].set_width(0.8f);
        g_state.platforms[i].set_height(0.7f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, 0, NULL);
    }
    for (int i = 2; i < 4; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 5.0f, -2.0f, 0.0f));
        g_state.platforms[i].set_width(0.8f);
        g_state.platforms[i].set_height(0.7f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, 0, NULL);
    }
    for (int i = 4; i < 7; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 5.0f, -2.5f, 0.0f));
        g_state.platforms[i].set_width(0.8f);
        g_state.platforms[i].set_height(0.7f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, 0, NULL);
    }
    
    for (int i = 7; i < 8; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 5.0f, -2.0f, 0.0f));
        g_state.platforms[i].set_width(0.8f);
        g_state.platforms[i].set_height(0.7f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, 0, NULL);
    }
    
    for (int i = 8; i < PLATFORM_COUNT-3; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 5.0f, -1.0f, 0.0f));
        g_state.platforms[i].set_width(0.8f);
        g_state.platforms[i].set_height(0.7f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, 0, NULL);
    }

    g_state.platforms[PLATFORM_COUNT - 3].m_texture_id = platform_texture_id;
    g_state.platforms[PLATFORM_COUNT - 3].set_position(glm::vec3(-0.9f, 1.0f, 0.0f));
    g_state.platforms[PLATFORM_COUNT - 3].set_width(0.8f);
    g_state.platforms[PLATFORM_COUNT - 3].set_height(0.7f);
    g_state.platforms[PLATFORM_COUNT - 3].set_entity_type(PLATFORM);
    g_state.platforms[PLATFORM_COUNT - 3].update(0.0f, g_state.player, NULL, 0, NULL);
    
    g_state.platforms[PLATFORM_COUNT - 2].m_texture_id = platform_texture_id;
    g_state.platforms[PLATFORM_COUNT - 2].set_position(glm::vec3(0.8f, 0.0f, 0.0f));
    g_state.platforms[PLATFORM_COUNT - 2].set_width(0.8f);
    g_state.platforms[PLATFORM_COUNT - 2].set_height(0.7f);
    g_state.platforms[PLATFORM_COUNT - 2].set_entity_type(PLATFORM);
    g_state.platforms[PLATFORM_COUNT - 2].update(0.0f, g_state.player, NULL, 0, NULL);
    
    // ————— POINT ————— //
    g_state.platforms[PLATFORM_COUNT - 1].m_texture_id = load_texture(PIGEON_FILEPATH);
    g_state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(-1.5f, -2.35f, 0.0f));
    g_state.platforms[PLATFORM_COUNT - 1].set_width(0.4f);
    g_state.platforms[PLATFORM_COUNT - 1].set_entity_type(POINT);
    g_state.platforms[PLATFORM_COUNT - 1].update(0.0f, g_state.player, NULL, 0, NULL);
    
    
    // ––––– PLAYER ––––– //
    g_state.player = new Entity();
    g_state.player->set_position(glm::vec3(3.75f, 4.0f, 0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->set_speed(1.0f);
    g_state.player->set_acceleration(glm::vec3(0.0f, GRAVITY, 0.0f));
    g_state.player->m_texture_id = load_texture(EAGLE_FILEPATH);
    
    g_state.player->set_height(0.9f);
    g_state.player->set_width(0.9f);
    g_state.player->set_entity_type(PLAYER);
    
    // ————— ENDING ————— //
    g_state.end_scenes = new Entity[2];
    
    // Win
    g_state.end_scenes[0].m_texture_id = load_texture(WIN_FILEPATH);
    g_state.end_scenes[0].set_position(glm::vec3(0.0f));
    g_state.end_scenes[0].set_entity_type(SCENE);
    g_state.end_scenes[0].update(0.0f, g_state.player, NULL, 0, NULL);
    g_state.end_scenes[0].deactivate();
    // Lose
    g_state.end_scenes[1].m_texture_id = load_texture(LOSE_FILEPATH);
    g_state.end_scenes[1].set_position(glm::vec3(0.0f));
    g_state.end_scenes[1].set_entity_type(SCENE);
    g_state.end_scenes[1].update(0.0f, g_state.player, NULL, 0, NULL);
    g_state.end_scenes[1].deactivate();
    
    
    
    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;
                        
//                    case SDLK_SPACE:
//                        // Jump
//                        if (g_state.player->m_collided_bottom)
//                        {
//                            g_state.player->m_is_jumping = true;
//                        }
//                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_state.player->move_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_state.player->move_right();
    }
    else
    {
        g_state.player->stay_still_x();
    }
    if (key_state[SDL_SCANCODE_UP])
    {
        g_state.player->move_up();
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_state.player->move_down();
    }
    else
    {
        g_state.player->stay_still_y(GRAVITY);
    }
    
    if (glm::length(g_state.player->get_movement()) > 1.0f)
    {
        g_state.player->set_movement(
                                     glm::normalize(
                                                    g_state.player->get_movement()
                                                    )
                                     );
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP)
    {
        g_state.player->update(FIXED_TIMESTEP, g_state.player, g_state.platforms, PLATFORM_COUNT, g_state.end_scenes);
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    g_state.player->render(&g_program);
    
    for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);
    
    g_state.end_scenes[0].render(&g_program);
    g_state.end_scenes[1].render(&g_program);
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete [] g_state.platforms;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
