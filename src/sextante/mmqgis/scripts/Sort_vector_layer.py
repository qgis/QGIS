##layer=vector
##attribute=field layer
##output=output vector
##direction=selection Ascending;Descending
from sextante.mmqgis import sextante_mmqgis_library as sxt_mmqgis
from sextante.mmqgis.DummyInterface import DummyInterface
direction_strings = ["Ascending", "Dscending"]
sxt_mmqgis.mmqgis_sort(DummyInterface(), layer, attribute, output, direction_Strings[direction])

