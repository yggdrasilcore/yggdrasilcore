-- Fix erroneous elemental model of invis LK stalker

DELETE FROM `creature_template_model` WHERE `CreatureID` = 28765;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(28765, 0, 23767, 1, 1, 12340);


-- Fixes Dark Rider of Acherus quest
DELETE FROM `vehicle_template_accessory` WHERE `entry` = 28782;

UPDATE `creature_template` SET `faction` = 16 WHERE `entry` = 28782;

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 28768;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(28768,0,0,0,0,0,100,0,1000,1000,6000,6000,0,0,11,52372,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Dark Rider of Acherus - Cast Spell Icy Touch'),
(28768,0,1,0,0,0,100,0,3000,3000,6000,6000,0,0,11,52374,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Dark Rider of Acherus - Cast Spell Blood Strike'),
(28768,0,2,0,0,0,100,0,5000,5000,6000,6000,0,0,11,50688,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Dark Rider of Acherus - Cast Spell Plague Strike'),
(28768,0,3,4,6,0,100,512,0,0,0,0,0,0,19,33554434,0,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - On Death - Remove Horse Not Selectable And Non Attackable'),
(28768,0,4,5,61,0,100,512,0,0,0,0,0,0,142,100,0,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - On Death - Horse Health Full'),
(28768,0,5,6,61,0,100,512,0,0,0,0,0,0,45,1,1,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - On Death - Set Horse Data'),
(28768,0,6,7,61,0,100,512,0,0,0,0,0,0,1,0,0,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - On Death - Horse Emote Message'),
(28768,0,7,0,61,0,100,512,0,0,0,0,0,0,203,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dark Rider of Acherus - On Death - Exit Vehicle'),
(28768,0,8,0,1,0,100,257,200,200,0,0,0,0,11,46598,2,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - 200 ms After Summon - Ride Horse'),
(28768,0,9,10,4,0,100,0,0,0,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Dark Rider of Acherus - On Aggro - Store Enemy'),
(28768,0,10,11,61,0,100,0,0,0,0,0,0,0,100,1,0,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - On Aggro - Send Stored Enemy To Horse'),
(28768,0,11,0,61,0,100,0,0,0,0,0,0,0,45,2,1,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - On Aggro - Set Horse Data Combat Start'),
(28768,0,12,0,7,0,100,0,0,0,0,0,0,0,45,2,0,0,0,0,0,19,28782,20,0,0,0,0,0,0,'Dark Rider of Acherus - On Evade - Set Horse Data Combat End');

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 28782;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(28782,0,0,1,27,0,100,512,0,0,0,0,0,0,28,23973,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Passenger Boarded - Remove Aura 23973'),
(28782,0,1,0,61,0,100,512,0,0,0,0,0,0,22,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Passenger Boarded - Set Phase 0'),
(28782,0,2,3,28,0,100,0,0,0,0,0,0,0,5,377,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Passenger Removed - Play Emote 377'),
(28782,0,3,4,61,0,100,0,0,0,0,0,0,0,2,35,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Passenger Removed - Set Faction Friendly'),
(28782,0,4,5,61,0,100,0,0,0,0,0,0,0,101,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Passenger Removed - Set Home Pos'),
(28782,0,5,6,61,0,100,0,0,0,0,0,0,0,75,23973,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Passenger Removed - Add Aura 23973 On Self'),
(28782,0,6,0,61,0,100,0,0,0,0,0,0,0,41,3000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Passenger Removed - Despawn In 3000 ms'),
(28782,0,7,15,38,0,100,0,1,1,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set - Set Phase 1'),
(28782,0,8,9,1,1,100,257,30000,30000,0,0,0,0,28,23973,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - Phase 1 After 30 Sec - Remove Aura 23973'),
(28782,0,9,0,61,0,100,0,0,0,0,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - Phase 1 After 30 Sec - Despawn'),
(28782,0,10,0,1,0,100,1,200,200,0,0,0,0,12,28768,6,30000,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - 200 ms After Spawn - Summon Dark Rider'),
(28782,0,11,0,38,0,100,0,2,1,0,0,0,0,49,0,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set Combat Start - Attack Stored Rider Enemy'),
(28782,0,12,0,38,0,100,0,2,0,0,0,0,0,27,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set Combat End - Combat Stop'),
(28782,0,13,0,0,1,100,0,500,500,500,500,0,0,2,35,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - Phase 1 In Combat - Keep Faction Friendly'),
(28782,0,14,0,1,1,100,0,500,500,500,500,0,0,2,35,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - Phase 1 Out Of Combat - Keep Faction Friendly'),
(28782,0,15,16,61,0,100,0,0,0,0,0,0,0,2,35,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set - Set Faction Friendly'),
(28782,0,16,17,61,0,100,0,0,0,0,0,0,0,27,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set - Stop Combat'),
(28782,0,17,19,61,0,100,0,0,0,0,0,0,0,75,23973,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set - Add Aura 23973 On Self'),
(28782,0,18,0,7,1,100,0,0,0,0,0,0,0,75,23973,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - Phase 1 On Evade - Add Aura 23973 On Self'),
(28782,0,19,20,61,0,100,0,0,0,0,0,0,0,117,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set - Disable Evade'),
(28782,0,20,21,61,0,100,0,0,0,0,0,0,0,103,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set - Root'),
(28782,0,21,0,61,0,100,0,0,0,0,0,0,0,211,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Acherus Deathcharger - On Data Set - Disable Phase Reset');