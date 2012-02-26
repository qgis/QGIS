##layer=vector
##hspacing=number 10
##vspacing=number 10
##gridified=output vector
##gridtype=selection Rectangle (line);Rectangle (polygon);Diamond (polygon);Hexagon (polygon)
from sextante.mmqgis import mmqgis_library as mmqgis
from sextante.core.Sextante import Sextante
mmqgis.mmqgis_gridify_layer(Sextante.getInterface(), layer, hspacing, vspacing, gridified, False)

