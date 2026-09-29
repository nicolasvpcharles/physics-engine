2D Physics Simulator
Petit simulateur de physique 2D développé en C++ avec Raylib.
Le projet permet de simuler différents objets soumis à une physique simple : gravité, vitesse, collisions et rebonds.
Le code est actuellement regroupé dans `main.cpp`.
---
Prérequis
Pour compiler le projet, il faut :
C++
g++
Raylib
MSYS2 sous Windows
VS Code (optionnel)
Le projet utilise actuellement les bibliothèques Raylib présentes dans :
```text
C:/msys64/ucrt64/include
C:/msys64/ucrt64/lib
```
Raylib
Raylib doit être installé dans l'environnement UCRT64 de MSYS2.
Le compilateur doit pouvoir trouver :
```text
raylib.h
```
ainsi que la bibliothèque Raylib.
---
Récupérer le projet
Pour cloner le repository :
```bash
git clone https://github.com/USERNAME/REPOSITORY.git
```
Puis entrer dans le dossier :
```bash
cd REPOSITORY
```
Pour faire un fork, utilisez le bouton Fork de GitHub puis clonez votre fork.
---
Compilation
Sous Windows avec MSYS2 UCRT64 :
```bash
g++ main.cpp -o main.exe -IC:/msys64/ucrt64/include -LC:/msys64/ucrt64/lib -lraylib -lopengl32 -lgdi32 -lwinmm
```
Cela produit :
```text
main.exe
```
Le programme peut ensuite être lancé avec :
```bash
./main.exe
```
ou :
```bash
main.exe
```
---
Structure générale
Le projet est actuellement organisé autour de plusieurs classes et fonctions :
```text
main.cpp
│
├── calculateAngle()
│
├── Particle
│
├── wall
│
├── player
│
├── Ball
│
├── shootParticles()
│
└── main()
```
Chaque partie possède un rôle différent dans le fonctionnement du simulateur.
---
Fonctionnement général
Le programme fonctionne avec une game loop.
Elle se trouve dans :
```cpp
while (!WindowShouldClose())
```
À chaque tour de boucle, le programme :
récupère le temps écoulé ;
récupère les entrées utilisateur ;
met à jour les objets ;
vérifie les collisions ;
supprime les particules terminées ;
dessine les objets à l'écran.
La structure générale est donc :
```text
Game Loop
    │
    ├── GetFrameTime()
    │
    ├── Inputs
    │
    ├── Update
    │
    ├── Collisions
    │
    └── Draw
```
---
Delta Time
Le programme utilise :
```cpp
float dt = GetFrameTime();
```
`dt` représente le temps écoulé depuis la dernière image.
Il est utilisé pour les déplacements et la physique :
```cpp
x += velocityX * dt;
y += velocityY * dt;
```
Cela permet de calculer les déplacements en fonction du temps.
---
`calculateAngle()`
La fonction `calculateAngle()` calcule une direction entre deux points.
Elle calcule `dx` et `dy`, puis la distance entre les deux points :
```cpp
float distance = std::sqrt(dx * dx + dy * dy);
```
Le vecteur est ensuite normalisé :
```cpp
dx /= distance;
dy /= distance;
```
La fonction retourne donc un vecteur représentant une direction.
Elle est notamment utilisée pour calculer la direction de propulsion du joueur.
---
Classe `player`
La classe `player` représente le joueur.
Elle contient notamment :
```cpp
float x;
float y;

float height;
float width;

float velocityX;
float velocityY;

float gravity;
float restitution;

Color color;
```
La position est représentée par `x` et `y`.
La vitesse est représentée par `velocityX` et `velocityY`.
---
Mise à jour du joueur
La méthode :
```cpp
Player.update(dt);
```
met à jour la physique du joueur.
La gravité est appliquée avec :
```cpp
velocityY += gravity * dt;
```
Puis la position est modifiée :
```cpp
x += velocityX * dt;
y += velocityY * dt;
```
Le joueur est également empêché de sortir de la fenêtre.
---
Propulsion du joueur
Lorsque le bouton gauche de la souris est pressé :
```cpp
IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
```
le programme appelle :
```cpp
Player.pushBack(mouse.x, mouse.y);
```
La direction entre le joueur et la souris est calculée.
Une force est ensuite appliquée :
```cpp
float pushForce = 500.0f;
```
La vitesse du joueur est modifiée en fonction de cette direction.
---
Arme
L'arme du joueur est dessinée dans :
```cpp
drawWeapon()
```
La position de la souris est récupérée avec :
```cpp
Vector2 mouse = GetMousePosition();
```
Le programme utilise ensuite `atan2()` pour calculer l'angle entre le joueur et la souris.
L'arme est dessinée avec :
```cpp
DrawRectanglePro()
```
Elle pointe donc vers la souris.
---
Collisions
Les collisions entre le joueur et les murs utilisent des rectangles AABB (Axis-Aligned Bounding Box).
Le programme calcule les limites du joueur :
```cpp
float playerLeft = x;
float playerRight = x + width;

float playerTop = y;
float playerBottom = y + height;
```
Puis celles du mur.
Une collision est détectée lorsque les deux rectangles se chevauchent.
Le programme utilise également `previousX` et `previousY` pour déterminer de quel côté le joueur est arrivé sur le mur.
Cela permet de gérer les collisions par :
le dessus ;
le dessous ;
la gauche ;
la droite.
---
Classe `wall`
La classe `wall` représente les murs présents dans le monde.
Un mur possède une position, une taille et une couleur.
Exemple :
```cpp
walls.emplace_back(
    100,
    100,
    30,
    300,
    ORANGE);
```
Les murs sont stockés dans :
```cpp
std::vector<wall> walls;
```
---
Classe `Ball`
La classe `Ball` représente une balle physique.
Elle possède notamment :
```cpp
float x;
float y;

float radius;

float gravity;
float velocityY;

float restitution;

Color color;
```
La balle est affectée par la gravité :
```cpp
velocityY += gravity * dt;
```
Puis elle se déplace :
```cpp
y += velocityY * dt;
```
---
Rebond
Lorsque la balle touche le bas de l'écran, sa vitesse est inversée :
```cpp
velocityY = -velocityY * restitution;
```
La variable `restitution` contrôle la quantité de vitesse conservée lors du rebond.
---
Classe `Particle`
La classe `Particle` représente une particule.
Elle possède notamment :
```cpp
float x;
float y;

float velocityX;
float velocityY;

float lifetime;
float maxLifetime;

float radius;

Color color;
```
Les particules sont utilisées pour créer un effet lors de la propulsion du joueur.
---
Création des particules
Les particules sont créées dans :
```cpp
shootParticles()
```
La fonction crée plusieurs particules et leur donne une vitesse avec une petite variation aléatoire.
Les particules apparaissent au niveau de l'extrémité de l'arme.
---
Durée de vie des particules
Chaque particule possède une durée de vie.
À chaque frame :
```cpp
lifetime -= dt;
```
Lorsqu'une particule arrive à :
```cpp
lifetime <= 0
```
elle est supprimée du `std::vector`.
---
Rendu graphique
Le rendu est effectué entre :
```cpp
BeginDrawing();
```
et :
```cpp
EndDrawing();
```
Le fond est nettoyé avec :
```cpp
ClearBackground(BLACK);
```
Puis les différents objets sont dessinés :
```cpp
Player.draw();

for (Ball &ball : balls)
{
    ball.draw();
}

for (wall &Wall : walls)
{
    Wall.draw();
}

for (Particle &particle : particles)
{
    particle.draw();
}
```
Chaque classe possède sa propre méthode `draw()`.
---
Ajouter un objet
Le projet utilise `std::vector` pour stocker plusieurs objets.
Pour ajouter une balle :
```cpp
std::vector<Ball> balls;

balls.emplace_back(
    500,
    125,
    20,
    RED);
```
Pour ajouter un mur :
```cpp
std::vector<wall> walls;

walls.emplace_back(
    400,
    600,
    200,
    30,
    GREEN);
```
---
Modifier les paramètres
Les paramètres physiques sont directement présents dans les classes.
Par exemple, la gravité du joueur :
```cpp
gravity = 500.0f;
```
La force de propulsion :
```cpp
float pushForce = 500.0f;
```
La restitution :
```cpp
restitution = 0.8f;
```
Ces valeurs peuvent être modifiées pour expérimenter avec le comportement du simulateur.
---
Modifier le projet
Après avoir cloné ou fork le projet, les modifications peuvent être faites directement dans :
```text
main.cpp
```
Après une modification, recompilez le programme :
```bash
g++ main.cpp -o main.exe -IC:/msys64/ucrt64/include -LC:/msys64/ucrt64/lib -lraylib -lopengl32 -lgdi32 -lwinmm
```
Puis lancez :
```bash
./main.exe
```
---
Git
Après avoir fork le projet, les modifications peuvent être enregistrées avec Git :
```bash
git add .
```
Puis :
```bash
git commit -m "Modification du simulateur"
```
Et enfin :
```bash
git push
```
Le fork GitHub sera alors mis à jour.
---
Licence
Aucune licence spécifique n'est actuellement définie pour ce projet.
