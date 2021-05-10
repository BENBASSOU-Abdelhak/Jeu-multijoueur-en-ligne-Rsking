-- -----------------------------------------------------
-- Table `user`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `user` (
  `id` VARBINARY(16) NOT NULL,
  `gamertag` VARCHAR(45) NOT NULL,
  `password` BINARY(128) NULL,
  `email` VARCHAR(128) NULL,
  CHECK ((`password` IS NULL AND `email` IS NULL) OR (`password` IS NOT NULL AND `email` IS NOT NULL)),
  PRIMARY KEY (`id`),
  UNIQUE INDEX `email_UNIQUE` (`email` ASC),
  UNIQUE INDEX `gamertag_UNIQUE` (`gamertag` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `game`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `game` (
  `id` VARBINARY(16) NOT NULL,
  `start_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP() ON UPDATE CURRENT_TIMESTAMP(),
  `end_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP() ON UPDATE CURRENT_TIMESTAMP(),
  `number_of_players` SMALLINT UNSIGNED NOT NULL CHECK(number_of_players > 1),
  PRIMARY KEY (`id`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `game_has_user`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `game_has_user` (
  `rank` SMALLINT UNSIGNED NOT NULL CHECK(`rank` > 0),
  `game_id` VARBINARY(16) NOT NULL,
  `user_id` VARBINARY(16) NOT NULL,
  PRIMARY KEY (`game_id`, `user_id`),
  INDEX `fk_game_has_user_game1_idx` (`game_id` ASC),
  INDEX `fk_game_has_user_user1_idx` (`user_id` ASC),
  CONSTRAINT `fk_game_has_user_game1`
    FOREIGN KEY (`game_id`)
    REFERENCES `game` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_game_has_user_user1`
    FOREIGN KEY (`user_id`)
    REFERENCES `user` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
    CONSTRAINT game_rank UNIQUE (game_id, `rank`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `ban`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `ban` (
  `id` VARBINARY(16) NOT NULL,
  `expiration` TIMESTAMP NULL,
  `date` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP(),
  `user_id` VARBINARY(16) NOT NULL,
  `reason` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `fk_ban_user1_idx` (`user_id` ASC),
  CONSTRAINT `fk_ban_user1`
    FOREIGN KEY (`user_id`)
    REFERENCES `user` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;
