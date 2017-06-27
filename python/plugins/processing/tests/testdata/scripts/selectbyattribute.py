##Select by attribute=name
##Tests=group

#inputs

##INPUT_LAYER=vector
##OUTPUT_LAYER=vectorOut

#outputs

##OUTPUT_LAYER=output outputVector

import processing

result = processing.run("qgis:selectbyattribute",
                        {'INPUT': INPUT_LAYER,
                         'FIELD': "id2",
                         'OPERATOR': 0,
                         'VALUE': "2"}, context=context, feedback=feedback)
result = processing.run("qgis:saveselectedfeatures",
                        {'INPUT': result["OUTPUT"],
                         'OUTPUT': parameters['OUTPUT_LAYER']}, context=context, feedback=feedback)
OUTPUT_LAYER = result['OUTPUT']
