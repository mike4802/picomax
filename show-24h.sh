#!/bin/bash

rrdtool graph office-24h.png \
  --start end-24h \
  --end now \
  --title "Temperature - Last 24 Hours" \
  --vertical-label "°C" \
  --width 800 \
  --height 300 \
  DEF:temp=office.rrd:temp:AVERAGE \
  LINE2:temp#0077CC:"Temperature"
