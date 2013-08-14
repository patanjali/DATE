for file in "c17" "c1908" "c2670" "c3540" "c432" "c499" "c5315" "c6288" "c7552" "c880a"
do
    echo $file
    echo "./parser_helper.out verilog $file.v"
    ./parser_helper.out verilog $file.v
done
