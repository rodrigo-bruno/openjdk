import java.io.*;
import java.util.*;

class CmdTest {

  private static final Map<String, List<byte[]>> data = new HashMap<String, List<byte[]>>();

  public static void cmd_sleep(String[] input) throws Exception {
    if (input.length < 2) {
      System.out.println("Error: syntax: sleep <# of seconds>");
      return;
    }
    Thread.sleep(Long.parseLong(input[1]) * 1000);
  }

  public static void cmd_alloc(String[] input) throws Exception {
    if (input.length < 4) {
      System.out.println("Error: syntax: alloc <id> <# buffers> <size of buffer>");
      return;
    }

    String id = input[1];
    int nbuf = Integer.parseInt(input[2]);
    int bufsz = Integer.parseInt(input[3]);
    
    if (data.containsKey(id)) {
      System.out.println("Error: id already in used");
      return;
    } else {
      ArrayList<byte[]> buf = new ArrayList<>();
      for (int i = 0; i < nbuf; i++) {
        buf.add(new byte[bufsz]);  
      }
      data.put(id, new ArrayList<>());
    }
  }

  public static void cmd_delete(String[] input) throws Exception {
    if (input.length < 2) {
      System.out.println("Error: syntax: delete <id>");
      return;
    }
    
    String id = input[1];

    if (!data.containsKey(id)) {
      System.out.println("Error: no such id");
      return;
    }

    data.remove(id);
  }

  public static void main(String[] args) throws Exception {
    BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in)); 
    String line; 

    while ((line = stdin.readLine()) != null) {

      if (line.trim().isEmpty()) {
        continue;
      }
 
      String[] input = line.trim().split(" ");
      System.out.println(String.format("Executing %s command...", input[0]));
      switch(input[0]) {
      case "sleep":
        cmd_sleep(input);
        break;
      case "alloc":
        cmd_alloc(input);
        break;
      case "delete":
        cmd_delete(input);
        break;
      case "gc":
        System.gc();
        break;

      default:
        System.out.println("unknown command");
      } 
      System.out.println(String.format("Executing %s command... Done!", input[0]));
    }
  }
}
