##Select by attribute=name
##Tests=group
##INPUT_LAYER=vector
##OUTPUT_LAYER=output vector

import processing

result = processing.runalg("qgis:selectbyattribute",
                           INPUT_LAYER,
                           "id2",
                           0,
                           "2")

processing.runalg("qgis:saveselectedfeatures",
                  result["OUTPUT"],
                  OUTPUT_LAYER)
