# Objectives and game states

## States

```cpp
enum class GameState { INTRO, PLAYING, VICTORY, DEFEATED };
```

| State | Behaviour |
| --- | --- |
| `INTRO` | Full-screen dark overlay with the `logo` texture, the control list, and "Press Space or Click to Begin". `Space`, `Enter`, or left click starts the match. |
| `PLAYING` | Normal frame: controller, shooting, grabbing, enemies, HUD. |
| `VICTORY` | "You found the scroll" overlay. `R` restarts. |
| `DEFEATED` | "You were taken down" overlay. `R` restarts. |

`R` is also accepted in `INTRO`. Restarting calls `reset_match()`, which rebuilds the world, the player position, enemy list, crates, objective, and bounds, and sets the state back to `PLAYING`.

The match starts in `INTRO`: `init()` calls `reset_match()` (which sets `PLAYING`) and then sets `INTRO`, so the world exists behind the intro overlay.

## The objective

One gold scroll is placed at a random point in the city, at least 25 m from the player, with clear sky above it. Victory is checked every playing step:

```cpp
distance2D(player, scroll) < 2.2 && abs(player.y - scroll.y) < 2.0
```

On victory the scroll object is hidden and the state becomes `VICTORY`.

## Failure

The player's health reaching 0 sets `DEFEATED` and returns from the update immediately.

## HUD cues

- While playing, the scroll's distance is shown at the top centre.
- The minimap shows the scroll (gold) and living enemies (red) within a 160 m radius.
- Enemy health bars appear over combat-state enemies within 35 m that are not occluded.
