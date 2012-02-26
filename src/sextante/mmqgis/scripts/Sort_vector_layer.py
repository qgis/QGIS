##layer=vector
##attribute=field layer
##output=output vector
##direction=selection Ascending;Descending
from sextante.mmqgis import mmqgis_library as mmqgis
from sextante.mmqgis.DummyInterface import DummyInterface
direction_strings = ["Ascending", "Dscending"]
mmqgis.mmqgis_sort(DummyInterface(), layer, attribute, output, direction_Strings[direction])

