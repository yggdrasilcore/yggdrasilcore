-- Makes flying guards above Outlands faction cities aggro on range
UPDATE `creature_template` SET `flags_extra` = `flags_extra` & ~2 WHERE `entry` IN (15241, 15242);

-- Set in combat with target when cast dismounting spell
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (15241,15242) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(15241,0,0,2,10,0,100,0,0,40,3000,8000,1,0,11,38182,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Gryphon Rider Guard - OOC LOS: cast stormhammer on enemy player'),
(15241,0,1,0,0,0,100,0,10000,15000,10000,15000,0,0,11,38182,0,0,0,0,0,5,40,1,0,0,0,0,0,0,'Gryphon Rider Guard - IC update: cast stormhammer on random enemy player'),
(15241,0,2,0,61,0,100,0,0,0,0,0,0,0,49,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Gryphon Rider Guard - Linked: start attack on invoker'),
(15242,0,0,2,10,0,100,0,0,40,3000,8000,1,0,11,38066,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Bat Rider Guard - OOC LOS: cast unstable concoction on enemy player'),
(15242,0,1,0,0,0,100,0,10000,15000,10000,15000,0,0,11,38066,0,0,0,0,0,5,40,1,0,0,0,0,0,0,'Bat Rider Guard - IC update: cast unstable concoction on random enemy player'),
(15242,0,2,0,61,0,100,0,0,0,0,0,0,0,49,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Bat Rider Guard - Linked: start attack on invoker');