#!/usr/bin/env python3

import numpy as np

for n, phase in enumerate(np.linspace(0, np.pi * 2, 100)):

	t = np.linspace(0, 1, 1024)
	y1 = np.sin(t * np.pi * 4 + phase)
	y2 = np.cos(t * np.pi * 4 + phase)

	outf = open("data.{:02d}.dat".format(n), 'w')
	
	for d in zip(t, y1, y2):
		outf.write("{:.10f} {:.10f} {:.10f}\n".format(*d))
