-- Fix Despawn Bonechewer Riding Wolf after sometimes aggroing Bonechewer Raider
UPDATE smart_scripts set EVENT_PARAM1=3000, EVENT_PARAM2=3000 where entryorguid=18706 and id=2;
