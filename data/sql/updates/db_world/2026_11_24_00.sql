-- ICC Crimson Hall: make every Darkfallen trash kill count toward the Blood Prince Council trash counter
-- Remove GUID-specific scripts so the entrance pack uses the shared entry scripts like all other Darkfallen
DELETE FROM smart_scripts WHERE source_type=0 AND entryorguid IN (-201479,-201482,-201646,-201659);
REPLACE INTO smart_scripts (entryorguid,source_type,id,link,event_type,event_phase_mask,event_chance,event_flags,event_param1,event_param2,event_param3,event_param4,event_param5,event_param6,action_type,action_param1,action_param2,action_param3,action_param4,action_param5,action_param6,target_type,target_param1,target_param2,target_param3,target_param4,target_x,target_y,target_z,target_o,comment) VALUES
(37571,0,4,0,6,0,100,512,0,0,0,0,0,0,34,300,1,0,0,0,0,1,0,0,0,0,0.0,0.0,0.0,0.0,'Darkfallen Advisor - On Death - Set Instance Data 300 to 1'),
(37595,0,4,0,6,0,100,512,0,0,0,0,0,0,34,300,1,0,0,0,0,1,0,0,0,0,0.0,0.0,0.0,0.0,'Darkfallen Blood Knight - On Death - Set Instance Data 300 to 1'),
(37662,0,3,0,6,0,100,512,0,0,0,0,0,0,34,300,1,0,0,0,0,1,0,0,0,0,0.0,0.0,0.0,0.0,'Darkfallen Commander - On Death - Set Instance Data 300 to 1'),
(37663,0,3,0,6,0,100,512,0,0,0,0,0,0,34,300,1,0,0,0,0,1,0,0,0,0,0.0,0.0,0.0,0.0,'Darkfallen Noble - On Death - Set Instance Data 300 to 1'),
(37664,0,7,0,6,0,100,512,0,0,0,0,0,0,34,300,1,0,0,0,0,1,0,0,0,0,0.0,0.0,0.0,0.0,'Darkfallen Archmage - On Death - Set Instance Data 300 to 1'),
(37665,0,3,0,6,0,100,512,0,0,0,0,0,0,34,300,1,0,0,0,0,1,0,0,0,0,0.0,0.0,0.0,0.0,'Darkfallen Lieutenant - On Death - Set Instance Data 300 to 1'),
(37666,0,4,0,6,0,100,512,0,0,0,0,0,0,34,300,1,0,0,0,0,1,0,0,0,0,0.0,0.0,0.0,0.0,'Darkfallen Tactician - On Death - Set Instance Data 300 to 1');
