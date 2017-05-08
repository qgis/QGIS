##Random selection within subset=name
##Tests=group
##INPUT_LAYER=vector
##OUTPUT_LAYER=output vector

import processing

result = processing.runalg("qgis:randomselectionwithinsubsets",
                           INPUT_LAYER,
                           "id2",
                           0,
                           "1")

processing.runalg("qgis:saveselectedfeatures",
                  result["OUTPUT"],
                  OUTPUT_LAYER)
