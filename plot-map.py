from __future__ import print_function, unicode_literals
import argparse
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.basemap import Basemap
import geoip2.database
import os

interval = 10

frame_number = 0

def render_file(input_file, output_file, gi):
	f = open(input_file, 'r')
	start_time = None
	lats=[]
	lons=[]
	prevx = []
	prevy = []
	global frame_number
	for ip in f:
		t, ip, v = ip.strip().split()
		t = int(t)
		if start_time == None: start_time = t

		if start_time + interval <= t:

			plt.clf()
			m = Basemap(resolution='l')
			m.drawlsmask(land_color='#f4f4f4', ocean_color='#ffffff')
			x, y = m(lons, lats)
			print('plotting %d points' % len(x))
			m.scatter(prevx, prevy, s=0.07, color='#000000', marker='.', alpha=0.15, edgecolors='none')
			m.scatter(x, y, s=0.07, color='#000000', marker='.', alpha=0.3, edgecolors='none')
			plt.savefig(output_file + ('-%06d.png' % frame_number), dpi=600, bbox_inches='tight')

			frame_number += 1
			start_time += interval
			lats=[]
			lons=[]
			prevx = x
			prevy = y

		try:
			r = gi.city(ip)
		except Exception:
			print("lookup failed: %s" % ip)
			continue
		if None in (r, r.location.latitude, r.location.longitude):
			print("failed to find location for: %s" % ip)
			continue
		lats.append(r.location.latitude)
		lons.append(r.location.longitude)
	f.close()


if len(sys.argv) != 4:
	print('Usage: plot_map <input-file-directory> <output-file-directory> <maxmind-db>\n')
	sys.exit(1)

gi = geoip2.database.Reader(sys.argv[3])

try: os.mkdir(sys.argv[2])
except: pass

for in_file in os.listdir(sys.argv[1]):
	render_file(os.path.join(sys.argv[1], in_file), os.path.join(sys.argv[2], in_file), gi)

