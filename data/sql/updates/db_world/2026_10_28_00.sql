UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 512 WHERE `entry` = 36853;
UPDATE `creature_template_addon` SET `bytes1` = 0 WHERE `entry` = 36853;
UPDATE `creature_template_movement` SET `Ground` = 0, `Swim` = 0, `Flight` = 1 WHERE `CreatureId` = 36853;
