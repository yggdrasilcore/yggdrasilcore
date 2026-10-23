UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_lil_xt' WHERE `entry` = 40703;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 40703 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 4070300 AND `source_type` = 9;
