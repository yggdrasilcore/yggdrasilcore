-- Scarlet Fleet Defender (28834): attack the cannon that hit them and pull nearby defenders
UPDATE `creature_template` SET `ScriptName`='npc_scarlet_fleet_defender' WHERE `entry`=28834;

-- Scarlet Fleet Defender yells when hit by the Scarlet Cannon
DELETE FROM `creature_text` WHERE `CreatureID`=28834 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(28834,0,0,'The Scourge has broken through!',14,0,100,0,0,0,28972,0,'Scarlet Fleet Defender - Cannon Hit'),
(28834,0,1,'BEHIND US! They\'ve taken the ships!',14,0,100,0,0,0,28973,0,'Scarlet Fleet Defender - Cannon Hit'),
(28834,0,2,'MONSTERS!',14,0,100,0,0,0,28974,0,'Scarlet Fleet Defender - Cannon Hit'),
(28834,0,3,'How could this have happened!?',14,0,100,0,0,0,28975,0,'Scarlet Fleet Defender - Cannon Hit'),
(28834,0,4,'Get \'em off that ship! Kill every last one of \'em!',14,0,100,0,0,0,28976,0,'Scarlet Fleet Defender - Cannon Hit'),
(28834,0,5,'Take cover! Take cover!',14,0,100,0,0,0,28977,0,'Scarlet Fleet Defender - Cannon Hit'),
(28834,0,6,'I don\'t believe it!',14,0,100,0,0,0,28978,0,'Scarlet Fleet Defender - Cannon Hit'),
(28834,0,7,'Death to the Scourge!',14,0,100,0,0,0,28979,0,'Scarlet Fleet Defender - Cannon Hit');
