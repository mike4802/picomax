#1-minute data for the last 48 hours
#15-minute data for the last 7 days
#1-hour data for the last 30 days
#4-hour data for the last 365 days

rrdtool create office.rrd \
  --step 10 \
  DS:temp:GAUGE:20:U:U \
  \
  RRA:AVERAGE:0.5:6:2880 \
  RRA:MIN:0.5:6:2880 \
  RRA:MAX:0.5:6:2880 \
  \
  RRA:AVERAGE:0.5:90:672 \
  RRA:MIN:0.5:90:672 \
  RRA:MAX:0.5:90:672 \
  \
  RRA:AVERAGE:0.5:360:720 \
  RRA:MIN:0.5:360:720 \
  RRA:MAX:0.5:360:720 \
  \
  RRA:AVERAGE:0.5:1440:2190 \
  RRA:MIN:0.5:1440:2190 \
  RRA:MAX:0.5:1440:2190
