# The following has been generated automatically from src/core/elevation/qgsabstractprofilegenerator.h
try:
    QgsProfileIdentifyContext.__attribute_docs__ = {'maximumSurfaceDistanceDelta': 'Maximum allowed snapping delta for the distance values when identifying a continuous elevation surface', 'maximumSurfaceElevationDelta': 'Maximum allowed snapping delta for the elevation values when identifying a continuous elevation surface', 'maximumPointDistanceDelta': 'Maximum allowed snapping delta for the distance values when identifying a point', 'maximumPointElevationDelta': 'Maximum allowed snapping delta for the elevation values when identifying a point', 'displayRatioElevationVsDistance': 'Display ratio of elevation vs distance units', 'project': 'Associated project.'}
    QgsProfileIdentifyContext.__annotations__ = {'maximumSurfaceDistanceDelta': float, 'maximumSurfaceElevationDelta': float, 'maximumPointDistanceDelta': float, 'maximumPointElevationDelta': float, 'displayRatioElevationVsDistance': float, 'project': 'QgsProject'}
    QgsProfileIdentifyContext.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractProfileResults.Feature.__attribute_docs__ = {'layerIdentifier': 'Identifier for grouping output features', 'geometry': 'Exported geometry', 'attributes': 'Exported attributes'}
    QgsAbstractProfileResults.Feature.__annotations__ = {'layerIdentifier': str, 'geometry': 'QgsGeometry', 'attributes': 'Dict[str, object]'}
    QgsAbstractProfileResults.Feature.__doc__ = """Encapsulates information about a feature exported from the profile results.

.. versionadded:: 3.32"""
    QgsAbstractProfileResults.Feature.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractProfileResults.__virtual_methods__ = ['asFeatures', 'snapPoint', 'identify', 'copyPropertiesFromGenerator']
    QgsAbstractProfileResults.__abstract_methods__ = ['type', 'distanceToHeightMap', 'sampledPoints', 'asGeometries', 'renderResults', 'zRange']
    QgsAbstractProfileResults.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractProfileGenerator.__virtual_methods__ = ['flags']
    QgsAbstractProfileGenerator.__abstract_methods__ = ['sourceId', 'generateProfile', 'feedback', 'takeResults']
    QgsAbstractProfileGenerator.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsProfileRenderContext.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsProfileIdentifyResults.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsProfileGenerationContext.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
