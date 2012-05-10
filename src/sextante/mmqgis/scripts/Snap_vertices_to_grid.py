##layer=vector
##hspacing=number 10
##vspacing=number 10
##gridified=output vector
##gridtype=selection Rectangle (line);Rectangle (polygon);Diamond (polygon);Hexagon (polygon)
from sextante.mmqgis import sextante_mmqgis_library as sxt_mmqgis
from sextante.core.Sextante import Sextante
sxt_mmqgis.mmqgis_gridify_layer(Sextante.getInterface(), layer, hspacing, vspacing, gridified, False)

