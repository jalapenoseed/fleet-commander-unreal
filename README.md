# Fleet Commander — Unreal Engine 5.8

Native C++ port of **Fleet Commander Unity 1.3.0**.

- Source game: [jalapenoseed/fleet-commander](https://github.com/jalapenoseed/fleet-commander/tree/unity) branch **`unity`**, commit `ccac89c`
- Unity project: `unity/` opened in **6000.6.2f1**, scene `Assets/FleetCommander/Scenes/FleetCommander.unity`
- This repo: C++ Unreal 5.8 project that loads those **same aircraft and GRIDRUNNER scenery files**

This is **not** the Three.js `main` branch and **not** a cube stand-in. `FleetAssets/` is the decompressed Unity 1.3 pack:

| Unity | Unreal |
| --- | --- |
| `*.fleetmesh` (gzip FCM1) | `FleetAssets/DroneModels/*_LOD.fcm1` Scout / Relay / Cargo / Utility |
| `*.fleetprop` (gzip FCP1) | `FleetAssets/ScenePacks/*.fcp1` houses, field camp, field ops |
| PBR / albedo PNGs | `FleetAssets/DroneTextures`, `FleetAssets/PropTextures` |

Open **`FleetCommander.uproject`** in **Unreal Engine 5.8**. The swarm, arena, cameras and HUD spawn from C++. The meshes are read at Play from `FleetAssets/` next to the uproject.

1.3 feature guide: [unity/RELEASE-1.3.md](https://github.com/jalapenoseed/fleet-commander/blob/unity/unity/RELEASE-1.3.md).

## Open and play

This is a **C++** project. Unreal cannot open it until `FleetCommander` is compiled once — that is the “module could not be found” dialog.

Do **not** use GitHub **Download ZIP**. Windows Defender flags the zip because of `Compile-FleetCommander.cmd`. Clone instead:

```
git clone https://github.com/jalapenoseed/fleet-commander-unreal.git
```

If you already cloned, `git pull` in that folder. Throw away the old Downloads unzip (that was the cube stand-in).

1. Install **Unreal Engine 5.8** and **Visual Studio 2022** with *Game development with C++*.
2. Clone so you can see `FleetCommander.uproject` in the folder, with `FleetAssets/` beside it.
3. Double-click **`Compile-FleetCommander.cmd`**. It builds `FleetCommanderEditor` (Win64 Development) then opens the editor.
4. Confirm **World Settings → GameMode Override** is `FCGameMode`, then press **Play**. A 96-aircraft night show launches with the Unity Scout / Relay / Cargo / Utility airframes and GRIDRUNNER camp + houses.

If the engine asks to switch version, pick **5.8**.

### “There is not enough space on the disk”

That is **not a code error**. UE 5.8 writes a multi-GB editor PCH plus a default **40 GB** Unreal Build Accelerator cache. The C: drive filled up.

Free ~**20 GB** on C:, then compile again:

1. Delete `Intermediate` and `Saved` inside this repo (failed compile leftovers).
2. Delete `C:\ProgramData\Epic\UnrealBuildAccelerator` (the 40 GB UBA cache).
3. Delete the old Downloads unzip `fleet-commander-unreal-main` if it is still there.
4. Empty Recycle Bin.

`git pull`, then run `Compile-FleetCommander.cmd` again. The script now refuses to start if C: has under 20 GB free, and it turns UBA off so it cannot reserve 40 GB.

If you cannot run `.cmd` files, open **x64 Native Tools Command Prompt for VS 2022** and run:

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" FleetCommanderEditor Win64 Development "%CD%\FleetCommander.uproject" -WaitMutex -NoHotReload -NoUBA -Executor=Parallel -UBAStoreCapacityGb=2 -MaxParallelActions=3
```

Then open the uproject.

Low-end (GTX 1050 Ti): in `Config/DefaultEngine.ini` under `[SystemSettings]`, use the commented 75% / 1800 MB / Lumen-off block.

## What was ported from Unity 1.3

| Unity 1.3 | Unreal |
| --- | --- |
| Imported Scout / Relay / Cargo / Utility FCM1 airframes + LOD | `FCAssetLoader` builds `UStaticMesh` at Play, instanced per frame |
| GRIDRUNNER houses, field camp, field ops FCP1 | Spawned from Unity `ScenePackPlacement` positions |
| Show fleet 0–10,000 | 0–2,000 (default 96). Everyday target 64–256 on a 1050 Ti |
| 6 skins, 10 weapons, frame speed/armor/mass | Same catalog numbers |
| 14 formations + orbit/wave/pulse/dance + Boids | `FCFormation.cpp` / `FCWorld.cpp` |
| Earth / Moon / Mars gravity, arcade lift, battery reserve/return | Same rules, Unreal Z-up centimeters |
| Arena 2–256, cyan vs orange, 16 AI styles | `G` starts 12v12 |
| Join Blue / Join Red / Pilot Selected | `1` / `2` / `F`, camera cycle `C` |
| Pulse / Rapid / Scatter / Shockwave + magazines, heat, reload, guard/dodge/boost | Arcade trace weapons |
| Director cameras, hide HUD, pause, launch/recall | Orbit/top/front/cinematic/action/survivor plus `H` / `P` / `L` / `K` |

Not in this Unreal drop (still Unity-only): sports, chess, command-center UI Toolkit, Night Brite, Art Studio pixel canvas, Nerd/Logic labs, replay JSON, Niagara weather, full-res (non-LOD) drone meshes, rotor animation on the imported meshes. Those can be added on this project without changing the sim.

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
FleetAssets/             Unity 1.3 FCM1 / FCP1 / PNG  (keep beside the uproject)
Config/
Content/Python/          sets FCGameMode on editor boot
Source/FleetCommander/
  FCAssetLoader.*        FCM1 / FCP1 runtime importer (Unity Y-up m → UE Z-up cm)
  FCTypes.h              Unity enums/structs
  FCCatalog.*            frame / weapon / skin tables
  FCFormation.*          formation + influence math
  FCWorld.*              60 Hz sim, combat, instanced Unity airframes
  FCGameMode.*           lights, stadium, GRIDRUNNER scenery
  FCPawn.*               camera pawn
  FCPlayerController.*   Unity key map
  FCHUD.*                command HUD
```

Units: **1 Unity meter = 100 Unreal cm**. `UE(X,Y,Z) = (Unity.z, Unity.x, Unity.y) * 100`. Triangle winding is flipped.

## Upload / move this project

Copy the whole project folder (the directory that contains `FleetCommander.uproject` **and** `FleetAssets/`) onto the Windows machine that has UE 5.8. Do not copy `Binaries/`, `Intermediate/`, or `Saved/` if they appear after the first compile.

Git: https://github.com/jalapenoseed/fleet-commander-unreal
