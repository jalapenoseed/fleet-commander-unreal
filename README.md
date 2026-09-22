# Fleet Commander — Unreal Engine 5.8

Native C++ port of **Fleet Commander Unity 1.3.0** (`jalapenoseed/fleet-commander` branch `unity`, commit `ccac89c`).

Clone this repo (or unzip the project folder) and open **`FleetCommander.uproject`** in **Unreal Engine 5.8**. The swarm, arena, cameras and HUD spawn from C++ — no `.uasset` pack is required.

Source: [Unity 1.3 feature guide](https://github.com/jalapenoseed/fleet-commander/blob/unity/unity/RELEASE-1.3.md).

## Open and play

This is a **C++** project. Unreal cannot open it until `FleetCommander` is compiled once — that is the “module could not be found” dialog.

1. Install **Unreal Engine 5.8** (same association as GRIDRUNNER Unreal) and **Visual Studio 2022** with *Game development with C++*.
2. Clone this repo, or unzip so you can see `FleetCommander.uproject` in the folder.
3. Double-click **`Compile-FleetCommander.cmd`**. It builds `FleetCommanderEditor` (Win64 Development) with the same `Build.bat` GRIDRUNNER uses, then opens the editor.
4. Confirm **World Settings → GameMode Override** is `FCGameMode`, then press **Play**. A 96-aircraft night stadium show launches automatically.

If you already double-clicked the `.uproject` and got *The game module 'FleetCommander' could not be found*, close that dialog and run `Compile-FleetCommander.cmd` instead. The splash screen is the editor; the missing file is `Binaries/Win64/UnrealEditor-FleetCommander.dll`, which only appears after a successful compile.

If the engine asks to switch version, pick **5.8**. Do not convert to an older engine.

Low-end (GTX 1050 Ti): in `Config/DefaultEngine.ini` under `[SystemSettings]`, use the commented 75% / 1800 MB / Lumen-off block.

## What was ported

| Unity 1.3 | Unreal |
| --- | --- |
| Show fleet 0–10,000 | 0–2,000 (default 96). Everyday target 64–256 on a 1050 Ti |
| Scout / Relay / Cargo / Utility profiles, 6 skins, 10 weapons | Same catalog numbers, instanced cube/cylinder/sphere airframes |
| 14 formations + orbit/wave/pulse/dance + 4 influence layers + Boids | `FCFormation.cpp` / `FCWorld.cpp` |
| Earth / Moon / Mars gravity, arcade lift, battery reserve/return | Same rules, Unreal Z-up centimeters |
| Arena 2–256, cyan vs orange, 16 AI styles, 20 arena formations | `G` starts 12v12; styles and slots ported |
| Join Blue / Join Red / Pilot Selected, FPV / shoulder / mounted | `1` / `2` / `F`, camera cycle `C` |
| Pulse / Rapid / Scatter / Shockwave + magazines, heat, reload, guard/dodge/boost | Arcade trace weapons, `R` / RMB / `E` / Shift |
| Director cameras, hide HUD, pause, launch/recall | Orbit/top/front/cinematic/action/survivor plus `H` / `P` / `L` / `K` |
| Destruction, wrecks, series wins | Falling wrecks, kill credit, BLUE/RED/DRAW banner |

Not in this first Unreal drop (still Unity-only): sports, chess, drone range, Night Brite, Art Studio pixel canvas, Nerd/Logic labs, replay JSON, imported GRIDRUNNER GLB/PBR packs, Niagara-authored weather. Those can be added on this project without changing the sim.

## Controls

Spectator (show / arena overview)

| Input | Action |
| --- | --- |
| Drag right mouse | Orbit |
| Wheel | Zoom |
| WASD | Pan the show origin |
| L / K | Launch / recall |
| [ / ] | Previous / next formation |
| M / N | Motion pattern / Boids |
| Tab / X | Next / previous drone |
| Left click | Select nearest aircraft |
| C | Cycle camera |
| G | Start 12v12 arena |
| 1 / 2 | Join Blue / Join Red |
| F | Pilot selected |
| T | Return to show fleet |
| H | Hide HUD |
| P | Pause |

Pilot (after 1 / 2 / F)

| Input | Action |
| --- | --- |
| WASD | Forward / back / strafe (A left, D right) |
| Space / Ctrl | Ascend / descend |
| Shift | Boost |
| Mouse (hold RMB) | Aim |
| Left mouse | Fire |
| R | Reload |
| RMB | Guard / parry |
| E | Dodge |
| Q or B | Secondary pulse |
| C | Shoulder / FPV / mounted / follow |
| Esc | Return aircraft to AI |

## Project layout

```
FleetCommander.uproject
Config/                  engine, input, packaging
Content/Python/          sets FCGameMode on editor boot
Source/FleetCommander/
  FCTypes.h              Unity enums/structs
  FCCatalog.*            frame / weapon / skin tables
  FCFormation.*          formation + influence math (Z-up cm)
  FCWorld.*              60 Hz sim, combat, instanced draw
  FCGameMode.*           lights, stadium, spawn
  FCPawn.*               camera pawn
  FCPlayerController.*   Unity key map
  FCHUD.*                command HUD
```

Units: **1 Unity meter = 100 Unreal cm**. Simulation is left-handed Z-up.

## Upload / move this project

Copy the whole project folder (the directory that contains `FleetCommander.uproject`) onto the Windows machine that has UE 5.8. Do not copy `Binaries/`, `Intermediate/`, or `Saved/` if they appear after the first compile.

Git: https://github.com/jalapenoseed/fleet-commander-unreal
