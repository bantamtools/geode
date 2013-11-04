"""random module"""

from __future__ import (division,absolute_import)

from geode import *

Sobols = {1:Sobol1d,2:Sobol2d,3:Sobol3d}
def Sobol(box):
  return Sobols[len(box.min)](box)
