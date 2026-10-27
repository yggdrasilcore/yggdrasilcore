-- Fix for quest the Ashenvale Hunt that can be taken and completed once from 4 different npcs
REPLACE INTO ycore_world.conditions
(SourceTypeOrReferenceId,SourceGroup,SourceEntry,SourceId,ElseGroup,ConditionTypeOrReference,ConditionTarget,ConditionValue1,ConditionValue2,ConditionValue3,NegativeCondition,ErrorType,ErrorTextId,ScriptName,Comment)
VALUES
(19,0,6382,0,0,14,0,235,0,0,0,0,0,'','Ashenvale Hunt: 235 never took/active/completed'),
(19,0,6382,0,0,14,0,742,0,0,0,0,0,'','Ashenvale Hunt: 742 never took/active/completed'),
(19,0,6382,0,0,14,0,6383,0,0,0,0,0,'','Ashenvale Hunt: 742 never took/active/completed'),
(19,0,235,0,0,14,0,6382,0,0,0,0,0,'','Ashenvale Hunt: 6382 never took/active/completed'),
(19,0,235,0,0,14,0,6383,0,0,0,0,0,'','Ashenvale Hunt: 6383 never took/active/completed'),
(19,0,235,0,0,14,0,742,0,0,0,0,0,'','Ashenvale Hunt: 742 never took/active/completed'),
(19,0,742,0,0,14,0,6382,0,0,0,0,0,'','Ashenvale Hunt: 6382 never took/active/completed'),
(19,0,742,0,0,14,0,6383,0,0,0,0,0,'','Ashenvale Hunt: 6382 never took/active/completed'),
(19,0,742,0,0,14,0,235,0,0,0,0,0,'','Ashenvale Hunt: 235 never took/active/completed');

UPDATE ycore_world.quest_template_addon SET ExclusiveGroup=235 WHERE id=6383; 
