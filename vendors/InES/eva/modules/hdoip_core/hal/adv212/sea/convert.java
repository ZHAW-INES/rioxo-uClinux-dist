import java.io.*;
import java.text.*;
import java.util.*;


public class convert {
  public static BufferedInputStream in;
  public static PrintWriter out;
  public static String filename;

  public static void main(String[] args) throws Exception {
    byte buf[] = new byte[1024];
    int n, size;
    int i32=0, b1, b2, b3, b4;
    boolean b = false;
    boolean swap32 = false, swap16 = false;

    if (args.length > 1)  {
      if (args[1].equals("swap32")) {
        swap32 = true;
        System.out.println("Swap 32 Bit Endian");
      } else if (args[1].equals("swap16")) {
        swap16 = true;
        System.out.println("Swap 16 Bit Endian");
      } else {
        System.out.println("Unknown Parameter");
      }
    }

    if (args.length > 0) {

        System.out.println("Input file: "+args[0]);

        filename = args[0].substring(0, args[0].length()-4);

        in = new BufferedInputStream(new FileInputStream(args[0]));

        out = new PrintWriter(new FileWriter(filename+".c"));

        out.println("const unsigned int "+filename+"[] = {");

        size = 0;
        while ((n = in.read(buf, 0, 1024)) > 0) {
          if ((n%4)!=0) System.out.println("padding to a multiple of 4 bytes");

          for (int i = 0; i < n; i += 4) {
            b1 = ((int)(buf[i+0])) & 0x00ff;
            b2 = ((int)(buf[i+1])) & 0x00ff;
            b3 = ((int)(buf[i+2])) & 0x00ff;
            b4 = ((int)(buf[i+3])) & 0x00ff;

            if (swap32) {
              i32 = (b1<<24) | (b2<<16) | (b3<<8) | (b4);
            } else if (swap16) {
              i32 = (b3<<24) | (b4<<16) | (b1<<8) | (b2);
            } else {
              i32 = (b4<<24) | (b3<<16) | (b2<<8) | (b1);
            }

            if (b) {
              if ((i%64)==0) out.println(",");
              else out.print(",");
            }

            out.print("0x"+Integer.toHexString(i32));
            b = true;
          }

          size += n;

        }

        in.close();

        out.println("};");

        out.println("const int size_"+filename+" = "+size+";");

        out.close();

        out = new PrintWriter(new FileWriter(filename+".h"));
          out.println("#ifndef "+filename.toUpperCase()+"_H");
          out.println("#define "+filename.toUpperCase()+"_H");
          out.println("extern const unsigned int "+filename+"[];");
          out.println("extern const int size_"+filename+";");
          out.println("#endif");
        out.close();

        System.out.println("converted "+size+" Byte");

    } else System.out.println("Use: java convert <file>");

  }

}