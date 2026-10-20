DELETE FROM `smart_scripts` WHERE (`entryorguid` = 10676 AND `source_type` = 0 AND `id` BETWEEN 11 AND 15) OR (`entryorguid` = 10682 AND `source_type` = 0 AND `id` BETWEEN 1 AND 5);

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `event_param6`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(10676, 0, 11, 0, 4, 0, 100, 512, 0, 0, 0, 0, 0, 0, 136, 1, 1, 12, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Jhash - On Aggro - Set Run Speed to 1.12'),
(10676, 0, 12, 0, 7, 0, 100, 512, 0, 0, 0, 0, 0, 0, 43, 0, 207, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Jhash - On Evade - Mount'),
(10676, 0, 13, 0, 7, 0, 100, 512, 0, 0, 0, 0, 0, 0, 136, 1, 1, 38571, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Jhash - On Evade - Set Run Speed to 1.38571'),
(10676, 0, 14, 0, 25, 0, 100, 512, 0, 0, 0, 0, 0, 0, 43, 0, 207, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Jhash - On Reset - Mount'),
(10676, 0, 15, 0, 25, 0, 100, 512, 0, 0, 0, 0, 0, 0, 136, 1, 1, 38571, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Jhash - On Reset - Set Run Speed to 1.38571'),
(10682, 0, 1, 0, 4, 0, 100, 512, 0, 0, 0, 0, 0, 0, 136, 1, 1, 12, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Kerr - On Aggro - Set Run Speed to 1.12'),
(10682, 0, 2, 0, 7, 0, 100, 512, 0, 0, 0, 0, 0, 0, 43, 0, 2328, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Kerr - On Evade - Mount'),
(10682, 0, 3, 0, 7, 0, 100, 512, 0, 0, 0, 0, 0, 0, 136, 1, 1, 38571, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Kerr - On Evade - Set Run Speed to 1.38571'),
(10682, 0, 4, 0, 25, 0, 100, 512, 0, 0, 0, 0, 0, 0, 43, 0, 2328, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Kerr - On Reset - Mount'),
(10682, 0, 5, 0, 25, 0, 100, 512, 0, 0, 0, 0, 0, 0, 136, 1, 1, 38571, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Raider Kerr - On Reset - Set Run Speed to 1.38571');
