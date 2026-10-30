UPDATE `creature_template` SET `flags_extra` = `flags_extra` | 512 WHERE `entry` IN (37533, 37534);
UPDATE `creature_template_addon` SET `bytes1` = 0 WHERE `entry` IN (37533, 37534);
UPDATE `creature_template_movement` SET `Ground` = 1, `Swim` = 0, `Flight` = 2 WHERE `CreatureId` IN (37533, 37534);
