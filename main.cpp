#include <raylib.h>
#include <iostream>
#include <vector>

const int screenX = 1000;
const int screenY = 1000;

class Ball
{
public:
    // Position
    float x;
    float y;

    // Taille
    float radius;

    // Physique
    float gravity;
    float velocityY;

    // Rebond
    float restitution;

    // Couleur
    Color color;
    // verifier si la colision s aplique
    bool colider;
    // Constructeur
    Ball(float startX, float startY, float startRadius, Color startColor)
    {
        x = startX;
        y = startY;
        radius = startRadius;
        color = startColor;

        gravity = 500.0f;
        velocityY = 0.0f;

        restitution = 0.8f;
    }

    void update(float dt)
    {
        // Gravité
        velocityY += gravity * dt;

        // Déplacement
        y += velocityY * dt;

        // Collision avec le sol
        if (y >= screenY - radius)
        {
            y = screenY - radius;

            // Rebond
            velocityY = -velocityY * restitution;
        }
    }

    void draw()
    {
        DrawCircle(x, y, radius, color);
    }
};
class wall
{
public:
    float x;
    float y;
    float height;
    float width;
    Color color;
    bool colider;
    wall(float wallX, float wallY, float wallH, float wallW, Color wallColor)
    {
        x = wallX;
        y = wallY;
        height = wallH;
        width = wallW;
        color = wallColor;
        colider = true;
    };

    // fonctions
    void draw()
    {
        DrawRectangle(x, y, height, width, color);
    };
};

int main()
{
    InitWindow(screenX, screenY, "Physics Simulator");

    // Liste de toutes les balles
    std::vector<Ball> balls;
    // Première balle
    balls.emplace_back(500, 125, 20, RED);

    std::vector<wall> walls;
    walls.emplace_back(500, 125, 20, 10, RED);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // pour ajouter une balle il faut faire balls.emplace_back(position.x,position.y,20,GREEN)
        // pour compiler  g++ main.cpp -o main.exe -IC:/msys64/ucrt64/include -LC:/msys64/ucrt64/lib -lraylib -lopengl32 -lgdi32 -lwinmm

        // Mise à jour de toutes les balles
        for (Ball &ball : balls)
        {
            ball.update(dt);
        }

        // Dessin
        BeginDrawing();

        ClearBackground(BLACK);

        // Dessiner toutes les balles
        for (Ball &ball : balls)
        {
            ball.draw();
        }
        for (wall &wall : walls)
        {
            wall.draw();
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}