build:
	gcc chat/chat.c -o chat/chat
	gcc echoS/echoS.c -o echoS/echoS
	gcc ttcp/ttcp.c -o ttcp/ttcp
clean:
	rm -f ttcp/ttcp 
	rm -f chat/chat
	rm -f echoS/echoS
install:
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 ttcp/ttcp $(DESTDIR)/usr/bin/ttcp
	install -m 0755 chat/chat $(DESTDIR)/usr/bin/chat
	install -m 0755 echoS/echoS $(DESTDIR)/usr/bin/echoS
	mkdir -p $(DESTDIR)/etc
	install -m 0755 ttcp.conf $(DESTDIR)/etc/ttcp.conf
