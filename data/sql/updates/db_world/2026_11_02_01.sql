-- Correct positioning on gryphon ride end after Massacre at Light's Point, give time for spline to finish

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (28864,2886400) AND `source_type` IN (0,9);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(28864,0,1,0,25,0,100,512,0,0,0,0,0,0,11,43775,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Scourge Gryphon - On Reset - Cast ''Flight'''),
(28864,0,2,0,25,0,100,512,0,0,0,0,0,0,53,2,28864,0,0,0,0,1,0,0,0,0,0,0,0,0,'Scourge Gryphon - On Reset - Start Waypoint'),
(28864,0,3,0,40,0,100,512,6,28864,0,0,0,0,80,2886400,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Scourge Gryphon - On Waypoint 6 Reached - Run Script'),
(2886400,9,0,0,0,0,100,0,4000,4000,0,0,0,0,28,46598,0,0,0,0,0,23,0,0,0,0,0,0,0,0,'Scourge Gryphon - Script - Remove Aura ''Ride Vehicle Hardcoded'''),
(2886400,9,1,0,0,0,100,0,100,100,0,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Scourge Gryphon - Script - Despawn Instant');