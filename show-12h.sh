rrdtool graph office-12h.png \
  --start end-12h \
  --end now \
  --title "Temperature - Last 12 Hours" \
  --vertical-label "°C" \
  --width 800 \
  --height 300 \
  DEF:temp=office.rrd:temp:AVERAGE \
  LINE2:temp#0077CC:"Temperature"
