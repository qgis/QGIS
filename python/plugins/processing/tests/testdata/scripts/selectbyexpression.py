##Select by expression=name
##Tests=group
##INPUT_LAYER=vector
##OUTPUT_LAYER=output vector

import processing

result = processing.runalg("qgis:selectbyexpression",
                           INPUT_LAYER,
                           '"id2" = 0 and "id" > 7',
                           "1")

processing.runalg("qgis:saveselectedfeatures",
                  result["RESULT"],
                  OUTPUT_LAYER)
