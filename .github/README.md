# <img alt="image" src="https://github.com/user-attachments/assets/6f96adf9-1867-4ca6-b627-1bd0ab7ee494" />

## Introduction

Yggdrasilcore is an AzerothCore-based open-source game server application and framework designed for emulating support for connections and packets from and sending emulated information to the original abandonware WotLK 3.3.5a client. Read more about the upstream project [here](https://github.com/azerothcore/azerothcore-wotlk).

Our latest mass merge from AzerothCore was [up until this commit in the Playerbots branch](https://github.com/mod-playerbots/azerothcore-wotlk/commit/39187083b04ede6a095e32b39ac4ae4e22dee22f) on 2025-10-25. Only cherry picks are planned hereafter.

Read more about Yggdrasil WoW and see Yggdrasilcore in action [here](https://yggdrasilwow.com/).


## Build Status

[![docker-build](https://github.com/yggdrasilcore/yggdrasilcore/actions/workflows/core-build.yml/badge.svg?branch=master)](https://github.com/yggdrasilcore/yggdrasilcore/actions/workflows/core-build.yml)
<br><br>
We build every commit on a local Dockerized Ubuntu 24.04 machine before anything is merged to master. 

## Philosophy

The AzerothCore project generally uses a rapid, rolling release system, which sees minimal testing between commits (apart from CI checks and basic runtime testing). Resultingly, many AzerothCore-based servers that adhere purely to the upstream codebase receive breaking changes more or less weekly. At Yggdrasilcore, we have inverted this logic to instead change purely that which needs changing - and we have decided that stable, functional code, such as threadsafe monoliths, doesn't need to be broken up/refactored/changed past that which is necessary for Blizzlike functionality. (Curiously enough, this train of thought is similar to the original SunwellCore split from TrinityCore, which in turn later birthed AzerothCore, and alas, here we are.)

Note that Blizzlike in this regard isn't interpreted as purely based on sniffs. We use more data sources and have a more lax policy on what to accept in our codebase, and consider using Wowhead information or YouTube videos for approximate details as satisfactory. 

### Modules

As an AzerothCore-based framework, Yggdrasilcore is designed to be highly modular, allowing developers to extend and customize the game to suit their preferences or create unique gameplay experiences. This flexibility enables the addition of custom features, content, and modifications. Most AzerothCore modules are supported to varying degrees. See their catalogue here [Module Catalogue](https://www.azerothcore.org/catalogue.html#/), or browse the modules we have adapted to Yggdrasilcore in our list of repos [here](https://github.com/YggdrasilWotLK?tab=repositories).

## Differences from upstream AzerothCore

The following is an excerpt of some of our improvements implemented, versus the upstream issues, per 2026-10-22:

1. **Playerbots branch baseline.** We are based on the `mod-playerbots` branch of AzerothCore, including commits [up to 3918708](https://github.com/mod-playerbots/azerothcore-wotlk/commit/39187083b04ede6a095e32b39ac4ae4e22dee22f) (2025-10-25), giving us native support for playerbots-based projects.
2. **Thread safety and UAF hardening.** We have more or less entirely threadproofed and use-after-free-proofed the core. Upstream is riddled with race conditions, and many AzerothCore-based projects see random, near-untraceable crashes from upstream threading and UAF issues. For example, the runtime `CharacterCache` upstream is an unordered list with no locks, so a read (e.g. sending mail or processing auctions) can segfault if it is mutated simultaneously by a character create or delete.
3. **Rewritten core pathing.** We deny pathing that breaks the navmesh or violates reciprocal line of sight, applying LoS checks, VMAP path validation, and proper navmesh resolution. Upstream pet handling, for instance, "extrapolates"/"predicts" future player position on movement, so a pet whose owner runs in place into a wall will run through the wall as a prediction measure. Our rewrite fixes this, so pets no longer run through walls when their owner runs in place. Our changes also greatly improve the pathing of shadows (our name for playerbots, emphasizing incompatibility with upstream `mod-playerbots`).
4. **Batched channel updates.** Channel updates are no longer sent on zone update, but folded into a 5-second update timer, preventing zone-change spam on taxis or rapid zone changes.
5. **Refactored `mod-ale` (rebranded `mod-yla`).** Although outside the core itself, `mod-yla` adds true multistate support and an actual threading model. Upstream has no thread safety and allows map and world threads to race each other, frequently killing Lua script-heavy servers. We deny execution of operations that touch objects on a state other than the caller's, and defer operations that traverse threads (map-change teleports, `RunCommand` called from a player object) to the intended target threads.
6. **Reworked NPC pause-on-interact.** The behaviour is now much more logical and covers all mob types, not just those with quests or gossip. Upstream, walking vendors walk away and break the trade interaction. We instead root all NPCs on interact and only clear the root if a script explicitly demands it (e.g. Muradin on Deathbringer's Rise), and we inject a Lua runtime addon on login that removes the root on interact end, since the client sends no default communication in this case.
7. **Native Warden Inject in C++.** This facilitates AIO and flexible UI serving/management. Thanks to Foereaper for open sourcing his Warden Inject implementation.
8. **Expanded `item_instance` GUID limit.** The limit is now 2⁴⁸ instead of 2³², which the WotLK 3.3.5a client natively supports. That is over 65,000 times more item GUIDs than upstream (and other MaNGOS-based servers), so you will virtually never run out of possible item GUIDs, hedging the issues encountered by projects like Warmane and ChromieCraft by simply using the client's own functionality.
9. **CatmullRom spline pathing.** Implemented for all creatures, so no more instant turns.

The result is a runtime that is much more stable than upstream while improving customizability, especially in `mod-yla`.

## Installation

Yggdarsilcore's installation instructions are the same as upstream. Detailed installation instructions are available [here](http://www.azerothcore.org/wiki/installation).

## Contributing

Yggdrasilcore is a privately managed repository. We do not recommend emulation enthusiasts to follow it. Instead, we recommend that enthusiasts contribute to the upstream AzerothCore project. See their information on how to contribute [here](https://www.azerothcore.org/wiki/contribute).

## Authors & Contributors

Authors from 2016 are available in our Git history. For information prior to 2016, see upstream project information here: [authors](https://github.com/azerothcore/azerothcore-wotlk/blob/master/AUTHORS) file for more details.

## License

- Source: Contrary to some of the source documentation and previous information distributed by AzerothCore, all Yggdrasilcore source components are licensed under GNU GPL v2. 
- Intellectual property: Yggdrasilcore is not related to Blizzard Entertainment. Yggdrasil WoW and its derivative projects lay no claim to Blizzard Entertainment's copyrights and intellectual property, and operate this project solely as an avenue for exploring the functionality of the abandonware WotLK 3.3.5a client in an educational capacity.
