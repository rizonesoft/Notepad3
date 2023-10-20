module jinvertertb;
  reg a;
  wire y;

  //Design Instance
  jinverter jinv(y,a);
  
    initial
    begin
        $display ("RESULT\ta\ty");

        a = 1; # 100; // Another value
        if ( y == 0 ) // Test for inversion
            $display ("  PASS  \t%d\t%d",a,y);
        else
            $display ("  FAIL \t%d\t%d",a,y);

        a = 0; # 100; // Initial value is set
        if ( y == 1 ) // Test for inversion
            $display ("  PASS  \t%d\t%d",a,y);
        else
            $display ("  FAIL  \t%d\t%d",a,y);

        a = 1; # 50; // Another value
        if ( y == 0 ) // Test for inversion
            $display ("  PASS  \t%d\t%d",a,y);
        else
            $display ("  FAIL  \t%d\t%d",a,y);

        a = 0; # 100; // Initial value is set
        if ( y == 1 ) // Test for inversion
            $display ("  PASS  \t%d\t%d",a,y);
        else
            $display ("  FAIL  \t%d\t%d",a,y);

    end
  
  //enabling the wave dump
  initial begin 
    $dumpfile("dump.vcd"); $dumpvars;
  end
endmodule


module jtristatebuffertb;
  reg  a,c;
  wire y;
  
  tristate_buffer jbuf(a,c,y);
  initial begin
        $display ("RESULT\ta\ty");

        a = 0; c = 0; # 100; // Initial value is set
        if ( y === 1'bz ) // Test for inversion
            $display ("PASS\t%d\t%d",a,y);
        else
            $display ("FAIL\t%d\t%d",a,y);

        a = 0; c = 1; # 100; // Initial value is set
        if ( y === 0 ) // Test for inversion
            $display ("PASS\t%d\t%d",a,y);
        else
            $display ("FAIL\t%d\t%d",a,y);

        a = 1; c = 0; # 100; // Another value
        if ( y === 1'bz ) // Test for inversion
            $display ("PASS\t%d\t%d",a,y);
        else
            $display ("FAIL\t%d\t%d",a,y);

        a = 1; c = 1; # 100; // Another value
        if ( y === 1 ) // Test for inversion
            $display ("PASS\t%d\t%d",a,y);
        else
            $display ("FAIL\t%d\t%d",a,y);

    end
  
  initial begin
    //dump waveform
    $dumpfile("dump.vcd"); $dumpvars;
  end
endmodule

module jmsflipfloptb;
    wire q, qbar;
    reg clk,rst;
    reg d;

    wire qn_1,qn_1bar; // to monitor current q and previous q values

    jmsflipflop jmsff(q,qbar,clk,rst,d);
//	jdflipflop  jdff(qn_1,qn_1bar,clk,1'b0,q);
  
    always #5 clk = ~clk;
  
  //enabling the wave dump
  initial begin 
    $dumpfile("dump.vcd"); $dumpvars;
  end

    initial
    begin
        d = 0; // JUST keep it initialized
                clk = 1'b0;
        rst = 1; # 20;
        $display("INITIALIZING");
        $display("RSLT\td\tqn_1\tq");
                $display ("PASS\t%d\t%d\t%d",d,qn_1,q);

        rst = 0; # 20;
                $display ("PASS\t%d\t%d\t%d",d,qn_1,q);

        $display("\nTESTING");
        $display("RSLT\td\tqn_1\tq");
                d = 0; # 10; // Another value
                if ( q === 1'b0 ) // Test for inversion
                        $display ("PASS\t%d\t%d\t%d",d,qn_1,q);
                else
                        $display ("FAIL\t%d\t%d\t%d",d,qn_1,q);
        
                d = 1; # 10; // Another value
                if ( q === 1'b0 ) // Test for inversion
                        $display ("PASS\t%d\t%d\t%d",d,qn_1,q);
                else
                        $display ("FAIL\t%d\t%d\t%d",d,qn_1,q);
    
                # 5; // Another value
                if ( q === 1'b1 ) // Test for inversion
                        $display ("PASS\t%d\t%d\t%d",d,qn_1,q);
                else
                        $display ("FAIL\t%d\t%d\t%d",d,qn_1,q);
    
                d = 0; # 5; // Another value
                if ( q === 1'b1 ) // Test for inversion
                        $display ("PASS\t%d\t%d\t%d",d,qn_1,q);
                else
                        $display ("FAIL\t%d\t%d\t%d",d,qn_1,q);
    
                d = 1; # 5; // JUST wait
                if ( q === 1'b0 ) // Test for inversion
                        $display ("PASS\t%d\t%d\t%d",d,qn_1,q);
                else
                        $display ("FAIL\t%d\t%d\t%d",d,qn_1,q);
    
        $finish;	
    end
endmodule
