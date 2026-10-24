-- Give core full control of movement flags for Svalna, to script flight anims
UPDATE `creature_template` SET `flags_extra` = `flags_extra`|512 WHERE `entry` IN (37126, 38258);

-- Stop spear throws from bringing players into combat with Svalna
UPDATE `spell_dbc` SET `AttributesEx3` = `AttributesEx3` | 131072 WHERE `ID` = 70078;
UPDATE `spell_dbc` SET `AttributesEx3` = `AttributesEx3` | 131072 WHERE `ID` = 70196;