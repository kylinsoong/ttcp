FROM alpine as build-env
RUN apk add --no-cache build-base
WORKDIR /app
COPY . .
# Compile the binaries
RUN gcc -o echoserver server.c lib_utils.c lib_ios.c lib_socket.c str_echo.c
FROM alpine
COPY --from=build-env /app/echoserver /app/echoserver
WORKDIR /app
EXPOSE 8877
CMD ["/app/echoserver"] 
