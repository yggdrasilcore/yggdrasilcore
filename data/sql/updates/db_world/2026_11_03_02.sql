-- Fix Position of Heckling Fel Sprite and Flamewaker Imp
UPDATE ycore_world.creature SET position_x=148.383, position_y=2358.45, position_z=54.4773 where guid=68413;
UPDATE ycore_world.creature SET wander_distance=5, MovementType=1 where guid in (68218,68217);
