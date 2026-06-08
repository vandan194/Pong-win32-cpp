#define is_down(b) input->buttons[b].is_down
#define pressed(b) input->buttons[b].is_down && input->buttons[b].changed
#define released(b) !input->buttons[b].is_down && input->buttons[b].changed
enum CurrentScreen {
    INTRO_SCREEN,
    GAMEPLAY,
    WIN,
    GAMEOVER
};
CurrentScreen cscreen = INTRO_SCREEN;
extern Bitmap font;
float ball_speed = 150;
float serve_timer = 0.f;
float game_time = 0.f;
int ai_reaction_x = 20;

internal void handleIntroScreen(float dt);
struct Player
{
    float x, y;
    float vel, acc;
    float half_size_x = 2.5, half_size_y = 10;
    int score = 0;
    void move(float dt)
    {
        acc -= vel * 12.f;
        vel += acc * dt;
        y += vel * dt;
    }
};
struct Ball
{
    float x, y;
    float x_vel = -150, y_vel;
    float half_size_x = 1, half_size_y = 1;

};

Player player = { -80, 0 };
Player player2 = { 80, 0 };
Ball ball;


float arena_half_size_x = 85, arena_half_size_y = 45;

internal void reset_game()
{
    player.score = 0;
    player2.score = 0;

    player.y = 0;
    player.vel = 0;
    player.acc = 0;

    player2.y = 0;
    player2.vel = 0;
    player2.acc = 0;

    ball.x = 0;
    ball.y = 0;
    ball.x_vel = -ball_speed;
    ball.y_vel = 0;

    serve_timer = 1.0f;
}
internal void player_arena_collision(Player& p)
{
    if (p.y + p.half_size_y > arena_half_size_y)
    {
        p.y = arena_half_size_y - p.half_size_y;
        p.vel = 0;
    }
    else if (p.y - p.half_size_y < -arena_half_size_y)
    {
        p.y = -arena_half_size_y + p.half_size_y;
        p.vel = 0;
    }
}
internal void ball_arena_collision(Ball& b)
{
    if (b.y + b.half_size_y > arena_half_size_y)
    {
        b.y = arena_half_size_y - b.half_size_y;
        b.y_vel *= -1;
    }
    else if (b.y - b.half_size_y < -arena_half_size_y)
    {
        b.y = -arena_half_size_y + b.half_size_y;
        b.y_vel *= -1;
    }
    if (b.x + b.half_size_x > arena_half_size_x)
    {
        b.x = 0;
        b.y = 0;
        b.x_vel = ball_speed;
        b.y_vel = 0;
        serve_timer = 1.0f;

        player.score++;
        if (player.score == 10)
        {
            reset_game();
            cscreen = WIN;
        }
    }
    else if (b.x - b.half_size_x < -arena_half_size_x)
    {
        b.x = 0;
        b.y = 0;
        b.x_vel = -ball_speed;
        b.y_vel = 0;
        serve_timer = 1.0f;
        player2.score++;
        if (player2.score == 10)
        {
            reset_game();
            cscreen = GAMEOVER;
        }
    }
}
internal bool ball_collides(Player& paddle)
{
    float ball_left = ball.x - ball.half_size_x,
        ball_right = ball.x + ball.half_size_x,
        paddle_left = paddle.x - paddle.half_size_x,
        paddle_right = paddle.x + paddle.half_size_x,
        ball_top = ball.y + ball.half_size_y,
        ball_bottom = ball.y - ball.half_size_y,
        paddle_top = paddle.y + paddle.half_size_y,
        paddle_bottom = paddle.y - paddle.half_size_y;

    bool x_overlap = ball_right > paddle_left && paddle_right > ball_left;
    bool y_overlap = ball_top > paddle_bottom && paddle_top > ball_bottom;
    return x_overlap && y_overlap;
}

internal void simulate_game(Input* input, float dt)
{
    game_time += dt;
    if (cscreen == GAMEPLAY)
    {
        clear_screen(0x0b468f);
        draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0x102f6b);
        char score_text[32];
        wsprintfA(score_text, "%d", player.score);
        char score_text2[32];
        wsprintfA(score_text2, "%d", player2.score);

        draw_text(font, score_text, -40, 40);
        draw_text(font, score_text2, 40, 40);


        // Reset Acceleration
        player.acc = 0;
        player2.acc = 0;

        // Input Detection 
        if (is_down(BUTTON_UP)) player.acc += 1000;
        if (is_down(BUTTON_DOWN)) player.acc -= 1000;

        player2.acc = 0;
        if (ball.x_vel > 0 && ball.x > ai_reaction_x)
        {
            float target_y = ball.y + ball.y_vel * 0.2f + 4;       //ball.y + ball.y_vel * .5f;
            float error = target_y - player2.y;
            player2.acc = error * 50.f;
            if (player2.acc > 1300) player2.acc = 1300;
            if (player2.acc < -1300) player2.acc = -1300;
        }



        // Movement
        player.move(dt);
        player2.move(dt);

        if (serve_timer > 0)
        {
            serve_timer -= dt;
        }
        else
        {
            ball.x += ball.x_vel * dt;
            ball.y += ball.y_vel * dt;
        }

        // Collision check
        player_arena_collision(player);
        player_arena_collision(player2);
        ball_arena_collision(ball);

        if (ball_collides(player))
        {
            ball.x = player.x + player.half_size_x + ball.half_size_x;
            ball.x_vel *= -1;
            ball.y_vel = (ball.y - player.y) * 5.f + player.vel * 0.2f;
        }
        else if (ball_collides(player2))
        {
            ball.x = player2.x - player2.half_size_x - ball.half_size_x;
            ball.x_vel *= -1;
            ball.y_vel = (ball.y - player2.y) * 1.2f;
        }




        // Draw ball
        draw_rect(ball.x, ball.y, ball.half_size_x, ball.half_size_y, 0xffffff);
        // Draw Players
        draw_rect(player.x, player.y, player.half_size_x, player.half_size_y, 0x92eeff);
        draw_rect(player2.x, player2.y, player2.half_size_x, player2.half_size_y, 0x92eeff);
    }
    else if (cscreen == INTRO_SCREEN) {
        if (pressed(BUTTON_SPACE)) {
            reset_game();
            cscreen = GAMEPLAY;
        }
        handleIntroScreen(dt);
    }
    else if (cscreen == GAMEOVER)
    {
        clear_screen(0x5a0000);
        draw_text(font, "GAME", -45, 30, 8);
        draw_text(font, "OVER", 0, 30, 8);

        if ((int)(game_time * 2) % 2)
        {
            draw_text(font, "PRESS", -45, -22, 4);
            draw_text(font, "SPACE", -15, -22, 4);
            draw_text(font, "TO", 15, -22, 4);
            draw_text(font, "RETRY", 28, -22, 4);

        }
        if (pressed(BUTTON_SPACE)) {
            reset_game();
            cscreen = GAMEPLAY;
        }
    }
    else if (cscreen == WIN)
    {
        clear_screen(0x006600);
        draw_text(font, "VICTORY", -30, 30, 8);

        if ((int)(game_time * 2) % 2)
        {
            draw_text(font, "PRESS", -45, -22, 4);
            draw_text(font, "SPACE", -15, -22, 4);
            draw_text(font, "TO", 15, -22, 4);
            draw_text(font, "PLAY", 28, -22, 4);

        }
        if (pressed(BUTTON_SPACE)) {
            reset_game();
            cscreen = GAMEPLAY;
        }
    }
}
internal void handleIntroScreen(float dt)
{
    clear_screen(0x000000);
    if ((int)(game_time * 1) % 2)
    {
        draw_text(font, "PONG", -20, 30, 10);

    }
    draw_text(font, "PONG", -21, 30, 10);
    draw_text(font, "VANDAN", 54, -45);
    draw_rect(ball.x, ball.y, 1, 1, 0xdddddd);


    if ((int)(game_time * 2) % 2)
    {
        draw_text(font, "Press", -25, -22, 4);
        draw_text(font, "Space", 5, -22, 4);

    }

    ball.x += ball.x_vel * dt;
    ball.y += ball.y_vel * dt;

    if (ball.x + ball.half_size_x > arena_half_size_x)
    {
        ball.x = arena_half_size_x - ball.half_size_x;
        ball.x_vel *= -1;
    }

    if (ball.x - ball.half_size_x < -arena_half_size_x)
    {
        ball.x = -arena_half_size_x + ball.half_size_x;
        ball.x_vel *= -1;
    }


}