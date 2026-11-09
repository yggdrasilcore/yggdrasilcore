UPDATE `creature_template` SET `HoverHeight` = 1 WHERE `entry` IN (23688);
REPLACE INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Chase`, `Random`, `InteractionPauseTimer`) VALUES
('23688','2','0','0','0','0','0',NULL);