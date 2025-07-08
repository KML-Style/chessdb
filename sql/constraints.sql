CREATE TRIGGER limit_games_per_tournament
BEFORE INSERT ON Games
FOR EACH ROW
BEGIN
    SELECT 
        CASE 
            WHEN (
                (SELECT COUNT(*) FROM Games WHERE tournament_id = NEW.tournament_id)
                >= (SELECT number_of_rounds FROM Tournaments WHERE id = NEW.tournament_id)
            )
            THEN RAISE(ABORT, 'Toutes les rondes de ce tournoi ont déjà été insérées')
        END;
END;