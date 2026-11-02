# Controls

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` / arrow keys | Move |
| Mouse | Look (when the cursor is captured) |
| Left `Ctrl` | Sprint (11.5 m/s) |
| Left `Shift` | Sneak (3.8 m/s) |
| `Space` | Jump (also "start" on the intro screen) |
| `Enter` | Start on the intro screen |
| Left mouse button | Fire |
| Right mouse button | Grab a crate / place the carried crate |
| `H` | Hide or show the carried crate (2 s cooldown) |
| `R` | Restart after victory/defeat (also on the intro screen) |
| `Escape` | Release the mouse cursor |
| `Tab` | Capture the mouse cursor again |
| `F1` (debug builds) | Toggle the debug menu and cursor |

The game captures the cursor at startup, so the mouse looks around immediately. Use `Escape` to get the cursor back for the window manager, `Tab` to return to play.

Movement only runs while the cursor is captured. When the debug menu is open the cursor is free; close the menu (`F1`) to resume play.

## Debug-only movement

With "Noclip" enabled in the F1 Player tab, movement ignores collision and gravity, and `E`/`Space` move up while `Q`/`Shift` move down.
