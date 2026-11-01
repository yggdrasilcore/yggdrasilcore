-- Fezzix Geartwist (25849) - Quest "Patching Up" (11894) roleplay
DELETE FROM smart_scripts WHERE entryorguid = 2584901 AND source_type = 9;
REPLACE INTO smart_scripts
(entryorguid,source_type,id,link,event_type,event_phase_mask,event_chance,event_flags,event_param1,event_param2,event_param3,event_param4,event_param5,event_param6,action_type,action_param1,action_param2,action_param3,action_param4,action_param5,action_param6,target_type,target_param1,target_param2,target_param3,target_param4,target_x,target_y,target_z,target_o,comment)
VALUES
(2584901,9,0,0,0,0,100,0,0,0,0,0,0,0,11,46419,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Cast Cosmetic - Explosion'),
(2584901,9,1,0,0,0,100,0,0,0,0,0,0,0,55,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Stop Waypoint Path (at crash point)'),
(2584901,9,2,0,0,0,100,0,1000,1000,0,0,0,0,43,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Dismount'),
(2584901,9,3,0,0,0,100,0,0,0,0,0,0,0,60,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Set Fly Off'),
(2584901,9,4,0,0,0,100,0,0,0,0,0,0,0,210,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Fall to ground'),
(2584901,9,5,0,0,0,100,0,2000,2000,0,0,0,0,11,42963,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Cast Cosmetic - Combat Knockdown Self'),
(2584901,9,6,0,0,0,100,0,0,0,0,0,0,0,101,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Reset Home Position'),
(2584901,9,7,0,0,0,100,0,0,0,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Set Event Phase 1'),
(2584901,9,8,0,0,0,100,0,0,0,0,0,0,0,66,0,0,0,0,0,0,8,0,0,0,0,0,0,0,4.06662,'Fezzix Geartwist - Actionlist - Set Orientation 4.06662'),
(2584901,9,9,0,0,0,100,0,0,0,0,0,0,0,1,3,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Say Line 3'),
(2584901,9,10,0,0,0,100,0,5000,5000,0,0,0,0,1,4,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Say Line 4'),
(2584901,9,11,0,0,0,100,0,2000,2000,0,0,0,0,59,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Walk back to spawn (set walk)'),
(2584901,9,12,0,0,0,100,0,0,0,0,0,0,0,69,0,0,0,0,0,0,8,0,0,0,0,3477.16,4103.97,17.9091,4.06662,'Fezzix Geartwist - Actionlist - Move to spawn position'),
(2584901,9,13,0,0,0,100,0,3000,3000,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Fezzix Geartwist - Actionlist - Restore run speed');
