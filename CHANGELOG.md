# Changelog

## [1.1.0](https://github.com/M-archand/GameBanFix-mm/compare/7af46ce...v1.1.0) (2026-09-15)


### Features

* **gamedata:** replace `gamebanfix.games.txt` with `gamebanfix.jsonc`, a JSONC file with comment support and IDA-style signatures (`41 54 48 81 EC ? ? ? ?`) instead of escaped byte strings (`\x41\x54\x48\x81\xEC\x2A\x2A\x2A\x2A`) ([704bd5d](https://github.com/M-archand/GameBanFix-mm/commit/704bd5d7fdea1dfa0394455bb181d9d7d0184519))


### Bug Fixes

* **lifecycle:** uninstall the detour on unload instead of leaving a trampoline behind, and report load failures through Metamod's error buffer rather than taking the server down ([2c5eff5](https://github.com/M-archand/GameBanFix-mm/commit/2c5eff54c134bd02f4a1a002c3ef8cb9ab7dfd43))
* **lifecycle:** release the mapped modules and the gamedata config in `GameBanFix::Unload`, and close the `dlmount` handle in `~CModule`. Reloading the plugin no longer leaks a module handle per load ([1a799e3](https://github.com/M-archand/GameBanFix-mm/commit/1a799e358bc2921ee8e0c622468b6a58c09cc408))
* **signatures:** carry an explicit mask alongside the pattern bytes so a wildcard is distinguished from a literal `0x2A` byte, which previously made any signature containing that byte match too loosely ([989ab84](https://github.com/M-archand/GameBanFix-mm/commit/989ab84fab091547123566c7d77422a830032f1a))
* **detour:** declare the full set of integer argument registers (four on Win64, six on SysV) and forward them unchanged to the original `GameSystem_Think_CheckSteamBan`, so its arguments survive the detour body ([8cf9836](https://github.com/M-archand/GameBanFix-mm/commit/8cf983650b3a9ae8e83756f6d81de1cbfc760c7e))


### Code Refactoring

* remove the unused schema layer, the vendored `network_connection.pb.h`, and the platform helpers nothing called. ~1,000 lines of scaffolding copied in from CS2Fixes that this plugin never exercised ([1d15a98](https://github.com/M-archand/GameBanFix-mm/commit/1d15a984c616fa347bab78f28ed241e641995a7c))


### Gamedata Changes

**The gamedata file was replaced, not edited.** If you maintain local signature edits, they have to be re-made in the new file.

**Removed**

* `gamedata/gamebanfix.games.txt` - no longer read, and no longer shipped

**New**

* `gamedata/gamebanfix.jsonc` - JSONC (JSON plus `//` comments), with no per-game `"Games"` / `"csgo"` wrapper. `"Signatures"` is now the top-level key

**Changed**

* Signature syntax is IDA style: space-separated hex bytes with `?` for a wildcard, replacing `\xAB` escapes with `\x2A` as the wildcard
* The install path moved from the platform bin folder to `addons/gamebanfix/gamedata/`. Extract the release over `game/csgo` as usual; a stale `gamebanfix.games.txt` left at the old path is ignored and can be deleted


### Miscellaneous

* **deps:** vendor nlohmann/json 3.11.3 as a single header, used only to parse the gamedata at plugin load
* **ci:** bump `actions/checkout` to v7, `actions/upload-artifact` to v7, and `actions/download-artifact` to v8
* **ci:** ship the Linux release archive as `.zip` to match Windows
