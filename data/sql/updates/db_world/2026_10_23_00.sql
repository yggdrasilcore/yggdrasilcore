-- Defendo-tank 66D has no aggro say. The text incorrectly calls the player a
-- flier even when the player is not flying.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 25758 AND `source_type` = 0 AND `id` = 0;
