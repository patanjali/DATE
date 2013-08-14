reset
set term png truecolor
set output "c6288.png"
set xlabel "Slack"
set ylabel "Num.gates/slack"
set grid
set boxwidth 0.95 relative
set style fill transparent solid 1 noborder
plot "c6288.dat" u 1:2 w boxes lc rgb "green" notitle
