-- Make dragon bosses in Sunken Temple hover

UPDATE `creature_template` SET `HoverHeight` = 8 WHERE `entry` IN (5719,5720,5721,5722);
REPLACE INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Chase`, `Random`, `InteractionPauseTimer`) values
('5719','2','0','0','0','0','0',NULL),
('5720','2','0','0','0','0','0',NULL),
('5721','2','0','0','0','0','0',NULL),
('5722','2','0','0','0','0','0',NULL);