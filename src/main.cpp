#include "Constants.hpp"
#include "Game.hpp"
#include <raylib.h>

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "the neverending world");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    Game game;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        game.update();
        game.render();

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
