##layer=vector
##output=output vector
##f=field layer
from sextante.mmqgis import sextante_mmqgis_library as sxt_mmqgis
from sextante.mmqgis.DummyInterface import DummyInterface
sxt_mmqgis.mmqgis_text_to_float(DummyInterface(), layer, [f], output, False)