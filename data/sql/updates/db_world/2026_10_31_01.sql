-- Makes all Warlock spells usable when metamorphosed, some were missed in original implementation

UPDATE spell_dbc SET Attributes = Attributes & ~65536 WHERE ID IN (17877,17962,18220,18708,19028,30108,30146,30283,47193,50796);