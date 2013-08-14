a="new"
echo $a
for file in "c17" "c1908" "c2670" "c3540" "c432" "c499" "c5315" "c6288" "c7552" "c880"
do
    echo $file
    echo "python gatechanger.py -f $file.type"
    python gatechanger.py -f $file.type
    cp $file$a.type $file.type
done
