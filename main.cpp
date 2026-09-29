#include <raylib.h>
#include <iostream>
#include <vector>
#include <cmath>

const int screenX = 1000;
const int screenY = 1000;

// ============================================================
// CALCUL DE DIRECTION
// ============================================================

std::vector<float> calculateAngle(
    float playerX,
    float playerY,
    float cursorX,
    float cursorY)
{
    float dx = playerX - cursorX;
    float dy = playerY - cursorY;

    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance == 0)
        return {0, 0};

    dx /= distance;
    dy /= distance;

    return {dx, dy};
}

// ============================================================
// PARTICULE
// ============================================================

class Particle
{
public:
    float x;
    float y;

    float velocityX;
    float velocityY;

    float lifetime;
    float maxLifetime;

    float radius;

    Color color;

    Particle(
        float startX,
        float startY,
        float startVelocityX,
        float startVelocityY)
    {
        x = startX;
        y = startY;

        velocityX = startVelocityX;
        velocityY = startVelocityY;

        lifetime = 0.5f;
        maxLifetime = 0.5f;

        radius = 4.0f;

        color = ORANGE;
    }

    void update(float dt)
    {
        x += velocityX * dt;
        y += velocityY * dt;

        lifetime -= dt;
    }

    bool isDead()
    {
        return lifetime <= 0;
    }

    void draw()
    {
        float alpha = lifetime / maxLifetime;

        Color particleColor = color;

        particleColor.a = (unsigned char)(255 * alpha);

        DrawCircle(
            x,
            y,
            radius,
            particleColor);
    }
};

// ============================================================
// MUR
// ============================================================

class wall
{
public:
    float x;
    float y;

    float height;
    float width;

    Color color;

    bool colider;

    wall(
        float wallX,
        float wallY,
        float wallH,
        float wallW,
        Color wallColor)
    {
        x = wallX;
        y = wallY;

        height = wallH;
        width = wallW;

        color = wallColor;

        colider = true;
    }

    void draw()
    {
        DrawRectangle(
            x,
            y,
            height,
            width,
            color);
    }
};

// ============================================================
// JOUEUR
// ============================================================

class player
{
public:
    // Position
    float x;
    float y;

    // Taille
    float height;
    float width;

    // Ancienne position
    float previousX;
    float previousY;

    // Physique
    float gravity;

    float velocityX;
    float velocityY;

    float restitution;

    Color color;

    player(
        float playerX,
        float playerY,
        float playerH,
        float playerW,
        Color playerColor)
    {
        x = playerX;
        y = playerY;

        height = playerH;
        width = playerW;

        previousX = x;
        previousY = y;

        color = playerColor;

        gravity = 500.0f;

        velocityX = 0.0f;
        velocityY = 0.0f;

        restitution = 0.8f;
    }

    // ========================================================
    // UPDATE
    // ========================================================

    void update(float dt)
    {
        // On sauvegarde la position précédente
        previousX = x;
        previousY = y;

        // Gravité
        velocityY += gravity * dt;

        // Déplacement
        x += velocityX * dt;
        y += velocityY * dt;

        // Sol
        if (y + height >= screenY)
        {
            y = screenY - height;

            velocityY = 0.0f;
            velocityX = 0.0f;
        }

        // Mur gauche
        if (x < 0)
        {
            x = 0;
            velocityX = 0;
        }

        // Mur droit
        if (x + width > screenX)
        {
            x = screenX - width;
            velocityX = 0;
        }
    }

    // ========================================================
    // REPOUSSE LE JOUEUR
    // ========================================================

    void pushBack(
        float cursorX,
        float cursorY)
    {
        std::vector<float> direction =
            calculateAngle(
                x + width / 2,
                y + height / 2,
                cursorX,
                cursorY);

        float pushForce = 500.0f;

        velocityX =
            direction[0] * pushForce;

        velocityY =
            direction[1] * pushForce;
    }

    // ========================================================
    // COLLISION AVEC UN MUR
    // ========================================================

    void checkWallCollision(wall &Wall)
    {
        // Joueur
        float playerLeft = x;
        float playerRight = x + width;

        float playerTop = y;
        float playerBottom = y + height;

        // Mur
        float wallLeft = Wall.x;
        float wallRight = Wall.x + Wall.height;

        float wallTop = Wall.y;
        float wallBottom = Wall.y + Wall.width;

        // Vérifie si les deux rectangles se touchent
        bool collision =
            playerRight > wallLeft &&
            playerLeft < wallRight &&
            playerBottom > wallTop &&
            playerTop < wallBottom;

        if (!collision)
            return;

        // ====================================================
        // COLLISION PAR LE DESSUS
        // ====================================================

        float previousBottom =
            previousY + height;

        if (
            previousBottom <= wallTop &&
            velocityY >= 0)
        {
            y = wallTop - height;

            velocityY = 0;
            velocityX = 0;

            return;
        }

        // ====================================================
        // COLLISION PAR LE DESSOUS
        // ====================================================

        float previousTop =
            previousY;

        if (
            previousTop >= wallBottom &&
            velocityY < 0)
        {
            y = wallBottom;

            velocityY = 0;

            return;
        }

        // ====================================================
        // COLLISION PAR LA GAUCHE
        // ====================================================

        float previousRight =
            previousX + width;

        if (
            previousRight <= wallLeft &&
            velocityX > 0)
        {
            x = wallLeft - width;

            velocityX = 0;

            return;
        }

        // ====================================================
        // COLLISION PAR LA DROITE
        // ====================================================

        float previousLeft =
            previousX;

        if (
            previousLeft >= wallRight &&
            velocityX < 0)
        {
            x = wallRight;

            velocityX = 0;

            return;
        }
    }

    // ========================================================
    // ARME
    // ========================================================

    void drawWeapon()
    {
        Vector2 mouse =
            GetMousePosition();

        float centerX =
            x + width / 2;

        float centerY =
            y + height / 2;

        float dx =
            mouse.x - centerX;

        float dy =
            mouse.y - centerY;

        float angle =
            atan2(dy, dx) * 180.0f / PI;

        float weaponLength = 50.0f;

        float weaponWidth = 10.0f;

        Rectangle weapon =
            {
                centerX,
                centerY - weaponWidth / 2,
                weaponLength,
                weaponWidth};

        Vector2 origin =
            {
                0,
                weaponWidth / 2};

        DrawRectanglePro(
            weapon,
            origin,
            angle,
            DARKGRAY);
    }

    // ========================================================
    // FIN DE L'ARME
    // ========================================================

    Vector2 getWeaponEnd()
    {
        Vector2 mouse =
            GetMousePosition();

        float centerX =
            x + width / 2;

        float centerY =
            y + height / 2;

        float dx =
            mouse.x - centerX;

        float dy =
            mouse.y - centerY;

        float distance =
            sqrt(
                dx * dx +
                dy * dy);

        if (distance == 0)
        {
            dx = 1;
            dy = 0;
        }
        else
        {
            dx /= distance;
            dy /= distance;
        }

        float weaponLength = 50.0f;

        Vector2 weaponEnd;

        weaponEnd.x =
            centerX +
            dx * weaponLength;

        weaponEnd.y =
            centerY +
            dy * weaponLength;

        return weaponEnd;
    }

    // ========================================================
    // DESSIN DU JOUEUR
    // ========================================================

    void draw()
    {
        // Corps
        DrawRectangle(
            x,
            y,
            width,
            height,
            color);

        // Tête
        DrawCircle(
            x + width / 2,
            y,
            width / 2,
            color);

        // Arme
        drawWeapon();
    }
};

// ============================================================
// BALLE
// ============================================================

class Ball
{
public:
    float x;
    float y;

    float radius;

    float gravity;

    float velocityY;

    float restitution;

    Color color;

    bool colider;

    Ball(
        float startX,
        float startY,
        float startRadius,
        Color startColor)
    {
        x = startX;
        y = startY;

        radius = startRadius;

        color = startColor;

        gravity = 500.0f;

        velocityY = 0.0f;

        restitution = 0.8f;

        colider = true;
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

            velocityY =
                -velocityY * restitution;
        }
    }

    void draw()
    {
        DrawCircle(
            x,
            y,
            radius,
            color);
    }
};

// ============================================================
// PARTICULES DE TIR
// ============================================================

void shootParticles(
    std::vector<Particle> &particles,
    player &Player)
{
    Vector2 mouse =
        GetMousePosition();

    float centerX =
        Player.x +
        Player.width / 2;

    float centerY =
        Player.y +
        Player.height / 2;

    float dx =
        mouse.x - centerX;

    float dy =
        mouse.y - centerY;

    float distance =
        sqrt(
            dx * dx +
            dy * dy);

    if (distance == 0)
        return;

    dx /= distance;
    dy /= distance;

    Vector2 weaponEnd =
        Player.getWeaponEnd();

    for (int i = 0; i < 8; i++)
    {
        float randomX =
            (float)GetRandomValue(
                -30,
                30) /
            10.0f;

        float randomY =
            (float)GetRandomValue(
                -30,
                30) /
            10.0f;

        float speed =
            GetRandomValue(
                100,
                250);

        float velocityX =
            dx * speed +
            randomX;

        float velocityY =
            dy * speed +
            randomY;

        particles.emplace_back(
            weaponEnd.x,
            weaponEnd.y,
            velocityX,
            velocityY);
    }
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    // ========================================================
    // FENÊTRE
    // ========================================================

    InitWindow(
        screenX,
        screenY,
        "Physics Simulator");

    SetTargetFPS(60);

    // ========================================================
    // JOUEUR
    // ========================================================

    player Player(
        500,
        300,
        50,
        30,
        BLUE);

    // ========================================================
    // BALLES
    // ========================================================

    std::vector<Ball> balls;

    balls.emplace_back(
        500,
        125,
        20,
        RED);

    // ========================================================
    // MURS
    // ========================================================

    std::vector<wall> walls;

    // Mur horizontal vert
    walls.emplace_back(
        400,
        600,
        200,
        30,
        GREEN);

    // Mur horizontal rouge
    walls.emplace_back(
        700,
        400,
        200,
        30,
        RED);

    // Mur vertical orange
    walls.emplace_back(
        100,
        100,
        30,
        300,
        ORANGE);

    // ========================================================
    // PARTICULES
    // ========================================================

    std::vector<Particle> particles;

    // ========================================================
    // BOUCLE PRINCIPALE
    // ========================================================

    while (!WindowShouldClose())
    {
        // Delta time
        float dt =
            GetFrameTime();

        // ====================================================
        // TIR
        // ====================================================

        if (
            IsMouseButtonPressed(
                MOUSE_BUTTON_LEFT))
        {
            // Particules
            shootParticles(
                particles,
                Player);

            // Repousse le joueur
            Vector2 mouse =
                GetMousePosition();

            Player.pushBack(
                mouse.x,
                mouse.y);
        }

        // ====================================================
        // UPDATE JOUEUR
        // ====================================================

        Player.update(dt);

        // ====================================================
        // COLLISION JOUEUR / MURS
        // ====================================================

        for (wall &Wall : walls)
        {
            Player.checkWallCollision(
                Wall);
        }

        // ====================================================
        // UPDATE BALLES
        // ====================================================

        for (Ball &ball : balls)
        {
            ball.update(dt);
        }

        // ====================================================
        // UPDATE PARTICULES
        // ====================================================

        for (Particle &particle : particles)
        {
            particle.update(dt);
        }

        // ====================================================
        // SUPPRESSION DES PARTICULES MORTES
        // ====================================================

        for (
            int i = particles.size() - 1;
            i >= 0;
            i--)
        {
            if (particles[i].isDead())
            {
                particles.erase(
                    particles.begin() + i);
            }
        }

        // ====================================================
        // DESSIN
        // ====================================================

        BeginDrawing();

        ClearBackground(BLACK);

        // Joueur
        Player.draw();

        // Balles
        for (Ball &ball : balls)
        {
            ball.draw();
        }

        // Murs
        for (wall &Wall : walls)
        {
            Wall.draw();
        }

        // Particules
        for (Particle &particle : particles)
        {
            particle.draw();
        }

        EndDrawing();
    }

    // ========================================================
    // FERMETURE
    // ========================================================

    CloseWindow();

    return 0;
}
