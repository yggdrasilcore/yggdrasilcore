-- Sister Svalna owns her movement flags in the ICC script: she hovers over the
-- gauntlet and then lands. Her creature_template_movement row has Flight =
-- DisableGravity, so Creature::UpdateMovementFlags() would otherwise re-apply or
-- clear the gravity flag on every terrain update and fight the script. 512 is
-- CREATURE_FLAG_EXTRA_NO_MOVE_FLAGS_UPDATE, which makes the script authoritative.
UPDATE `creature_template` SET `flags_extra` = `flags_extra`|512 WHERE `entry` IN (37126, 38258);
