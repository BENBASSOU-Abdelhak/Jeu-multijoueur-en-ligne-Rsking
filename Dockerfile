# Build part
FROM alpine:latest as compile-part

RUN apk add --no-cache g++ make cmake boost-dev openssl-dev unixodbc-dev wget tar git

# Install mariadb-connector-odbc manually (package mariadb-connector-odbc package seems missing)
WORKDIR /root/mariadb-connector-odbc
RUN git clone https://github.com/MariaDB/mariadb-connector-odbc.git .
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL .
RUN cmake --build . --config Release
RUN make -j install

COPY . /root/risking-serveur
WORKDIR /root/risking-serveur

RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TEST=OFF .
RUN make

# Production environnement part
FROM alpine:latest as runtime-part

# Alpine seems to use musl-libc (issue similar to https://github.com/kohlschutter/junixsocket/issues/33)
RUN apk add --no-cache libstdc++
# Required for boost
RUN apk add --no-cache boost-dev
# Required for database connection
RUN apk add --no-cache unixodbc

COPY --from=compile-part /root/risking-serveur/risking /usr/bin/risking
COPY --from=compile-part /usr/local/lib/mariadb /usr/local/lib/mariadb

EXPOSE 42424/tcp
CMD /usr/bin/risking
