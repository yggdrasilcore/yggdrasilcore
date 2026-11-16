--
-- Shoveltusk Stag (23691) - territorial headbutt sparring.
--
-- Two male stags periodically square off and ram their antlers together.
-- The C++ AI (npc_shoveltusk_stag) drives the display and the headbutt
-- animation, while creature_sparring keeps the mock fight non-lethal.

UPDATE `creature_template`
SET `ScriptName` = 'npc_shoveltusk_stag'
WHERE `entry` = 23691;

DELETE FROM `creature_sparring`
WHERE `GUID` IN (SELECT `guid` FROM `creature` WHERE `id1` = 23691);

INSERT INTO `creature_sparring` (`GUID`, `SparringPCT`)
SELECT `guid`, 50
FROM `creature`
WHERE `id1` = 23691;

UPDATE ycore_world.creature SET position_x=1482.3374, position_y=-4624.3496, position_z=178.3095 where guid=108650;
UPDATE ycore_world.creature SET MovementType=1, wander_distance=5 where id1=23691 and MovementType=0 and wander_distance=0;
