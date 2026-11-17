-- Fix say line 0 of Sandfury Executioner instead of repeating it on aggro event
UPDATE smart_scripts SET event_flags=1 where entryorguid=7274 and id=0;
