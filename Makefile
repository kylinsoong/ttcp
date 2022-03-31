build:
	gcc src-chat/chatserver.c -o tcp-chat-server
	gcc src-chat/chatclient.c -o tcp-chat-client
	gcc src-echo/server.c src-echo/lib_utils.c src-echo/lib_ios.c src-echo/lib_socket.c src-echo/str_echo.c -o tcp-echo-server
	gcc src-echo/client.c src-echo/lib_utils.c src-echo/lib_ios.c src-echo/lib_socket.c src-echo/str_cli.c -o tcp-echo-client
	ls -ll tcp-*
clean:
	rm -f tcp-chat-server
	rm -f tcp-chat-client
	rm -f tcp-echo-server
	rm -f tcp-echo-client
install:
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 tcp-chat-server $(DESTDIR)/usr/bin/tcp-chat-server
	install -m 0755 tcp-chat-client $(DESTDIR)/usr/bin/tcp-chat-client
	install -m 0755 tcp-echo-server $(DESTDIR)/usr/bin/tcp-echo-server
	install -m 0755 tcp-echo-client $(DESTDIR)/usr/bin/tcp-echo-client