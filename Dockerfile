# Build part
FROM registry.app.unistra.fr/risking/build-image:master as compile-part

COPY . /root/risking-serveur
WORKDIR /root/risking-serveur

RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TEST=OFF .
RUN make

# Production environnement part
FROM alpine:latest as runtime-part

# Set time
RUN apk add --no-cache tzdata
ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Alpine seems to use musl-libc (issue similar to https://github.com/kohlschutter/junixsocket/issues/33)
RUN apk add --no-cache libstdc++
# Required for boost
RUN apk add --no-cache boost-dev
# Required for database connection
RUN apk add --no-cache unixodbc

COPY --from=compile-part /usr/local/lib/mariadb/libmaodbc.so /usr/local/lib/mariadb/libmaodbc.so
COPY --from=compile-part /root/risking-serveur/risking /root/risking-serveur/risking
COPY --from=compile-part /root/risking-serveur/1 /root/risking-serveur/1

# Required to find the certificate
WORKDIR /root/risking-serveur

EXPOSE 42424/tcp

ENTRYPOINT ["./risking"]
CMD ["0.0.0.0", "42424", "4"]
