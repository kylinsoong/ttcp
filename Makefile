build:
	gcc chat/chat.c -o chat/chat
	gcc echoS/echoS.c -o echoS/echoS
	gcc ttcp/ttcp.c -o ttcp/ttcp
	gcc bancs/bancs.c -o bancs/bancs
clean:
	rm -f ttcp/ttcp 
	rm -f chat/chat
	rm -f echoS/echoS
	rm -f bancs/bancs
install:
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 ttcp/ttcp $(DESTDIR)/usr/bin/ttcp
	install -m 0755 chat/chat $(DESTDIR)/usr/bin/chat
	install -m 0755 echoS/echoS $(DESTDIR)/usr/bin/echoS
	install -m 0755 bancs/bancs $(DESTDIR)/usr/bin/bancs
	mkdir -p $(DESTDIR)/etc
	install -m 0755 ttcp.conf $(DESTDIR)/etc/ttcp.conf
	install -m 0755 bancs.data $(DESTDIR)/etc/bancs.data
