-- Brazie Getz: the "You're an odd little guy." gossip had no action menu, so
-- selecting it did nothing. The script closes the conversation.
UPDATE `creature_template` SET `ScriptName` = 'npc_brazie_getz' WHERE `entry` = 37904;

-- Give the Icecrown Citadel Gunship Battle rocketeers/cannoneers their C++ AI
-- in every difficulty (base + 10/25 normal/heroic template entries).
UPDATE `creature_template` SET `ScriptName` = 'npc_gunship_rocketeer'
WHERE `entry` IN (36978, 36982, 38405, 38407, 38681, 38682, 38687, 38688);
