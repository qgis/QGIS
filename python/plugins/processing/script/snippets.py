##Iterate over the features of a layer.
feats = processing.features(layer)
n = len(feats)
for i, feat in enumerate(feats):
    progress.setPercentage(int(100 * i / n))
    #do something with 'feat'

##Create a new layer from another one, with an extra field
fields = processing.fields(layer)
# int, float and bool can be used as well as types
fields.append(('NEW_FIELD', str))
writer = processing.VectorWriter(output_file, None, fields,
							processing.geomtype(layer), layer.crs())

##Create a new table
writer = processing.TableWriter(output_file, None,  ['field1', 'field2'])