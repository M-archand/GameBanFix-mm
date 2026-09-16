# GameBanFix

A Metamod:Source plugin for CS2 that stops one game-banned player from locking everyone else out of the server.

Fix taken from [CS2Fixes](https://github.com/Source2ZE/CS2Fixes/commit/d67f069374e349efb95b9fb5a360172fa08550c8).

## The problem

With `sv_kick_players_with_cooldown` enabled, the server asks Steam about each connecting player and caches the answer in an internal ban map. When a player with a game ban (a cooldown) connects, they are kicked correctly, but their entry stays in that map. Every later connection is then checked against the stale entry, so unbanned players are rejected too and the console fills with the same error message. The server keeps refusing joins until it is restarted.

## What the plugin does

It detours the server's `CheckSteamBan` think function and clears the ban map after each call, so a kicked player's entry never carries over to the next person who connects. Nothing else is touched: no convars, no commands, no chat output, no gameplay changes.

There is nothing to configure. Install it and it works.

## Requirements

- CS2 dedicated server
- [Metamod:Source](https://cs2.poggu.me/metamod/installation/) build **1461 or newer**

> [!IMPORTANT]
> Builds 2.0.0 and later are KHook-based (plugin API 18) and will not load on older Metamod builds. If you are on an older Metamod, either update it or stay on GameBanFix v1.0.7.

## Installation

1. Install Metamod:Source.
2. Download the [latest release](https://github.com/M-archand/GameBanFix-mm/releases/latest) for your platform (`-linux.zip` or `-windows.zip`).
3. Extract the archive over `game/csgo` on your server.
4. Restart the server, then confirm it loaded with `meta list`.

The package installs three files:

```
addons/metamod/gamebanfix.vdf
addons/gamebanfix/bin/linuxsteamrt64/gamebanfix.so   (win64/gamebanfix.dll on Windows)
addons/gamebanfix/gamedata/gamebanfix.jsonc
```

> [!CAUTION]
> Do not run this alongside CS2Fixes or any other plugin that detours `CheckSteamBan`. Two detours on the same function will fight over the same bytes.

## When a game update breaks it

The plugin finds its two targets by scanning the server binary for byte patterns, because neither is exported. A CS2 update that changes that code invalidates the patterns, and the plugin then refuses to load with an error naming the entry that failed:

```
[GameBanFix] Failed to find address for GameSystem_Think_CheckSteamBan
```

This is deliberate, it fails to load rather than detouring the wrong function. When it happens, update `addons/gamebanfix/gamedata/gamebanfix.jsonc` with fresh signatures, or wait for a release that ships them. The gamedata file is plain JSONC, so a signature fix needs no new binary; editing that file and restarting is enough.

`meta unload` is safe at any time, the detour is uninstalled and the mapped modules released.

## Building from source

Requires [AMBuild](https://wiki.alliedmods.net/Ambuild), a C++20 compiler (clang on Linux, MSVC on Windows), and checkouts of [Metamod:Source](https://github.com/alliedmodders/metamod-source) and the [CS2 HL2SDK](https://github.com/alliedmodders/hl2sdk/tree/cs2) beside this repository.

```sh
mkdir build && cd build
python ../configure.py --enable-optimize --symbol-files --sdks cs2
ambuild
```

The finished package lands in `build/package`. On Linux, install `lld` as well, the build uses it to keep `STB_GNU_UNIQUE` symbols out of the binary so Metamod can unload the plugin cleanly.

## Credits

Original fix by aiolos1045 and Vauff (Source2ZE), standalone plugin by [Cruze](https://github.com/Cruze03/GameBanFix). Licensed under [GPLv3](LICENSE).
