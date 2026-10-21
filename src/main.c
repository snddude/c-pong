#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

#include "sound.h"

#define TITLE      "Pong"
#define WIDTH      640
#define HEIGHT     480
#define TARGET_FPS 60

#define BOARD_CENTER_X WIDTH / 2.0f
#define BOARD_CENTER_Y HEIGHT / 2.0f

#define NET_SEGMENT_COUNT  65
#define NET_SEGMENT_HEIGHT HEIGHT / NET_SEGMENT_COUNT

#define SCORE_RENDER_SCALE       2.0f
#define LEFT_SCORE_POS_X(score)  ((WIDTH - BOARD_CENTER_X) / 2.0f - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_snprintf(NULL, 0, "%d", score)) * (1.0f / SCORE_RENDER_SCALE)
#define LEFT_SCORE_POS_Y         ((HEIGHT - BOARD_CENTER_Y) / 2.0f) * (1.0f / SCORE_RENDER_SCALE)
#define RIGHT_SCORE_POS_X(score) (BOARD_CENTER_X + (BOARD_CENTER_X / 2.0f) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_snprintf(NULL, 0, "%d", score)) * (1.0f / SCORE_RENDER_SCALE)
#define RIGHT_SCORE_POS_Y        ((HEIGHT - BOARD_CENTER_Y) / 2.0f) * (1.0f / SCORE_RENDER_SCALE)

#define BALL_SIZE     8.0f
#define BALL_SPEED    300.0f
#define BALL_COOLDOWN 1500

#define PADDLE_WIDTH  BALL_SIZE * 1.5f
#define PADDLE_HEIGHT BALL_SIZE * 8.0f
#define PADDLE_SPEED  BALL_SPEED * 0.85f

#define SND_HIT_VOLUME   0.25f
#define SND_SCORE_VOLUME 0.15f

#define EXIT_ERR { exit_code = EXIT_FAILURE; goto exit; }

int get_input_axis(SDL_Keycode negative, SDL_Keycode positive)
{
    const bool *keys = SDL_GetKeyboardState(NULL);
    return keys[SDL_GetScancodeFromKey(positive, NULL)] - keys[SDL_GetScancodeFromKey(negative, NULL)];
}

uint32_t start_ball(void *userdata, SDL_TimerID timerID, Uint32 interval)
{
    /* Set ball_on_cooldown to false to resume ball's movement after interval */
    *(bool *)userdata = false;
    return 0;
}

int main(void)
{
    uint8_t exit_code = EXIT_SUCCESS;

    uint32_t left_score = 0;
    uint32_t right_score = 0;
    uint64_t last = 0;
	uint64_t now = SDL_GetPerformanceCounter();

    bool ball_on_cooldown = true;

    float left_paddle_dy = 0.0f;
    float right_paddle_dy = 0.0f;
    float ball_dx = 1.0f;
    float ball_dy = 1.0f;

    double delta = 0.0;

    SDL_Event event = {0};
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    SDL_FRect ball = {
        .w = BALL_SIZE,
        .h = BALL_SIZE,
        .x = WIDTH / 2 - BALL_SIZE / 2,
        .y = HEIGHT / 2 - BALL_SIZE / 2
    };
    SDL_FRect left_paddle = {
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT,
        .x = 0 + BALL_SIZE * 2.0f,
        .y = HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f
    };
    SDL_FRect right_paddle = {
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT,
        .x = WIDTH - PADDLE_WIDTH - BALL_SIZE * 2.0f,
        .y = HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f
    };

    sound_t snd_hit;
    sound_t snd_score;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("SDL_Init() failed: %s", SDL_GetError());
        EXIT_ERR;
    }

    window = SDL_CreateWindow(TITLE, WIDTH, HEIGHT, 0);
    if (window == NULL)
    {
        SDL_Log("SDL_CreateWindow() failed: %s", SDL_GetError());
        EXIT_ERR;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
    {
        SDL_Log("SDL_CreateRenderer() failed: %s", SDL_GetError());
        EXIT_ERR;  
    }

    if (!sound_init(&snd_hit, "res/sound/hit.wav", SND_HIT_VOLUME))
    {
        SDL_Log("Failed to initialize hit sound");
        EXIT_ERR;
    }

    if (!sound_init(&snd_score, "res/sound/score.wav", SND_SCORE_VOLUME))
    {
        SDL_Log("Failed to initialize score sound");
        EXIT_ERR;
    }

    SDL_AddTimer(BALL_COOLDOWN, start_ball, &ball_on_cooldown);

    while (true)
    {
        while (SDL_PollEvent(&event))
            if (event.type == SDL_EVENT_QUIT)
                goto exit;

        /* Calculate delta time */
        last = now;
		now = SDL_GetPerformanceCounter();
		delta = (double)(now - last) / (double)SDL_GetPerformanceFrequency();

        /* Update left paddle position */
        left_paddle_dy = get_input_axis(SDLK_W, SDLK_S);
        left_paddle.y += PADDLE_SPEED * delta * left_paddle_dy;
        left_paddle.y = SDL_clamp(left_paddle.y, 0.0f, HEIGHT - PADDLE_HEIGHT);

        /* Update right paddle position */
        right_paddle_dy = get_input_axis(SDLK_UP, SDLK_DOWN);
        right_paddle.y += PADDLE_SPEED * delta * right_paddle_dy;
        right_paddle.y = SDL_clamp(right_paddle.y, 0.0f, HEIGHT - PADDLE_HEIGHT);

        /* Update ball position */
        if (SDL_HasRectIntersectionFloat(&ball, &left_paddle))
        {
            ball.x = left_paddle.x + BALL_SIZE + 1.0f;
            ball_dx *= -1;
            sound_play(&snd_hit);
        }

        if (SDL_HasRectIntersectionFloat(&ball, &right_paddle))
        {
            ball.x = right_paddle.x - BALL_SIZE - 1.0f;
            ball_dx *= -1;
            sound_play(&snd_hit);
        }

        if (ball.y <= 0.0f || ball.y + BALL_SIZE >= HEIGHT)
        {
            ball_dy *= -1;
            sound_play(&snd_hit);
        }

        if (!ball_on_cooldown)
        {
            ball.x += BALL_SPEED * delta * ball_dx;
            ball.y += BALL_SPEED * delta * ball_dy;
        }

        /* Check if the left or right scored */
        if (ball.x > WIDTH)
        {
            left_score++;
            ball_dx *= -1;

            sound_play(&snd_score);

            ball_on_cooldown = true;
            ball.x = BOARD_CENTER_X - BALL_SIZE / 2.0f;
            ball.y = BOARD_CENTER_Y - BALL_SIZE / 2.0f;

            SDL_AddTimer(BALL_COOLDOWN, start_ball, &ball_on_cooldown);
        }

        if (ball.x < 0.0f)
        {
            right_score++;
            ball_dx *= -1;

            sound_play(&snd_score);

            ball_on_cooldown = true;
            ball.x = BOARD_CENTER_X - BALL_SIZE / 2.0f;
            ball.y = BOARD_CENTER_Y - BALL_SIZE / 2.0f;

            SDL_AddTimer(BALL_COOLDOWN, start_ball, &ball_on_cooldown);
        }

        SDL_SetRenderDrawColor(renderer, 24, 24, 24, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        /* Render the scores */
        SDL_SetRenderScale(renderer, SCORE_RENDER_SCALE, SCORE_RENDER_SCALE);
        SDL_RenderDebugTextFormat(renderer, LEFT_SCORE_POS_X(left_score), LEFT_SCORE_POS_Y, "%d", left_score);
        SDL_RenderDebugTextFormat(renderer, RIGHT_SCORE_POS_X(right_score), RIGHT_SCORE_POS_Y, "%d", right_score);
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);

        /* Render the net */
        for (size_t i = 0; i < NET_SEGMENT_COUNT; i++)
            if (i % 2 == 0)
            {
                float segment_start = i * NET_SEGMENT_HEIGHT;
                SDL_RenderLine(renderer, BOARD_CENTER_X, segment_start, BOARD_CENTER_X, segment_start + NET_SEGMENT_HEIGHT);
            }

        /* Render the paddles and the ball */
        SDL_RenderFillRect(renderer, &left_paddle);
        SDL_RenderFillRect(renderer, &right_paddle);
        SDL_RenderFillRect(renderer, &ball);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / TARGET_FPS);
    }

exit:
    sound_destroy(&snd_hit);
    sound_destroy(&snd_score);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return exit_code;
}
