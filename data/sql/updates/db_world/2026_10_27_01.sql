-- Fix Silithid Protector not being hostile on spawn
UPDATE ycore_world.creature_template SET faction=14 where entry=3503;