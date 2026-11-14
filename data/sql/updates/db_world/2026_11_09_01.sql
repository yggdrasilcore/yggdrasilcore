-- Sets hover on all Chimaeras without special air pathing (e.g., Nuramoc), dragons, and Proto-Whelps
UPDATE `creature_template` SET `HoverHeight` = 1 WHERE `entry` IN (26322, 2726, 10321, 20142, 23688,12801,21033,10807,7448,7449,6167,12802,12800,7447,4031,12803,8764,8763,8660,11497,21879,4032);
UPDATE `creature_template` SET `HoverHeight` = 5 WHERE `entry` IN (23164, 14388, 12496, 12497, 12498, 12899, 12900, 10662, 10663, 10664, 10202, 4339, 5312, 5718, 7044, 7045, 8197, 8976);
REPLACE INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Chase`, `Random`, `InteractionPauseTimer`) VALUES
(23688,2,0,0,0,0,0,NULL),
(4031,2,0,0,0,0,0,NULL),
(4032,2,0,0,0,0,0,NULL),
(6167,2,0,0,0,0,0,NULL),
(7447,2,0,0,0,0,0,NULL),
(7448,2,0,0,0,0,0,NULL),
(7449,2,0,0,0,0,0,NULL),
(10807,2,0,0,0,0,0,NULL),
(21033,2,1,1,0,0,0,NULL),
(21879,2,1,1,0,0,0,NULL),
(12802,2,0,0,0,0,0,NULL),
(12800,2,0,0,0,0,0,NULL),
(12803,2,0,0,0,0,0,NULL),
(8976,2,0,0,0,0,0,NULL),
(8764,2,0,0,0,0,0,NULL),
(8763,2,0,0,0,0,0,NULL),
(2726,2,0,0,0,0,0,NULL),
(8660,2,0,0,0,0,0,NULL),
(20142,2,0,0,0,0,0,NULL),
(11497,2,0,0,0,0,0,NULL),
(10321,2,0,0,0,0,0,NULL),
(14388,2,0,0,0,0,0,NULL),
(12801,2,0,0,0,0,0,NULL),
(8197,2,0,0,0,0,0,NULL),
(26322,2,0,0,0,0,0,NULL),
(23164,2,0,0,0,0,0,NULL),
(7045,2,0,0,0,0,0,NULL),
(7044,2,0,0,0,0,0,NULL),
(5718,2,0,0,0,0,0,NULL),
(5312,2,0,0,0,0,0,NULL),
(10202,2,0,0,0,0,0,NULL),
(12496,2,0,0,0,0,0,NULL),
(12497,2,0,0,0,0,0,NULL),
(12498,2,0,0,0,0,0,NULL),
(12899,2,0,0,0,0,0,NULL),
(12900,2,0,0,0,0,0,NULL),
(10662,2,0,0,0,0,0,NULL),
(10663,2,0,0,0,0,0,NULL),
(10664,2,0,0,0,0,0,NULL),
(4339,2,0,0,0,0,0,NULL);

-- Wowhead data for Blink Dragons: https://www.wowhead.com/wotlk/npc=3815/blink-dragon
-- Moves a Blink Dragon up from underground
UPDATE creature SET position_z = 181.3 WHERE guid = 34503;

-- Add missing Blink Dragon spawn re Wowhead data 
replace into `creature` (`guid`, `id1`, `id2`, `id3`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) values('15001858','3815','0','0','1','0','0','1','1','0','1854.09','-1729.31','60.585','5.63287','300','0','0','788','0','0','0','0','0','',NULL,'0',NULL);

-- Re Wowhead data, Blink Dragons in the wild move slightly, set wanderdistance 5
UPDATE creature SET wander_distance = 5, MovementType = 1 WHERE guid IN (34503, 15001858);

-- Wowhead data for Wily Fey Dragon: https://www.wowhead.com/classic/npc=4017/wily-fey-dragon#comments
-- This one should move quite a bit, but currently doesn't move at all
UPDATE creature SET wander_distance = 40, MovementType = 1 WHERE id1 = 4017;

-- YouTube info for Nal'Taszar, short random movement here (2 yds ish): https://www.youtube.com/watch?v=MYJqEqwtToc
UPDATE creature SET wander_distance = 2, MovementType = 1 WHERE id1 = 4066;

-- Spellmaw needs large wander area: https://www.wowhead.com/classic/npc=10662/spellmaw
UPDATE creature SET wander_distance = 40, MovementType = 1 WHERE id1 = 10662;

-- Manaclaw reports slight movement on Wowhead: https://www.wowhead.com/classic/npc=10663/manaclaw
UPDATE creature SET wander_distance = 5, MovementType = 1 WHERE id1 = 10663;

-- Fixes previously flying Ruby Watcher corpse
replace into `creature` (`guid`, `id1`, `id2`, `id3`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) values('108330','27530','0','0','571','0','0','1','1','0','3776','750.696','58.0335','2.30279','300','5','0','102820','0','1','0','0','0','','0','0',NULL);

-- Fix Unmount of Proto-Drake Handler with Enslaved Proto Drake
DELETE from vehicle_template_accessory where entry=24083;

-- Add spawn of Njorndar Proto-Drake 
REPLACE INTO creature (guid,id1,id2,id3,map,zoneId,areaId,spawnMask,phaseMask,equipment_id,position_x,position_y,position_z,orientation,spawntimesecs,wander_distance,currentwaypoint,curhealth,curmana,MovementType,npcflag,unit_flags,dynamicflags,ScriptName,VerifiedBuild,CreateObject,Comment) VALUES
(15001862,30272,0,0,571,0,0,1,1,0,7793.9,3736.35,654.095,1.9553,300,0.0,0,37800,0,0,0,0,0,'',NULL,0,NULL),
(15001863,30272,0,0,571,0,0,1,1,0,7819.02,3733.21,653.442,1.27122,300,0.0,0,37800,0,0,0,0,0,'',NULL,0,NULL);
