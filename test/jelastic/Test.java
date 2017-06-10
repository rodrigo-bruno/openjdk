import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.Date;

public class Test {
    private static final boolean debug = true;
    private static final int buf_size = 64*1024;
    private static final int threads = 4;
    private static final int alloc_rate = 8; // buf_size per second per thread
    // Working set size = n_lists * max_lists_size * buf_size;
    private static final int n_lists = 16;
    private static final int max_lists_size = 2048;

    static class T2Runnable implements Runnable {

        private final int tid;
        private final List<List<ByteBuffer>> lists = new ArrayList<>();
        private final Random randGen = new Random();

        public T2Runnable(int tid) {
            this.tid = tid;
        }

        public void run() {
            for (int i = 0; i < n_lists; i++) {
                lists.add(new LinkedList());
            }
        
            for (;;) {
                // Get a random
                int rand = randGen.nextInt(n_lists);
                lists.get(rand).add(ByteBuffer.allocate(buf_size));
                if (debug) { System.out.println(String.format("T%d: Added buffer to %d th list.", tid, rand)); }
                if (lists.get(rand).size() == max_lists_size) {
                    lists.get(rand).clear();
                    if (debug) { System.out.println(String.format("T%d: Cleared %d th list.", tid, rand)); }
                }
                try { Thread.sleep(1000/alloc_rate); } catch (Exception e) { e.printStackTrace(); }
            }

        }
    }

    public static void main(String[] args) throws Exception {
        Thread[] workers = new Thread[threads];
        System.out.println(String.format("Using %d lists of up to %d buffers with %d bytes.", n_lists, max_lists_size, buf_size));
	Date start = new Date();
        // Launch threads
        for (int i = 0; i < threads; i++) {
            workers[i] = new Thread(new T2Runnable(i));
            workers[i].start();
        }
        // Wait for threads
        for (int i = 0; i < threads; i++) {
            workers[i].join();
        }

        Date finish = new Date();
        System.out.println("Finished in " + (finish.getTime() - start.getTime()) + " seconds");
    }
}
