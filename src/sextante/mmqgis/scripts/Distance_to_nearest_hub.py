##source_layer=vector
##hubs_layer=vector
##attribute_in_hubs_layer=field hubs_layer
##result=output vector
##output_type=selection Point;Lines to hub
from sextante.mmqgis import sextante_mmqgis_library as sxt_mmqgis
from sextante.core.Sextante import Sextante
sxt_mmqgis.mmqgis_hub_distance(Sextante.getInterface(), source_layer, hubs_layer, attribute_in_hubs_layer, output_type > 0, False, result)

