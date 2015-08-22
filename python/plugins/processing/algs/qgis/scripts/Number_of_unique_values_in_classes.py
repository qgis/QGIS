##Vector table tools=group
##input=vector
##class_field=field input
##value_field=field input
##N unique values=output vector

layer = processing.getObject(input)
provider = layer.dataProvider()
fields = provider.fields()
fields.append(QgsField('UNIQ_COUNT', QVariant.Int))
writer = processing.VectorWriter(output, None, fields, provider.geometryType(),
                                 layer.crs())

class_field_index = layer.fieldNameIndex(class_field)
value_field_index = layer.fieldNameIndex(value_field)

outFeat = QgsFeature()
classes = {}
feats = processing.features(layer)
nFeat = len(feats)
for n, inFeat in enumerate(feats):
    progress.setPercentage(int(100 * n / nFeat))
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    value = attrs[value_field_index]
    if clazz not in classes:
        classes[clazz] = []
    if value not in classes[clazz]:
        classes[clazz].append(value)

feats = processing.features(layer)
for n, inFeat in enumerate(feats):
    progress.setPercentage(int(100 * n / nFeat))
    inGeom = inFeat.geometry()
    outFeat.setGeometry(inGeom)
    attrs = inFeat.attributes()
    clazz = attrs[class_field_index]
    attrs.append(len(classes[clazz]))
    outFeat.setAttributes(attrs)
    writer.addFeature(outFeat)

del writer
