##source_layer=vector
##hubs_layer=vector
##attribute_in_hubs_layer=number 10
##result=output vector
##output_type=selection Point;Lines to hub
from sextante.mmqgis import mmqgis_library as mmqgis
from sextante.core.Sextante import Sextante
def mmqgis_hub_distance(qgis, sourcename, destname, nameattributename, addlines, addlayer, savename):
mmqgis.mmqgis_hub_distance(Sextante.getInterface(), source_layer, hubs_layer, attribute_in_hubs_layer, output_type > 0, False, result)

