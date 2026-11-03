-- Makes Borean Frogs have appropriate movement (previously these were standing still)
UPDATE ycore_world.creature SET wander_distance=5, MovementType=1 WHERE id1=25677;
UPDATE ycore_world.creature_template SET speed_run=1.14286, movementtype=1, speed_walk=1.7 WHERE entry=25677;