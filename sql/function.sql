DELIMITER //

# Identique à UUID_TO_BIN de Mysql 8+ mais qui ne semble pas disponible chez nous
CREATE OR REPLACE FUNCTION UID_TO_BIN(uuid CHAR(36))
    RETURNS BINARY(16)
    READS SQL DATA
    DETERMINISTIC
BEGIN
    RETURN UNHEX(REPLACE(uuid, '-', ''));
END
//

CREATE OR REPLACE FUNCTION ban(p_gamertag VARCHAR(45), p_reason VARCHAR(255))
    RETURNS VARBINARY(16)
    MODIFIES SQL DATA
    NOT DETERMINISTIC
BEGIN
    # TODO: Théoriquement, cela pourrait dupliquer l'UUID (faible proba mais quand même...)
    SET @uid = UID_TO_BIN(UUID());
    IF (SELECT u.id FROM user u WHERE u.gamertag = p_gamertag) IS NULL THEN
        # Player data not exist, return NULL
        RETURN NULL;
    END IF;
    SELECT COUNT(b.id) >= 1
    INTO @cdt
    FROM ban b
    WHERE b.user_id = (SELECT u.id FROM user u WHERE u.gamertag = p_gamertag);
    INSERT INTO ban (id, expiration, user_id, reason)
    VALUES (@uid,
            IF(@cdt, NULL, CURRENT_TIMESTAMP + INTERVAL 7 DAY),
            (SELECT u.id FROM user u WHERE u.gamertag = p_gamertag),
            p_reason);
    RETURN @uid;
END
//

CREATE OR REPLACE FUNCTION create_game(p_num_players SMALLINT(5))
    RETURNS VARBINARY(16)
    MODIFIES SQL DATA
    NOT DETERMINISTIC
BEGIN
    # TODO: Théoriquement, cela pourrait dupliquer l'UUID (faible proba mais quand même...)
    SET @uid = UID_TO_BIN(UUID());
    INSERT INTO game (id, number_of_players) VALUE (@uid, p_num_players);
    RETURN @uid;
END //

DELIMITER ;