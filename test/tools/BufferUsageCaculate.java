import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

/**
 * 
 * Hot to use?
 * 
 *   javac BufferUsageCaculate.java 
 *   
 *   java BufferUsageCaculate
 * 
 * @author ksong
 *
 */
public class BufferUsageCaculate {

	public static void main(String[] args) throws IOException {

		Integer[] r_array = readFromFile("ttcp-r", 6291456);
		
		Integer[] w_array = readFromFile("ttcp-t", 4194304);
		
		System.out.println("Time,TTCP recv buffer usage,TTCP trans buffer usage");
		
		for (int i = 0 ; i < 20 ; i ++) {
			System.out.println((i +1) * 5 + "," + r_array[i] + "," + w_array[i]);
		}
	}

	private static Integer[] readFromFile(String file, Integer max) throws IOException {
		Integer[] r = new Integer[100];
		try (BufferedReader br = new BufferedReader(new FileReader(new File(file)))) {
		    String line = null;
		    int i = 0;
		    while ((line = br.readLine()) != null) {
		    	r[i++] = Integer.parseInt(line);
		    }
		}
		
		Integer[] rs = new Integer[20];
		Integer sum = 0;
		Integer cur = 0;
		for (int i = 0 ; i < 100 ; i ++) {
			sum += r[i];
			if((i + 1) % 5 == 0) {
				sum = sum / 5;
				rs[cur++] = sum;
				sum = 0;
			}
		}
		
		for (int i = 0 ; i < 20 ; i ++) {
			rs[i] = (int)((double)rs[i]/max * 100);
		}
		
		
		
		return rs;
	}

}
