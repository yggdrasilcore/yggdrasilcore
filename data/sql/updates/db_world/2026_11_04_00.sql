-- Fix Crypt Guardian not pathing
UPDATE ycore_world.creature SET MovementType=2 where guid=129660;
UPDATE ycore_world.creature_addon SET PATH_ID=1296600 where guid=129660;
REPLACE INTO ycore_world.waypoint_data (id,point,position_x,position_y,position_z,orientation,delay,move_type,`action`,action_chance,wpguid) VALUES
(1296600,1,1861.9023,-5765.6704,105.0897,NULL,0,0,0,100,0),
(1296600,2,1862.0205,-5742.764,95.5179,NULL,0,0,0,100,0),
(1296600,3,1877.0914,-5743.0674,89.9824,NULL,0,0,0,100,0),
(1296600,4,1876.5425,-5759.717,84.2212,NULL,0,0,0,100,0),
(1296600,5,1877.0914,-5743.0674,89.9824,NULL,0,0,0,100,0),
(1296600,6,1862.0205,-5742.764,95.5179,NULL,0,0,0,100,0),
(1296600,7,1861.81,-5755.25,101.954,NULL,0,0,0,100,0);
-- Fix Scarlet Farm Hound and Scourge Plaguehound not moving
UPDATE ycore_world.creature SET wander_distance=5, MovementType=1 where guid in (128588,130279);
-- Fix Scarlet Champion random movement
UPDATE ycore_world.creature SET wander_distance=0, MovementType=0 where guid=129992;
-- Fix Scarlet Champion path to top
UPDATE ycore_world.creature SET wander_distance=0, MovementType=2 where guid=129971;
UPDATE ycore_world.creature SET wander_distance=0, MovementType=0 where guid=129972;
UPDATE ycore_world.creature_addon SET path_id=1299710 where guid=129971;
REPLACE INTO ycore_world.waypoint_data (id,point,position_x,position_y,position_z,orientation,delay,move_type,action,action_chance,wpguid) VALUES
(1299710,1,1282.7618,-6071.833,14.7632,NULL,0,0,0,100,0),
(1299710,2,1312.9241,-6012.234,40.9688,NULL,0,0,0,100,0),
(1299710,3,1350.3253,-5994.3765,61.7394,NULL,0,0,0,100,0),
(1299710,4,1381.2189,-5963.8843,89.1111,NULL,0,0,0,100,0),
(1299710,5,1432.0283,-5970.586,111.7729,NULL,0,0,0,100,0),
(1299710,6,1381.2189,-5963.8843,89.1111,NULL,0,0,0,100,0),
(1299710,7,1350.3253,-5994.3765,61.7394,NULL,0,0,0,100,0),
(1299710,8,1312.9241,-6012.234,40.9688,NULL,0,0,0,100,0),
(1299710,9,1301.6796,-6107.136,13.9595,NULL,0,0,0,100,0);
REPLACE INTO ycore_world.creature_formations (leaderGUID,memberGUID,dist,angle,groupAI,point_1,point_2) VALUES
(129971,129971,0.0,0.0,515,0,0),
(129971,129972,3.0,180.0,515,0,0);
