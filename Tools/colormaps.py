#!/usr/bin/env python3

import matplotlib.cm as cm
import numpy as np


for name, cmap in cm.cmaps_listed.items():

	if name.endswith('_r'):
		continue

	table = (np.array(cmap.colors) * 256).astype(int)
	np.savetxt(name + '.cmap', table, fmt='%d')
