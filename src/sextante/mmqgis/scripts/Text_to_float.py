##layer=vector
##output=output vector
##f=field layer
from sextante.mmqgis import mmqgis_library as mmqgis
from sextante.mmqgis.DummyInterface import DummyInterface
mmqgis.mmqgis_text_to_float(DummyInterface(), layer, [f], output, False)