# TuffMenu

A dark + blue hack menu for Geometry Dash, built with [Geode](https://geode-sdk.org).

## Features

| Category | What's in it |
|---|---|
| **Level** | Speedhack (type any speed; the music slows/speeds up with the game), Auto Clicker (1-100 CPS), Hitboxes |
| **Player** | Noclip (gear icon next to the switch opens options: death counter on/off + reset), Jump Hack (cube) |
| **Cosmetics** | No Death Effect, No Spider Dash, No Wave Trail, No Trail, No Camera Shake, Hide Attempts |
| **Labels** | FPS, CPS, TPS, Run From, Attempt, Percent, Clock (all off by default) |
| **Bypass** | Unlock all icons (local only) |
| **Interface** | Change the UI color with RGB values or presets, then press APPLY |

Open the menu with the **round "T" button**. It floats over every screen and you can **drag it anywhere**
(mouse on desktop, finger on mobile). Its position is saved.

## Building

1. Install the [Geode SDK + CLI](https://docs.geode-sdk.org/getting-started/) and set `GEODE_SDK`.
2. Open `mod.json` and set `"geode"` to the exact SDK version you have installed (`geode sdk version`).
3. Build:

```
geode build
```

Android/iOS/macOS builds: push this folder to a GitHub repo, the included workflow
(`.github/workflows/build.yml`) builds every platform and gives you the `.geode` file.

## Notes

- This was written against the Geode v5 API and GD 2.2081 bindings. I couldn't compile it
  in my environment, so expect to fix a signature or two if the bindings differ.
- Speedhack scales the whole game scheduler and the master audio pitch (clamped to 0.1x-4x for audio).
- Jump Hack only affects the cube gamemode.
- Auto Clicker is driven by physics steps, so high CPS values stay accurate.
- Unlock-icons only changes what your own client displays; it does not change anything on servers.
