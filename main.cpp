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

// nombres de fps
const int targetFps = 60;

// autres variables
int numberOfEnemies = 1;
long long score = 0;
int maxNumberOfEnemies = 15;

// variables sliding enemies

int numberOfSlidingEnemies = 0;
int maxNumberOfSlidingEnemies = 10;

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

    int bulets;
    int maxBulets;

    float restitution;
    float health;
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

        bulets = 10;
        maxBulets = 10;
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

            health = health - 1;
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

        // Plafond
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
        // Joueur
        float playerLeft = x;
        float playerRight = x + width;

        float playerTop = y;
        float playerBottom = y + height;

        // Mur
        float wallLeft = Wall.x;
        float wallRight =
            Wall.x + Wall.height;

        float wallTop = Wall.y;
        float wallBottom =
            Wall.y + Wall.width;

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

            velocityY =
                velocityY / 2;

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
// ENNEMI NORMAL
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

        color = enemyColor;

        gravity = 500.0f;

        velocityY = 0.0f;

        restitution = 0.8f;

        colider = true;
    }

    void draw()
    {
        DrawCircle(
            x,
            y,
            radius,
            color);
    }

    void update()
    {
        // Déplacement horizontal
        x = x - 1;
    }
};

// ============================================================
// SPINNING OBJECT
// ============================================================

class slidingEnemy
{
public:
    float x;
    float y;

    float h;
    float w;

    Color color;

    float velocity;
    float rotation;

    slidingEnemy(
        float slidingEnemyX,
        float slidingEnemyY,
        float slidingEnemyH,
        float slidingEnemyW,
        Color slidingEnemyColor)
    {
        x = slidingEnemyX;
        y = slidingEnemyY;

        h = slidingEnemyH;
        w = slidingEnemyW;

        color = slidingEnemyColor;

        velocity = 5.0f;

        rotation = 0.0f;
    }

    void update(float dt)
    {
        // Même vitesse de déplacement
        // que ton objet d'origine
        x -= velocity;

        // Rotation
        rotation += 1.0f;

        if (rotation >= 360.0f)
        {
            rotation -= 360.0f;
        }
    }

    void draw()
    {
        DrawRectanglePro(
            {x,
             y,
             w,
             h},
            {w / 2,
             h / 2},
            rotation,
            color);
    }
};

// ============================================================
// PROJECTION POUR COLLISION SAT
// ============================================================

void projectPoints(
    const std::vector<Vector2> &points,
    Vector2 axis,
    float &min,
    float &max)
{
    min =
        points[0].x * axis.x +
        points[0].y * axis.y;

    max = min;

    for (size_t i = 1;
         i < points.size();
         i++)
    {
        float projection =
            points[i].x * axis.x +
            points[i].y * axis.y;

        if (projection < min)
        {
            min = projection;
        }

        if (projection > max)
        {
            max = projection;
        }
    }
}

// ============================================================
// COLLISION DES PROJECTIONS
// ============================================================

bool projectionsOverlap(
    float minA,
    float maxA,
    float minB,
    float maxB)
{
    return maxA >= minB &&
           maxB >= minA;
}

// ============================================================
// COLLISION JOUEUR / SPINNING OBJECT
// ============================================================

bool playerVsSlidingEnemy(
    player &Player,
    slidingEnemy &enemy)
{
    // ========================================================
    // RECTANGLE DU JOUEUR
    // ========================================================

    std::vector<Vector2> playerPoints;

    playerPoints.push_back(
        {Player.x,
         Player.y});

    playerPoints.push_back(
        {Player.x + Player.width,
         Player.y});

    playerPoints.push_back(
        {Player.x + Player.width,
         Player.y + Player.height});

    playerPoints.push_back(
        {Player.x,
         Player.y + Player.height});

    // ========================================================
    // ANGLE DE L'ENNEMI
    // ========================================================

    float angle =
        enemy.rotation *
        PI /
        180.0f;

    float cosAngle =
        cos(angle);

    float sinAngle =
        sin(angle);

    // ========================================================
    // DEMI-TAILLES
    // ========================================================

    float halfW =
        enemy.w / 2;

    float halfH =
        enemy.h / 2;

    // ========================================================
    // POINT LOCAL -> MONDE
    // ========================================================

    auto rotatePoint =
        [&](float localX, float localY)
    {
        Vector2 point;

        point.x =
            enemy.x +
            localX * cosAngle -
            localY * sinAngle;

        point.y =
            enemy.y +
            localX * sinAngle +
            localY * cosAngle;

        return point;
    };

    // ========================================================
    // RECTANGLE TOURNÉ
    // ========================================================

    std::vector<Vector2> enemyPoints;

    enemyPoints.push_back(
        rotatePoint(
            -halfW,
            -halfH));

    enemyPoints.push_back(
        rotatePoint(
            halfW,
            -halfH));

    enemyPoints.push_back(
        rotatePoint(
            halfW,
            halfH));

    enemyPoints.push_back(
        rotatePoint(
            -halfW,
            halfH));

    // ========================================================
    // AXES DU SAT
    // ========================================================

    Vector2 axes[4];

    // Axes du joueur
    axes[0] =
        {
            1,
            0};

    axes[1] =
        {
            0,
            1};

    // Axes du spinning object
    axes[2] =
        {
            cosAngle,
            sinAngle};

    axes[3] =
        {
            -sinAngle,
            cosAngle};

    // ========================================================
    // TEST DES 4 AXES
    // ========================================================

    for (int i = 0; i < 4; i++)
    {
        float playerMin;
        float playerMax;

        float enemyMin;
        float enemyMax;

        projectPoints(
            playerPoints,
            axes[i],
            playerMin,
            playerMax);

        projectPoints(
            enemyPoints,
            axes[i],
            enemyMin,
            enemyMax);

        if (!projectionsOverlap(
                playerMin,
                playerMax,
                enemyMin,
                enemyMax))
        {
            return false;
        }
    }

    return true;
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    // ========================================================
    // FENÊTRE
    // ========================================================

    SetConfigFlags(
        FLAG_WINDOW_UNDECORATED);

    int monitorWidth =
        GetMonitorWidth(0);

    int monitorHeight =
        GetMonitorHeight(0);

    InitWindow(
        monitorWidth,
        monitorHeight,
        "Physics Simulator");

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
        RED);

    // ========================================================
    // MURS
    // ========================================================

    std::vector<wall> walls;

    // Mur horizontal vert
    walls.emplace_back(
        screenX * 0.26f,
        screenY * 0.60f,
        screenX * 0.13f,
        30,
        GREEN);

    // Mur horizontal rouge
    walls.emplace_back(
        screenX * 0.46f,
        screenY * 0.40f,
        screenX * 0.13f,
        30,
        RED);

    // Mur vertical orange
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

    // Spinning objects
    std::vector<slidingEnemy> SlidingEnemies;

    // ========================================================
    // CHRONOMETRES
    // ========================================================

    auto lastEnemySpawn =
        std::chrono::steady_clock::now();

    auto lastBulletRecharge =
        std::chrono::steady_clock::now();

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
            ToggleFullscreen();

            screenX =
                GetScreenWidth();

            screenY =
                GetScreenHeight();
        }

        // ====================================================
        // TEMPS
        // ====================================================

        auto now =
            std::chrono::steady_clock::now();

        // ====================================================
        // SPAWN ENNEMIS TOUTES LES 5 SECONDES
        // ====================================================

        auto enemyElapsed =
            std::chrono::duration_cast<
                std::chrono::seconds>(
                now - lastEnemySpawn);

        if (enemyElapsed.count() >= 5)
        {
            std::cout
                << "spawn d un enemi"
                << std::endl;

            int i = 0;

            int n =
                1 +
                std::rand() %
                    numberOfEnemies;
            if (numberOfEnemies <= maxNumberOfEnemies)
            {
                numberOfEnemies = numberOfEnemies + 1;
            }
            while (i != n)
            {
                enemies.emplace_back(
                    std::rand() % screenY,
                    screenX +
                        std::rand() %
                            (screenX / 20),
                    5,
                    PURPLE);

                score =
                    score +
                    numberOfEnemies;

                i++;
            }

            // =================================================
            // SPINNING OBJECT
            // =================================================

            // Même taille : 25 x 25
            // Même couleur : RED

            if (numberOfEnemies == maxNumberOfEnemies)
            {
                if (numberOfEnemies % 2 == 0)
                {

                    // on spawn a la base 1 ennemis sur le y du joueur
                    SlidingEnemies.emplace_back(
                        screenX + 25,
                        Player.y +
                            Player.height / 2,
                        25,
                        25,
                        RED);

                    i = 0;
                    while (i < numberOfSlidingEnemies)
                    {

                        SlidingEnemies.emplace_back(
                            screenX + 25,
                            rand() % screenY +
                                Player.height / 2,
                            25,
                            25,
                            RED);
                        i = i + 1;
                    };
                    if (numberOfSlidingEnemies != maxNumberOfSlidingEnemies)
                    {
                        numberOfSlidingEnemies = numberOfSlidingEnemies + 1;
                    };
                };
            }

            lastEnemySpawn =
                now;
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

            lastBulletRecharge =
                now;
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
            enemy.update();
        }

        // ====================================================
        // UPDATE SPINNING OBJECTS
        // ====================================================

        for (
            slidingEnemy &enemy :
            SlidingEnemies)
        {
            enemy.update(dt);
        }

        // ====================================================
        // SUPPRESSION DES PARTICULES MORTES
        // ====================================================

        for (
            int i =
                particles.size() - 1;
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
        // SUPPRESSION DES ENNEMIS SORTIS
        // ====================================================

        for (
            int i =
                enemies.size() - 1;
            i >= 0;
            i--)
        {
            if (enemies[i].x < -100)
            {
                enemies.erase(
                    enemies.begin() + i);
            }
        }

        // ====================================================
        // SUPPRESSION DES SPINNING OBJECTS
        // ====================================================

        for (
            int i =
                SlidingEnemies.size() - 1;
            i >= 0;
            i--)
        {
            if (
                SlidingEnemies[i].x <
                -100)
            {
                SlidingEnemies.erase(
                    SlidingEnemies.begin() + i);
            }
        }

        // ====================================================
        // COLLISION JOUEUR / ENNEMIS NORMAUX
        // ====================================================

        for (enemyA &enemy : enemies)
        {
            if (
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
                        enemy.radius)
            {
                std::cout
                    << "COLLISION !"
                    << std::endl;

                Player.health =
                    Player.health - 10;
            }
        }

        // ====================================================
        // COLLISION JOUEUR / SPINNING OBJECT
        // ====================================================

        for (
            slidingEnemy &enemy :
            SlidingEnemies)
        {
            if (
                playerVsSlidingEnemy(
                    Player,
                    enemy))
            {
                std::cout
                    << "COLLISION SPINNING OBJECT !"
                    << std::endl;

                // 30 dégâts
                Player.health =
                    Player.health - 30;

                // On repousse légèrement
                // le joueur pour éviter
                // plusieurs collisions
                Player.velocityX = -250;
            }
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
        // SPINNING OBJECTS
        // ====================================================

        for (
            slidingEnemy &enemy :
            SlidingEnemies)
        {
            enemy.draw();
        }

        // ====================================================
        // SCORE
        // ====================================================

        std::string scoreMessage =
            "ton score " +
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
            std::to_string(
                (int)Player.bulets) +
            " / " +
            std::to_string(
                (int)Player.maxBulets);

        DrawText(
            buletsText.c_str(),
            screenX - 100,
            screenY - 40,
            25,
            GREEN);

        EndDrawing();

        // ====================================================
        // GAME OVER
        // ====================================================

        if (Player.health <= 0)
        {
            std::cout
                << "GAME OVER"
                << std::endl;

            break;
        }
    }

    // ========================================================
    // FERMETURE
    // ========================================================

    CloseWindow();

    return 0;
}
