##Select by expression=name
##Tests=group

#inputs

##INPUT_LAYER=vector
##OUTPUT_LAYER=vectorDestination


import processing

result = processing.run("qgis:selectbyexpression",
                        {'INPUT': INPUT_LAYER,
                         'EXPRESSION': '"id2" = 0 and "id" > 7',
                         'METHOD': 1}, context=context, feedback=feedback)

result = processing.run("qgis:saveselectedfeatures",
                        {'INPUT': result["OUTPUT"],
                         'OUTPUT': parameters['OUTPUT_LAYER']},
                        context=context, feedback=feedback)
OUTPUT_LAYER = result['OUTPUT']
