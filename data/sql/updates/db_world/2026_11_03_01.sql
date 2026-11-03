-- Sindragosa (36853): Increase aggro/detection range from 20 to 25 yards
UPDATE `creature_template` SET `detection_range` = 25 WHERE `entry` = 36853;
