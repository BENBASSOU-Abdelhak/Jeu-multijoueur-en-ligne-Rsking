# Execute this SQL statement with a user having GRANT OPTION on risking and before running tests

CREATE DATABASE IF NOT EXISTS riskingtest DEFAULT CHARACTER SET utf8;

CREATE
    USER 'unittest'@'localhost' IDENTIFIED BY 'boost';

GRANT ALL PRIVILEGES ON
    riskingtest.* TO 'unittest'@'localhost';