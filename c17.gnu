reset
set term png truecolor
set output "c17.png"
set xlabel "Slack"
set ylabel "Num.gates/slack"
set grid
set boxwidth 0.95 relative
set style fill transparent solid 1 noborder
plot "c17.dat" u 1:2 w boxes lc rgb "green" notitle
