def mapping_feature(feature):
    geom = feature.geometry()
    fields = [field.name() for field in feature.fields()]
    properties = dict(list(zip(fields, feature.attributes())))
    return {'type': 'Feature',
            'properties': properties,
            'geometry': geom.__geo_interface__}
