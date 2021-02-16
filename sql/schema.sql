-- Tested on MariaDB 10.3.25

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';

-- -----------------------------------------------------
-- Schema risking
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `risking` DEFAULT CHARACTER SET utf8 ;
USE `risking` ;


-- -----------------------------------------------------
-- Table `risking`.`user`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `risking`.`user` (
  `id` VARBINARY(16) NOT NULL,
  `gamertag` VARCHAR(45) NOT NULL,
  `password` BINARY(64) NOT NULL,
  `email` VARCHAR(128) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE INDEX `email_UNIQUE` (`email` ASC),
  UNIQUE INDEX `gamertag_UNIQUE` (`gamertag` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `risking`.`game`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `risking`.`game` (
  `id` VARBINARY(16) NOT NULL,
  `start_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP() ON UPDATE CURRENT_TIMESTAMP(),
  `end_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP() ON UPDATE CURRENT_TIMESTAMP(),
  `number_of_players` SMALLINT UNSIGNED NOT NULL CHECK(number_of_players > 1),
  PRIMARY KEY (`id`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `risking`.`game_has_user`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `risking`.`game_has_user` (
  `rank` SMALLINT UNSIGNED NOT NULL CHECK(`rank` > 0),
  `game_id` VARBINARY(16) NOT NULL,
  `user_id` VARBINARY(16) NOT NULL,
  PRIMARY KEY (`game_id`, `user_id`),
  INDEX `fk_game_has_user_game1_idx` (`game_id` ASC),
  INDEX `fk_game_has_user_user1_idx` (`user_id` ASC),
  CONSTRAINT `fk_game_has_user_game1`
    FOREIGN KEY (`game_id`)
    REFERENCES `risking`.`game` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_game_has_user_user1`
    FOREIGN KEY (`user_id`)
    REFERENCES `risking`.`user` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
    CONSTRAINT game_rank UNIQUE (game_id, `rank`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `risking`.`ban`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `risking`.`ban` (
  `id` VARBINARY(16) NOT NULL,
  `expiration` TIMESTAMP NULL,
  `date` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP(),
  `user_id` VARBINARY(16) NOT NULL,
  `reason` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `fk_ban_user1_idx` (`user_id` ASC),
  CONSTRAINT `fk_ban_user1`
    FOREIGN KEY (`user_id`)
    REFERENCES `risking`.`user` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
