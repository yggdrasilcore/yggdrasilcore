-- Scarlet Cannon (28833): use the Salvaged Siege Turret vehicle kit (337) and spellclick spell 65031
UPDATE `creature_template` SET `VehicleId`=337 WHERE `entry`=28833;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=28833;
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES (28833,65031,1,0);

-- Keep the "Massacre At Light's Point" quest requirement on the spellclick
UPDATE `conditions` SET `SourceEntry`=65031 WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=28833 AND `SourceEntry`=52447;
