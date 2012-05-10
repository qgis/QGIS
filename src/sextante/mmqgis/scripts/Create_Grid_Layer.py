##hspacing=number 10
##vspacing=number 10
##width=number 100
##height=number 100
##originx=number 0
##originy=number 0
##grid=output vector
##gridtype=selection Rectangle (line);Rectangle (polygon);Diamond (polygon);Hexagon (polygon)
from sextante.mmqgis import sextante_mmqgis_library as sxt_mmqgis
from sextante.mmqgis.DummyInterface import DummyInterface
type_strings = ["Rectangle (line)","Rectangle (polygon)","Diamond (polygon)","Hexagon (polygon)"]
out_type = type_strings[gridtype]
sxt_mmqgis.mmqgis_grid(DummyInterface(), grid, hspacing, vspacing, width, height, originx, originy, out_type)

