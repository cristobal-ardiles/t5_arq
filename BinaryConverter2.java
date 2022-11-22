/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
import java.util.Scanner;

/**
 *
 * @author leijurv
 */
public class BinaryConverter2 {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) throws Exception {
        // System.err.println(args.length);
        System.err.println("_start direction: "+args[1]);

        //Scanner scan = new Scanner(new ProcessBuilder("/opt/riscv/bin/riscv32-unknown-elf-objdump", "-s", "/Users/leijurv/Downloads/test").start().getInputStream());
        Scanner scan = new Scanner(System.in);
        scan.nextLine();
        scan.nextLine();
        scan.nextLine();

        System.out.println("v2.0 raw");
        System.out.println("16379*0");
        // The following binary code is generated with make bootstrap2.dump
        System.out.println("0040056f");  //         jal    a0,.next
        System.out.println("00852583");  //  .next: lw     a1,8(a0)
        System.out.println("000580e7");  //         jalr   a1
        System.out.println(args[1]);     //         .word  args[1]
        System.out.println("00000000");  //         .word  0
        int pos = 64*1024;
        while (scan.hasNextLine()) {
            String line = scan.nextLine();
            // System.err.println(line);
            if (line.startsWith("Conten")) {
                if (line.contains(".comment:") ||
                    line.contains(".riscv.attributes:") ) {
                    break;
                }
                continue;
            }
            int linePos = Integer.parseInt(line.substring(0, 7).trim(), 16);
            if (linePos - pos != 0) {
                if (linePos-pos == 4) {
                  System.out.println("00000000");
                  // System.err.println("00000000");
                }
                else {
                  System.out.println((linePos - pos) / 4 + "*0");
                  // System.err.println((linePos - pos) / 4 + "*0");
                }
                pos = linePos;
            }
            String restOfLine = line.substring(7, 44).trim();
            String[] parts = restOfLine.split(" ");
            for (String part : parts) {
                for (int i = part.length() - 2; i >= 0; i -= 2) {
                    System.out.print(part.substring(i, i + 2));
                }
                System.out.print("\n");
            }
            pos += restOfLine.replace(" ", "").length() / 2;
        }
    }

}
