# The following has been generated automatically from src/core/labeling/qgslabelobstaclesettings.h
# monkey patching scoped based enum
QgsLabelObstacleSettings.PolygonInterior = QgsLabelObstacleSettings.ObstacleType.PolygonInterior
QgsLabelObstacleSettings.PolygonInterior.is_monkey_patched = True
QgsLabelObstacleSettings.PolygonInterior.__doc__ = "Avoid placing labels over interior of polygon (prefer placing labels totally outside or just slightly inside polygon)"
QgsLabelObstacleSettings.PolygonBoundary = QgsLabelObstacleSettings.ObstacleType.PolygonBoundary
QgsLabelObstacleSettings.PolygonBoundary.is_monkey_patched = True
QgsLabelObstacleSettings.PolygonBoundary.__doc__ = "Avoid placing labels over boundary of polygon (prefer placing outside or completely inside polygon)"
QgsLabelObstacleSettings.PolygonWhole = QgsLabelObstacleSettings.ObstacleType.PolygonWhole
QgsLabelObstacleSettings.PolygonWhole.is_monkey_patched = True
QgsLabelObstacleSettings.PolygonWhole.__doc__ = "Avoid placing labels over ANY part of polygon. Where PolygonInterior will prefer to place labels with the smallest area of intersection between the label and the polygon, PolygonWhole will penalise any label which intersects with the polygon by an equal amount, so that placing labels over any part of the polygon is avoided"
QgsLabelObstacleSettings.ObstacleType.__doc__ = """Valid obstacle types, which affect how features within the layer will act as obstacles
for labels.

* ``PolygonInterior``: Avoid placing labels over interior of polygon (prefer placing labels totally outside or just slightly inside polygon)
* ``PolygonBoundary``: Avoid placing labels over boundary of polygon (prefer placing outside or completely inside polygon)
* ``PolygonWhole``: Avoid placing labels over ANY part of polygon. Where PolygonInterior will prefer to place labels with the smallest area of intersection between the label and the polygon, PolygonWhole will penalise any label which intersects with the polygon by an equal amount, so that placing labels over any part of the polygon is avoided

"""
# --
try:
    QgsLabelObstacleSettings.__group__ = ['labeling']
except (NameError, AttributeError):
    pass
