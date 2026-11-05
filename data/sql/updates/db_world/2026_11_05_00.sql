-- Koltira Deathweaver (28912) - fix the mounted escape route used after
-- High Inquisitor Valroth is killed during the quest "Bloody Breakout".
-- The old route cut the corner and ran him off the path between the bastion
-- exit and the road outside.

DELETE FROM `ycore_world`.`waypoint_data` WHERE `id` = 289130;

INSERT INTO `ycore_world`.`waypoint_data`
    (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`)
VALUES
    (289130, 1, 1653.3800, -5971.8700, 132.2570, NULL, 0, 1, 0, 100, 0),
    (289130, 2, 1655.8798, -5902.9210, 116.1420, NULL, 0, 1, 0, 100, 0),
    (289130, 3, 1687.9838, -5884.5034, 116.1447, NULL, 0, 1, 0, 100, 0);
