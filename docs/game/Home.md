# Game

RivalGrounds is a single-player first-person shooter built on RGEngine in `engine/`. The game lives in `game/`:

```text
game/
├── include/rivalgrounds/
│   ├── RivalGrounds.h     Scene subclass: world, player, combat, HUD, debug menu
│   └── Enemy.h            enemy state and behaviour
├── src/
│   ├── main.cpp           window config, post-process, frame loop
│   ├── Enemy.cpp          enemy AI and body
│   ├── RivalGrounds.cpp   match setup, city, enemy spawning and culling
│   ├── PlayerController.cpp  per-frame update, movement, enemy shots
│   ├── Combat.cpp         shooting, crate grab/throw/hide
│   ├── Hud.cpp            title/victory/defeat screens, HUD, minimap
│   └── RivalGroundsDebug.cpp  F1 debug menu (debug builds only)
└── shaders/
    ├── cube.vert/frag     the game's material shader
    └── grayscale.frag     the game's post-process
```

The goal of a match is to find the gold scroll in a large city while enemy soldiers hunt the player. The game has one hitscan weapon (an M4 Colt viewmodel), fourteen patrolling enemies plus reinforcements, a HUD with a minimap, and a debug menu.

## Pages

- [Overview](Overview.md) — what the game is
- [Architecture](Architecture.md) — how the game layer is organised
- [Game loop](Game-Loop.md) — the exact per-frame order
- [Player](Player.md) — controller, health, grabbing
- [Weapons](Weapons.md) — the rifle, viewmodel, recoil
- [Enemies](Enemies.md) and [enemy AI](Enemy-AI.md)
- [Combat and damage](Combat-and-Damage.md)
- [World and city](World-and-City.md)
- [Collision system](Collision-System.md)
- [Objectives and game states](Objectives-and-Game-States.md)
- [Rendering and post-processing](Rendering-and-Post-Processing.md)
- [HUD and interface](HUD-and-Interface.md)
- [Debug tools](Debug-Tools.md)
