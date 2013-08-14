rm slackfile.txt
mkdir results
for file in "c17" "c1908" "c2670" "c3540" "c432" "c499" "c5315" "c6288" "c7552" "c880"
do
    echo $file
    echo "time ./distribution.out $file.hgr $file.type contest.timing contest.leakage >& $file.dump"
    time ./distribution.out $file.hgr $file.type contest.timing contest.leakage >& $file.dump
    mv slackfile.txt $file.slackfile
    mv slack.dat $file.dat
    cp histograms.gnu $file.gnu
    sed -i "s/graph/$file/g" $file.gnu 
    gnuplot $file.gnu
    mkdir results/$file
    cp $file* results/$file
done
