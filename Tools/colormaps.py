#!/usr/bin/env python3

import matplotlib.cm as cm
import numpy as np

as_float = False

for name, cmap in cm.cmaps_listed.items():

    if name.endswith('_r'):
        continue

    if as_float:
        table = np.array(cmap.colors)
        np.savetxt(name + '.cmap', table, fmt='%f')
    else:
        table = (np.array(cmap.colors) * 256).astype(int)
        np.savetxt(name + '.cmap', table, fmt='%d')
