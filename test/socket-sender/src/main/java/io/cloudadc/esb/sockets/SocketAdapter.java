package io.cloudadc.esb.sockets;

import java.io.DataOutputStream;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketAddress;

import java.util.Random;
import java.util.UUID;

import org.apache.commons.lang3.RandomStringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class SocketAdapter implements CommandLineRunner {
	
	
	Logger log = LoggerFactory.getLogger(SocketAdapter.class);

	public static void main(String[] args) {
		SpringApplication.run(SocketAdapter.class, args);
	}

	@Override
	public void run(String... args) throws Exception {
		
		
		String host = "";
		int port = 0;	
		int count = 0;
		boolean isServer = false ;

        for(int i = 0 ; i < args.length ; i++) {
			
			if(args[i].equals("--host")  || args[i].equals("-h") ) {
				host = args[++i];
			} else if (args[i].equals("--port")  || args[i].equals("-p") ) {
				port = Integer.parseInt(args[++i]);
			} else if (args[i].equals("--count")  || args[i].equals("-c") ) {
				count = Integer.parseInt(args[++i]);
			} else if (args[i].equals("--server")  || args[i].equals("-s") ) {
				isServer = true;
			} else if (args[i].equals("--help")) {
				help();
			}
		}
        
        if(count < 3) {
			count = 3;
		}
				
		if(isServer) {
			serverimpl(count);
		}
		
		if(host.equals("") || port == 0) {
			help();
		}
		
		
		
		SocketAddress address = new InetSocketAddress(host, port);
		Socket socket = new Socket();
		socket.connect(address, 2000);
		
		log.info("connect to " + address);
		
		DataOutputStream out = new DataOutputStream(socket.getOutputStream());
		
		Random ran = new Random();
		
		for(int i = 1 ; i <= count ; i ++) {
			
			int payloadsize = ran.nextInt(4000) + 1000;
			
			if(i % 8 == 0) {
				payloadsize = 201535;
			} else if (i % 9 == 0 ) {
				payloadsize = 19371;
			} else if (i % 10 == 0) {
				payloadsize = 2469;
			}
			
			String prefix = UUID.randomUUID() + "-";
			String trim = "</response>";
			String payload = prefix + RandomStringUtils.random(payloadsize - prefix.length() - trim.length(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRST00112233445566778899<>") + trim;
			
			if(payloadsize == 201535) {
				char[] array = payload.toCharArray();
				for (int c = 91535 ; c < 190000 ; c ++) {
					array[c] = ' ';
				}
				payload = new String(array);
			}
			
			
			String header = buildHeader(payload.length());
			
			String msg = header + payload;
			
			out.write(msg.getBytes());
			log.info("write to sockets, message length: " + msg.length() + ", payload length: " + payload.length() + ", header: " + header);
		}
		
		socket.close();
		
		log.info(address + " closed");

	}

    private void serverimpl(int count) throws Exception {
		
		ServerSocket serverSocket = new ServerSocket(12345);
		log.info("server socket started on: " + serverSocket);
		
		Socket socket = serverSocket.accept();
		log.info("Received a socket: " + socket);
		
		InputStream in = socket.getInputStream();
		
		int c = 0;
		
		while(true) {
			byte[] bufHeader = in.readNBytes(9);
			String header = new String(bufHeader);
			Integer payloadLen = Integer.parseInt(trimzero(header));
			byte[] bufpayload = in.readNBytes(payloadLen);
			String payload = new String(bufpayload);
			log.info("Received message, header: " + header + ", payload length: " + payloadLen + ", total length: " + (header.length() + payload.length()) + ", message: " + header + payload.substring(0, 50));
		    
			if(++c == count) {
		    	break;
		    } 
		}
		
		serverSocket.close();
		
		Runtime.getRuntime().exit(0);
	}
	
	private String trimzero(String header) {
		
		if(header.startsWith("0")) {
			trimzero(header.substring(1));
		}
		return header;
	}

	private String buildHeader(int number) {
		
		String str = String.valueOf(number);
		
		String result = "";
		
		for (int i = 0 ; i < 9 - str.length() ; i ++) {
			result += "0";
		}
		
		return result + str;
	}

	private void help() {

		System.out.println("Run ");
		System.out.println("  java -jar esb-socket-sender.jar --host <Host> --port <Port> ");			
		System.exit(0);
	}

	
}
