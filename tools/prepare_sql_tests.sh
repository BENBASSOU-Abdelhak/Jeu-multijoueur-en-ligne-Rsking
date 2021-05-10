#!/bin/bash

# Download mariadb-server
apk add mariadb-client

# Append configuration to the user file
cat << EOF >> ~/.odbc.ini
[risking-test]
Description=Risking database connection for unit-test
Driver=/usr/local/lib/mariadb/libmaodbc.so
Database=riskingtest
Server=$DB_UNIT_URI
User=$DB_UNIT_USERNAME
Password=$DB_UNIT_PASSWORD
EOF

exit 0