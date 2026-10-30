-- Buzzard, Bloodseeker Bat: waypoint move_type changed from 2 (constantly calling move_land) to 0 (walk)
 
UPDATE ycore_world.waypoint_data
SET move_type = 0
WHERE id IN (70060, 71370, 72510, 72520, 72530, 76140, 491860); 

-- Large AOI Underbat, Avian Flyer, Night Elf Wisp, Wildhammer Gryphon Rider: waypoint move_type changed from 2 (constantly calling move_land) to 1 (run)
UPDATE ycore_world.waypoint_data
SET move_type = 1
WHERE id IN (
    1383120, 1383140, 1383160, 1383210, 1383270, 1383410, 1383480, 1383520,
    1387600, 1387610, 1387620, 1387630, 1387660, 1387690,
    1396910, 1396920, 1396930, 1396940, 1396950, 1396960, 1396970, 1396990,
    11580, 1259220, 1259270, 1259300, 1259310, 1259320, 1260520
);
 
-- Makes Dragonmaw Wind Reaver who previously slow mo landed and called incorrect splines in air, now call correct splines and land speed 
UPDATE ycore_world.waypoint_data
SET move_type = CASE
    WHEN point = 1 THEN 2
    WHEN point = 2 THEN 3
    ELSE 1
END
WHERE id = 1481120;
