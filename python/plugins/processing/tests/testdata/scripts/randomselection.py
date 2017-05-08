##Random Selection=name
##Tests=group
##INPUT_LAYER=vector
##number=Number
##OUTPUT_LAYER=output vector

import processing

result = processing.runalg("qgis:randomselection",
                           INPUT_LAYER,
                           "0",
                           number)

processing.runalg("qgis:saveselectedfeatures",
                  result["OUTPUT"],
                  OUTPUT_LAYER)
