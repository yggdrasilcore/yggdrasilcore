UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_icc_flesh_eating_insect' WHERE `entry` = 37782;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 37782 AND `source_type` = 0;
