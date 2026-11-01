-- Makes Hippogryphs hover as they should
INSERT INTO creature_template_movement (CreatureId, Ground, Swim, Flight, Rooted, Chase, Random, InteractionPauseTimer) VALUES
(6648,2,0,0,0,0,0,NULL),
(5347,2,1,1,0,0,0,NULL),
(40165,2,0,0,0,0,0,NULL),
(16098,2,0,0,0,0,0,NULL),
(9527,2,0,0,0,0,0,NULL),
(5300,2,0,0,0,0,0,NULL),
(5306,2,0,0,0,0,0,NULL),
(5305,2,0,0,0,0,0,NULL),
(5304,2,0,0,0,0,0,NULL),
(12676,2,0,0,0,0,0,NULL),
(23772,2,0,1,0,0,0,NULL),
(24515,2,0,0,0,0,0,NULL),
(24514,2,0,1,0,0,0,NULL),
(6380,2,0,0,0,0,0,NULL),
(6375,2,0,0,0,0,0,NULL),
(6379,2,0,0,0,0,0,NULL),
(6378,2,0,0,0,0,0,NULL),
(6377,2,0,0,0,0,0,NULL)
ON DUPLICATE KEY UPDATE
Ground = VALUES(Ground),
Swim = VALUES(Swim),
Flight = VALUES(Flight),
Rooted = VALUES(Rooted),
Chase = VALUES(Chase),
Random = VALUES(Random),
InteractionPauseTimer = VALUES(InteractionPauseTimer);
