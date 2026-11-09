-- Fix Dark Portal NPC to return in formation instead of staying impiled at the end of waypoint
UPDATE ycore_world.smart_scripts SET action_param1 = 1 WHERE source_type = 0 AND entryorguid IN (18948, 18950, 18965, 18970, 18972, 18986) AND id = 5 AND event_type = 59 AND action_type = 101 AND action_param1 = 0;
