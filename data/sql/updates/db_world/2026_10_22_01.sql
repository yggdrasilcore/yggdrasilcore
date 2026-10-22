-- Toy Train Set (193963) and Wind-Up Train Wrecker (33404)
-- Move both NPCs from SmartAI to dedicated C++ scripts.

UPDATE `gameobject_template` SET `AIName` = '', `ScriptName` = 'go_toy_train_set' WHERE `entry` = 193963;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_train_wrecker' WHERE `entry` = 33404;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 193963 AND `source_type` = 1;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 33404 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 3340400 AND `source_type` = 9;
