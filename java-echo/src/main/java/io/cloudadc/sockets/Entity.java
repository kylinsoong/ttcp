package io.cloudadc.sockets;

import java.net.Socket;

public class Entity {
	
	public Entity(Socket s) {
		setSrc(s.getLocalSocketAddress().toString());
		setDst(s.getRemoteSocketAddress().toString());
	}

	private String src;
	
	private String dst;

	public String getSrc() {
		return src;
	}

	public void setSrc(String src) {
		this.src = src;
	}

	public String getDst() {
		return dst;
	}

	public void setDst(String dst) {
		this.dst = dst;
	}

	@Override
	public String toString() {
		return "[src=" + src + ", dst=" + dst + "]";
	}
	
}
