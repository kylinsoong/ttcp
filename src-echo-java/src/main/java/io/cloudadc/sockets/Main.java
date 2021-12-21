package io.cloudadc.sockets;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.math.BigInteger;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class Main implements CommandLineRunner {
	
	Logger logger = LoggerFactory.getLogger(getClass());

	public static void main(String[] args) {
		SpringApplication.run(Main.class, args);
	}

	@Override
	public void run(String... args)  throws Exception{
		
		if (args.length >=1) {
			String host = args[0];
			if(args.length > 1) {
				Integer port = Integer.parseInt(args[1]);
				doSend(host, port);
			} else {
				doSend(host);
			}
        } else {
        	int port = 8877;
    		
    		ServerSocket serverSocket = null;
    		try {
    			serverSocket = new ServerSocket(port);
    			System.out.println("Server started: " + serverSocket);
    		} catch (IOException e) {
    			e.printStackTrace();
    		}
    		
    		try {
    			
    			while(true) {
    				Socket socket = null;
    				try {
    					socket = serverSocket.accept();
    					System.out.println("Receive a docket: " + socket);
    				} catch (IOException e) {
    					e.printStackTrace();
    					continue;
    				}
    				
    				while(true) {
    					
    					byte[] buf = null;
    					try {
    						InputStream in = socket.getInputStream();
    						buf = new byte[2048];
    						in.read(buf);
    					} catch (IOException e) {
    						e.printStackTrace();
    						break;
    					}
    					
    					int length = 0;
    					for(int i = 0 ; i < 2048 ; i ++) {
    						if(buf[i] == 0) {
    							break;
    						}
    						length ++;
    						
    					}
    					
    					byte[] rBuf = Arrays.copyOf(buf, length);
    					if(length == 0) {
    						break;
    					} 
    					
    					System.out.println(new Entity(socket) + " received bytes, length: [" + rBuf.length + "], data: " + new BigInteger(rBuf).toString());
    					
    				}
    			}
    		}  finally {
    			if(serverSocket != null) {
    				serverSocket.close();
    			}
    		}
        }
		

        
	}

	private void doSend(String host, Integer port) throws Exception {

		Socket socket = new Socket(host, port);
		OutputStream out = socket.getOutputStream();
		BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
		while(true) {
			String data = br.readLine();
			byte[] sends = data.getBytes();
			out.write(sends);
			out.flush();
			System.out.println(new Entity(socket) + " send bytes, length: [" + sends.length + "], data: " + new BigInteger(sends).toString());
		}
	}

	private void doSend(String host) throws Exception {
		
		Socket socket = new Socket(host, 8877);
		OutputStream out = socket.getOutputStream();
		BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
		while(true) {
			String data = br.readLine();
			byte[] sends = data.getBytes();
			out.write(sends);
			out.flush();
			System.out.println(new Entity(socket) + " send bytes, length: [" + sends.length + "], data: " + new BigInteger(sends).toString());
		}
	}
	

}
