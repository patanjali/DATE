reset
set term png truecolor
set output "graph.png"
set xlabel "Slack"
set ylabel "Num.gates/slack"
set grid
set boxwidth 0.95 relative
set style fill transparent solid 1 noborder
plot "graph.dat" u 1:2 w boxes lc rgb "green" notitle
