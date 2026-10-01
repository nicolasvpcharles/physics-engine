#include <raylib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <string>
#include <cstdlib>

// ============================================================
// TAILLE DE L'ECRAN
// ============================================================

int screenX = 0;
int screenY = 0;

const int targetFps = 60;

int numberOfEnemies = 1;
int maxEnemies = 20;

long long score = 0;

bool colisions=true;
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

    float distance =
        std::sqrt(dx * dx + dy * dy);

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
        float alpha =
            lifetime / maxLifetime;

        if (alpha < 0)
            alpha = 0;

        Color particleColor = color;

        particleColor.a =
            (unsigned char)(255 * alpha);

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
    float x;
    float y;

    float height;
    float width;

    float previousX;
    float previousY;

    float gravity;

    float velocityX;
    float velocityY;

    int bulets;
    int maxBulets;

    float restitution;
    float health;

    // Cooldown entre deux dégâts
    float damageCooldown;

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

        health = 100;

        gravity = 500.0f;

        velocityX = 0.0f;
        velocityY = 0.0f;

        restitution = 0.8f;

        bulets = 5;
        maxBulets = 5;

        damageCooldown = 0;
    }

    // ========================================================
    // UPDATE
    // ========================================================

    void update(float dt)
    {
        previousX = x;
        previousY = y;

        // Cooldown dégâts
        if (damageCooldown > 0)
        {
            damageCooldown -= dt;

            if (damageCooldown < 0)
                damageCooldown = 0;
        }

        // Gravité
        velocityY += gravity * dt;

        // Déplacement
        x += velocityX * dt;
        y += velocityY * dt;

        // ====================================================
        // SOL
        // ====================================================

        if (y + height >= screenY)
        {
            y = screenY - height;

            velocityY = 0;
            velocityX = 0;

            health -= 1;

            if (health < 0)
                health = 0;
        }

        // ====================================================
        // MUR GAUCHE
        // ====================================================

        if (x < 0)
        {
            x = 0;

            velocityX = 0;
        }

        // ====================================================
        // MUR DROIT
        // ====================================================

        if (x + width > screenX)
        {
            x = screenX - width;

            velocityX = 0;
        }

        // ====================================================
        // PLAFOND
        // ====================================================

        if (y < 0)
        {
            y = 0;

            velocityY = 0;
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

        float pushForce = 400.0f;

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
        float playerLeft = x;
        float playerRight = x + width;

        float playerTop = y;
        float playerBottom = y + height;

        float wallLeft = Wall.x;
        float wallRight =
            Wall.x + Wall.height;

        float wallTop = Wall.y;
        float wallBottom =
            Wall.y + Wall.width;

        bool collision =
            playerRight > wallLeft &&
            playerLeft < wallRight &&
            playerBottom > wallTop &&
            playerTop < wallBottom;

        if (!collision)
            return;

        // ====================================================
        // DESSUS
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
        // DESSOUS
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
        // GAUCHE
        // ====================================================

        float previousRight =
            previousX + width;

        if (
            previousRight <= wallLeft &&
            velocityX > 0)
        {
            x = wallLeft - width;

            velocityY =
                velocityY / 2;

            velocityX = 0;

            return;
        }

        // ====================================================
        // DROITE
        // ====================================================

        float previousLeft =
            previousX;

        if (
            previousLeft >= wallRight &&
            velocityX < 0)
        {
            x = wallRight;

            velocityY =
                velocityY / 2;

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
            atan2(dy, dx) *
            180.0f /
            PI;

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
    // DESSIN
    // ========================================================

    void draw()
    {
        DrawRectangle(
            x,
            y,
            width,
            height,
            color);

        DrawCircle(
            x + width / 2,
            y,
            width / 2,
            color);

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
        float ballX,
        float ballY,
        float ballRadius,
        Color ballColor)
    {
        x = ballX;
        y = ballY;

        radius = ballRadius;

        gravity = 500.0f;

        velocityY = 0;

        restitution = 0.8f;

        color = ballColor;

        colider = true;
    }

    void update(float dt)
    {
        velocityY += gravity * dt;

        y += velocityY * dt;

        if (y >= screenY - radius)
        {
            y = screenY - radius;

            velocityY =
                -velocityY *
                restitution;
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
// TIR DE PARTICULES
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
            GetRandomValue(-30, 30) / 10.0f;

        float randomY =
            GetRandomValue(-30, 30) / 10.0f;

        float speed =
            GetRandomValue(100, 250);

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
// ENNEMI
// ============================================================

class enemyA
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

    enemyA(
        float enemyY,
        float enemyX,
        float enemyRadius,
        Color enemyColor)
    {
        x = enemyX;
        y = enemyY;

        radius = enemyRadius;

        gravity = 500.0f;

        velocityY = 0;

        restitution = 0.8f;

        color = enemyColor;

        colider = true;
    }

    // ========================================================
    // UPDATE ENNEMI
    // ========================================================

    void update(float dt)
    {
        float speed = 60.0f;

        x -= speed * dt;
    }

    // ========================================================
    // DESSIN
    // ========================================================

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
// MAIN
// ============================================================

int main()
{
    // ========================================================
    // FENETRE
    // ========================================================

    InitWindow(
        1280,
        720,
        "Physics Simulator");

    // ========================================================
    // MONITEUR
    // ========================================================

    int monitorWidth =
        GetMonitorWidth(0);

    int monitorHeight =
        GetMonitorHeight(0);

    // ========================================================
    // FULLSCREEN WINDOWED
    // ========================================================

    SetWindowState(
        FLAG_WINDOW_UNDECORATED);

    SetWindowSize(
        monitorWidth,
        monitorHeight);

    SetWindowPosition(
        0,
        0);

    screenX =
        GetScreenWidth();

    screenY =
        GetScreenHeight();

    SetTargetFPS(targetFps);

    // ========================================================
    // JOUEUR
    // ========================================================

    player Player(
        screenX / 2,
        screenY / 3,
        50,
        30,
        BLUE);

    // ========================================================
    // BALLES
    // ========================================================

    std::vector<Ball> balls;

    balls.emplace_back(
        screenX / 2,
        125,
        20,
        GRAY);

    // ========================================================
    // MURS
    // ========================================================

    std::vector<wall> walls;

    walls.emplace_back(
        screenX * 0.26f,
        screenY * 0.60f,
        screenX * 0.13f,
        30,
        GREEN);

    walls.emplace_back(
        screenX * 0.46f,
        screenY * 0.40f,
        screenX * 0.13f,
        30,
        RED);

    walls.emplace_back(
        screenX * 0.06f,
        screenY * 0.10f,
        30,
        screenY * 0.30f,
        ORANGE);

    // ========================================================
    // PARTICULES
    // ========================================================

    std::vector<Particle> particles;

    // ========================================================
    // ENNEMIS
    // ========================================================

    std::vector<enemyA> enemies;

    // ========================================================
    // CHRONOMETRES
    // ========================================================

    auto lastEnemySpawn =
        std::chrono::steady_clock::now();

    auto lastBulletRecharge =
        std::chrono::steady_clock::now();

    auto event =
        std::chrono::steady_clock::now();

    // ========================================================
    // FULLSCREEN
    // ========================================================

    bool trueFullscreen = false;

    // ========================================================
    // BOUCLE PRINCIPALE
    // ========================================================

    while (!WindowShouldClose())
    {
        // ====================================================
        // F11
        // ====================================================

        if (IsKeyPressed(KEY_F11))
        {
            if (!trueFullscreen)
            {
                ToggleFullscreen();

                trueFullscreen = true;
            }
            else
            {
                ToggleFullscreen();

                SetWindowState(
                    FLAG_WINDOW_UNDECORATED);

                SetWindowSize(
                    monitorWidth,
                    monitorHeight);

                SetWindowPosition(
                    0,
                    0);

                trueFullscreen = false;
            }

            screenX =
                GetScreenWidth();

            screenY =
                GetScreenHeight();
        }

        // ====================================================
        // TEMPS ACTUEL
        // ====================================================

        auto now =
            std::chrono::steady_clock::now();

        // ====================================================
        // SPAWN DES ENNEMIS
        // ====================================================

        auto enemyElapsed =
            std::chrono::duration_cast<
                std::chrono::seconds>(
                now - lastEnemySpawn);

        if (enemyElapsed.count() >= 5)
        {
            std::cout
                << "Spawn d'un ennemi"
                << std::endl;

            int i = 0;

            int n =
                1 +
                std::rand() %
                    numberOfEnemies;

            while (i < n)
            {
                enemies.emplace_back(
                    std::rand() % screenY,

                    screenX +
                        std::rand() %
                            (screenX / 20),

                    5,

                    PURPLE);

                i++;
            }

            // Augmente progressivement
            // le nombre maximum d'ennemis

            if (numberOfEnemies < maxEnemies)
            {
                numberOfEnemies++;
            }
            score += numberOfEnemies;
            lastEnemySpawn = now;
        }

        // ====================================================
        // EVENEMENT BOSS
        // ====================================================

        auto eventSpawnBoss =
            std::chrono::duration_cast<
                std::chrono::seconds>(
                now - event);

        if (eventSpawnBoss.count() >= 30)
        {
            std::cout
                << "================================"
                << std::endl;

            std::cout
                << "       EVENT BOSS SPAWN !"
                << std::endl;

            std::cout
                << "================================"
                << std::endl;

            // IMPORTANT :
            // on reset le timer

            event = now;
        }

        // ====================================================
        // RECHARGE DES BALLES
        // ====================================================

        auto bulletElapsed =
            std::chrono::duration_cast<
                std::chrono::seconds>(
                now - lastBulletRecharge);

        if (bulletElapsed.count() >= 1)
        {
            if (Player.bulets <
                Player.maxBulets)
            {
                Player.bulets++;
            }

            lastBulletRecharge = now;
        }

        // ====================================================
        // DELTA TIME
        // ====================================================

        float dt =
            GetFrameTime();

        // ====================================================
        // TIR
        // ====================================================

        if (
            IsMouseButtonPressed(
                MOUSE_BUTTON_LEFT))
        {
            if (Player.bulets > 0)
            {
                shootParticles(
                    particles,
                    Player);

                Vector2 mouse =
                    GetMousePosition();

                Player.pushBack(
                    mouse.x,
                    mouse.y);

                Player.bulets--;
            }
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
        // UPDATE ENNEMIS
        // ====================================================

        for (enemyA &enemy : enemies)
        {
            enemy.update(dt);
        }

        // ====================================================
        // COLLISION JOUEUR / ENNEMIS
        // ====================================================

        for (enemyA &enemy : enemies)
        {
            bool collision =
                Player.x <
                    enemy.x +
                        enemy.radius &&

                Player.x +
                        Player.width >
                    enemy.x -
                        enemy.radius &&

                Player.y <
                    enemy.y +
                        enemy.radius &&

                Player.y +
                        Player.height >
                    enemy.y -
                        enemy.radius;

            if (collision)
            {
                // Seulement si le cooldown
                // est terminé

                if (Player.damageCooldown <= 0)
                {
                    std::cout
                        << "COLLISION !"
                        << std::endl;

                    Player.health -= 10;

                    if (Player.health < 0)
                        Player.health = 0;

                    // 0.5 seconde
                    // d'invincibilité

                    Player.damageCooldown =
                        0.5f;
                }
            }
        }

        // ====================================================
        // SUPPRESSION PARTICULES
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
        // SUPPRESSION ENNEMIS
        // ====================================================

        for (
            int i = enemies.size() - 1;
            i >= 0;
            i--)
        {
            if (enemies[i].x < 0)
            {
                enemies.erase(
                    enemies.begin() + i);
            }
        }

        // ====================================================
        // GAME OVER
        // ====================================================

        if (Player.health <= 0)
        {
            CloseWindow();

            return 0;
        }

        // ====================================================
        // DESSIN
        // ====================================================

        BeginDrawing();

        ClearBackground(BLACK);

        // ====================================================
        // JOUEUR
        // ====================================================

        Player.draw();

        // ====================================================
        // BALLES
        // ====================================================

        for (Ball &ball : balls)
        {
            ball.draw();
        }

        // ====================================================
        // MURS
        // ====================================================

        for (wall &Wall : walls)
        {
            Wall.draw();
        }

        // ====================================================
        // PARTICULES
        // ====================================================

        for (Particle &particle : particles)
        {
            particle.draw();
        }

        // ====================================================
        // ENNEMIS
        // ====================================================

        for (enemyA &enemy : enemies)
        {
            enemy.draw();
        }

        // ====================================================
        // SCORE
        // ====================================================

        std::string scoreMessage =
            "Score : " +
            std::to_string(score);

        DrawText(
            scoreMessage.c_str(),
            screenX / 2,
            0,
            25,
            WHITE);

        // ====================================================
        // FPS
        // ====================================================

        int fps = 0;

        if (dt > 0)
        {
            fps =
                (int)(1.0f / dt);
        }

        std::string fpsText =
            "FPS : " +
            std::to_string(fps);

        DrawText(
            fpsText.c_str(),
            screenX - 100,
            20,
            15,
            WHITE);

        // ====================================================
        // VIE
        // ====================================================

        std::string playerHealth =
            "HP : " +
            std::to_string(
                (int)Player.health);

        DrawText(
            playerHealth.c_str(),
            screenX / 100,
            20,
            25,
            RED);

        // ====================================================
        // BALLES
        // ====================================================

        std::string buletsText =
            "Ammo : " +
            std::to_string(
                Player.bulets) +
            " / " +
            std::to_string(
                Player.maxBulets);

        DrawText(
            buletsText.c_str(),
            screenX - 150,
            screenY - 40,
            25,
            GREEN);

        // ====================================================
        // NOMBRE D'ENNEMIS
        // ====================================================

        std::string enemyText =
            "Enemies : " +
            std::to_string(
                enemies.size());

        DrawText(
            enemyText.c_str(),
            20,
            55,
            20,
            PURPLE);

        // ====================================================
        // NIVEAU DE SPAWN
        // ====================================================

        std::string spawnText =
            "Vague : " +
            std::to_string(
                numberOfEnemies);

        DrawText(
            spawnText.c_str(),
            20,
            80,
            20,
            WHITE);

        EndDrawing();
    }

    // ========================================================
    // FERMETURE
    // ========================================================

    CloseWindow();

    return 0;
}
