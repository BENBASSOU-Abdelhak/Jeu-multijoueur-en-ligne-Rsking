#!/bin/bash

mysql -u $DB_UNIT_USERNAME -h $DB_UNIT_URI -p$DB_UNIT_PASSWORD << EOF
USE riskingtest;
DELETE FROM ban;
DELETE FROM game;
EOF
