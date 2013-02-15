from sextante.core.QGisLayers import QGisLayers


def getAttributeValues(layer, *attributeNames):
    ret = {}
    for name in attributeNames:
        values = []
        features = QGisLayers.features(layer)
        index = layer.fieldNameIndex(name)
        if index == -1:
            raise ValueError('Wrong field name')
        for feature in features:
            try:
                v = float(feature.attributes()[index].toString())
                values.append(v)
            except:
                values.append(None)
        ret[name] = values;
    return ret
        
 