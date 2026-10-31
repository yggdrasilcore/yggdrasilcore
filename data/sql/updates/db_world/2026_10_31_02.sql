-- Makes all steam vents look like steam vents
DELETE FROM `creature_template_model` WHERE `CreatureID` = 25739;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES('25739','0','23343','1','1','12340');