##hspacing=number 10
##vspacing=number 10
##width=number 100
##height=number 100
##originx=number 0
##originy=number 0
##grid=output vector
##gridtype=selection Rectangle (line);Rectangle (polygon);Diamond (polygon);Hexagon (polygon)
from sextante.mmqgis import mmqgis_library as mmqgis
from sextante.core.Sextante import Sextante
from sextante.mmqgis.DummyInterface import DummyInterface
type_strings = ["Rectangle (line)","Rectangle (polygon)","Diamond (polygon)","Hexagon (polygon)"]
out_type = type_strings[gridtype]
mmqgis.mmqgis_grid(DummyInterface(), grid, hspacing, vspacing, width, height, originx, originy, out_type)

