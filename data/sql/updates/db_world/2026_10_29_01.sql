-- Gryphon Rider Guard (15241) and Bat Rider Guard (15242) cast Stormhammer (38182)
-- and Unstable Concoction (38066) on players. The core spell script dismounts the
-- player when either spell lands.

UPDATE ycore_world.creature_template_movement SET Rooted=0 where CreatureId=15241;
UPDATE ycore_world.creature_template SET AIName='SmartAI' where entry=15241;

REPLACE INTO ycore_world.smart_scripts
(entryorguid,source_type,id,link,event_type,event_phase_mask,event_chance,event_flags,event_param1,event_param2,event_param3,event_param4,event_param5,action_type,action_param1,action_param2,action_param3,target_type,target_param1,target_param2,target_param3,comment)
VALUES
(15241,0,0,0, 10,0,100,0, 0,40,3000,8000,1,11,38182,0,0,7,0,0,0, 'Gryphon Rider Guard - OOC LOS: cast stormhammer on enemy player'),
(15241,0,1,0, 0,0,100,0, 1000,2000,3000,8000,0,11,38182,0,0,5,40,1,0, 'Gryphon Rider Guard - IC update: cast stormhammer on random enemy player');

UPDATE ycore_world.creature_template SET AIName='SmartAI' where entry=15242;
REPLACE INTO ycore_world.smart_scripts
(entryorguid,source_type,id,link,event_type,event_phase_mask,event_chance,event_flags,event_param1,event_param2,event_param3,event_param4,event_param5,action_type,action_param1,action_param2,action_param3,target_type,target_param1,target_param2,target_param3,comment)
VALUES
(15242,0,0,0, 10,0,100,0, 0,40,3000,8000,1,11,38066,0,0,7,0,0,0, 'Bat Rider Guard - OOC LOS: cast unstable concoction on enemy player'),
(15242,0,1,0, 0,0,100,0, 1000,2000,3000,8000,0,11,38066,0,0,5,40,1,0, 'Bat Rider Guard - IC update: cast unstable concoction on random enemy player');

-- Bind the dismount spell script from Core/Scripts/Spells/spell_generic.cpp
REPLACE INTO ycore_world.spell_script_names (`spell_id`, `ScriptName`) VALUES
(38182, 'spell_gen_rider_guard_dismount'),
(38066, 'spell_gen_rider_guard_dismount');
