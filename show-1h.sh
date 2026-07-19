rrdtool graph office-1h.png \
  --start end-1h \
  --end now \
  --title "Temperature - Last Hour" \
  --vertical-label "°C" \
  --width 800 \
  --height 300 \
  DEF:temp=office.rrd:temp:AVERAGE \
  LINE2:temp#0077CC:"Temperature"
