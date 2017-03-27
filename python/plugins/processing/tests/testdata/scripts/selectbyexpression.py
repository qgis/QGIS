##Select by expression=name
##Tests=group
##INPUT_LAYER=vector
##OUTPUT_LAYER=output vector

import processing

result = processing.run("qgis:selectbyexpression",
                           INPUT_LAYER,
                           '"id2" = 0 and "id" > 7',
                           "1")

processing.run("qgis:saveselectedfeatures",
                  result["RESULT"],
                  OUTPUT_LAYER)
