-- Brazie Getz: the "You're an odd little guy." gossip had no action menu, so
-- selecting it did nothing. The script closes the conversation.
UPDATE `creature_template` SET `ScriptName` = 'npc_brazie_getz' WHERE `entry` = 37904;
