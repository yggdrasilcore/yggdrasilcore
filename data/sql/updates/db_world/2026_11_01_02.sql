-- Make Hamat and Saga flag ally enemies for PvP
UPDATE `factiontemplate_dbc` SET `Flags` = `Flags` | 2048 WHERE `ID` = 1921;

-- Make Hamat sleep when not in combat
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (25604, 25606) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(25606,0,0,0,37,0,100,0,0,0,0,0,0,0,75,45787,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'On AI Init - Apply Aura 45787 on Self'),
(25606,0,1,0,21,0,100,0,0,0,0,0,0,0,75,45787,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'On Reached Home - Apply Aura 45787 on Self'),
(25606,0,2,0,4,0,100,0,0,0,0,0,0,0,28,45787,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'On Aggro - Remove Aura 45787 from Self');