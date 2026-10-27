-- Wintergarde Gryphon (27661): quest 12325 ride to Thorson's Post
UPDATE `creature_template` SET `ScriptName` = 'npc_wintergarde_thorson_gryphon' WHERE `entry` = 27661;

DELETE FROM `waypoint_data` WHERE `id` = 2766100;

INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Flight`) VALUES (27661, 1, 2)
ON DUPLICATE KEY UPDATE `Ground` = 1, `Flight` = 2;

UPDATE creature SET position_x = 3722.9023, position_y = -702.7046, position_z = 215.56587 WHERE guid = 131110;
UPDATE creature SET position_x = 3723.7, position_y = -699.1963, position_z = 215.81393 WHERE guid = 131109;
UPDATE creature SET position_x = 3723.6633, position_y = -695.6471, position_z = 215.76389 WHERE guid = 131111;
UPDATE creature SET position_x = 3721.792, position_y = -692.0553, position_z = 215.62848 WHERE guid = 131112;
DELETE FROM vehicle_template_accessory WHERE entry = 27661;
