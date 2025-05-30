# The following has been generated automatically from src/core/qgsdiagramrenderer.h
QgsDiagramLayerSettings.AroundPoint = QgsDiagramLayerSettings.Placement.AroundPoint
QgsDiagramLayerSettings.OverPoint = QgsDiagramLayerSettings.Placement.OverPoint
QgsDiagramLayerSettings.Line = QgsDiagramLayerSettings.Placement.Line
QgsDiagramLayerSettings.Curved = QgsDiagramLayerSettings.Placement.Curved
QgsDiagramLayerSettings.Horizontal = QgsDiagramLayerSettings.Placement.Horizontal
QgsDiagramLayerSettings.Free = QgsDiagramLayerSettings.Placement.Free
QgsDiagramLayerSettings.OnLine = QgsDiagramLayerSettings.LinePlacementFlag.OnLine
QgsDiagramLayerSettings.AboveLine = QgsDiagramLayerSettings.LinePlacementFlag.AboveLine
QgsDiagramLayerSettings.BelowLine = QgsDiagramLayerSettings.LinePlacementFlag.BelowLine
QgsDiagramLayerSettings.MapOrientation = QgsDiagramLayerSettings.LinePlacementFlag.MapOrientation
QgsDiagramLayerSettings.LinePlacementFlags = lambda flags=0: QgsDiagramLayerSettings.LinePlacementFlag(flags)
# monkey patching scoped based enum
QgsDiagramLayerSettings.BackgroundColor = QgsDiagramLayerSettings.Property.BackgroundColor
QgsDiagramLayerSettings.BackgroundColor.is_monkey_patched = True
QgsDiagramLayerSettings.BackgroundColor.__doc__ = "Diagram background color"
QgsDiagramLayerSettings.StrokeColor = QgsDiagramLayerSettings.Property.StrokeColor
QgsDiagramLayerSettings.StrokeColor.is_monkey_patched = True
QgsDiagramLayerSettings.StrokeColor.__doc__ = "Stroke color"
QgsDiagramLayerSettings.StrokeWidth = QgsDiagramLayerSettings.Property.StrokeWidth
QgsDiagramLayerSettings.StrokeWidth.is_monkey_patched = True
QgsDiagramLayerSettings.StrokeWidth.__doc__ = "Stroke width"
QgsDiagramLayerSettings.PositionX = QgsDiagramLayerSettings.Property.PositionX
QgsDiagramLayerSettings.PositionX.is_monkey_patched = True
QgsDiagramLayerSettings.PositionX.__doc__ = "X-coordinate data defined diagram position"
QgsDiagramLayerSettings.PositionY = QgsDiagramLayerSettings.Property.PositionY
QgsDiagramLayerSettings.PositionY.is_monkey_patched = True
QgsDiagramLayerSettings.PositionY.__doc__ = "Y-coordinate data defined diagram position"
QgsDiagramLayerSettings.Distance = QgsDiagramLayerSettings.Property.Distance
QgsDiagramLayerSettings.Distance.is_monkey_patched = True
QgsDiagramLayerSettings.Distance.__doc__ = "Distance to diagram from feature"
QgsDiagramLayerSettings.Priority = QgsDiagramLayerSettings.Property.Priority
QgsDiagramLayerSettings.Priority.is_monkey_patched = True
QgsDiagramLayerSettings.Priority.__doc__ = "Diagram priority (between 0 and 10)"
QgsDiagramLayerSettings.ZIndex = QgsDiagramLayerSettings.Property.ZIndex
QgsDiagramLayerSettings.ZIndex.is_monkey_patched = True
QgsDiagramLayerSettings.ZIndex.__doc__ = "Z-index for diagram ordering"
QgsDiagramLayerSettings.IsObstacle = QgsDiagramLayerSettings.Property.IsObstacle
QgsDiagramLayerSettings.IsObstacle.is_monkey_patched = True
QgsDiagramLayerSettings.IsObstacle.__doc__ = "Whether diagram features act as obstacles for other diagrams/labels"
QgsDiagramLayerSettings.Show = QgsDiagramLayerSettings.Property.Show
QgsDiagramLayerSettings.Show.is_monkey_patched = True
QgsDiagramLayerSettings.Show.__doc__ = "Whether to show the diagram"
QgsDiagramLayerSettings.AlwaysShow = QgsDiagramLayerSettings.Property.AlwaysShow
QgsDiagramLayerSettings.AlwaysShow.is_monkey_patched = True
QgsDiagramLayerSettings.AlwaysShow.__doc__ = "Whether the diagram should always be shown, even if it overlaps other diagrams/labels"
QgsDiagramLayerSettings.StartAngle = QgsDiagramLayerSettings.Property.StartAngle
QgsDiagramLayerSettings.StartAngle.is_monkey_patched = True
QgsDiagramLayerSettings.StartAngle.__doc__ = "Angle offset for pie diagram"
QgsDiagramLayerSettings.Property.__doc__ = """Data definable properties.

* ``BackgroundColor``: Diagram background color
* ``StrokeColor``: Stroke color
* ``StrokeWidth``: Stroke width
* ``PositionX``: X-coordinate data defined diagram position
* ``PositionY``: Y-coordinate data defined diagram position
* ``Distance``: Distance to diagram from feature
* ``Priority``: Diagram priority (between 0 and 10)
* ``ZIndex``: Z-index for diagram ordering
* ``IsObstacle``: Whether diagram features act as obstacles for other diagrams/labels
* ``Show``: Whether to show the diagram
* ``AlwaysShow``: Whether the diagram should always be shown, even if it overlaps other diagrams/labels
* ``StartAngle``: Angle offset for pie diagram

"""
# --
QgsDiagramLayerSettings.Single = QgsDiagramLayerSettings.DiagramType.Single
QgsDiagramLayerSettings.Stacked = QgsDiagramLayerSettings.DiagramType.Stacked
QgsDiagramSettings.Height = QgsDiagramSettings.LabelPlacementMethod.Height
QgsDiagramSettings.XHeight = QgsDiagramSettings.LabelPlacementMethod.XHeight
QgsDiagramSettings.Up = QgsDiagramSettings.DiagramOrientation.Up
QgsDiagramSettings.Down = QgsDiagramSettings.DiagramOrientation.Down
QgsDiagramSettings.Left = QgsDiagramSettings.DiagramOrientation.Left
QgsDiagramSettings.Right = QgsDiagramSettings.DiagramOrientation.Right
QgsDiagramSettings.Clockwise = QgsDiagramSettings.Direction.Clockwise
QgsDiagramSettings.Counterclockwise = QgsDiagramSettings.Direction.Counterclockwise
QgsDiagramSettings.Horizontal = QgsDiagramSettings.StackedDiagramMode.Horizontal
QgsDiagramSettings.Vertical = QgsDiagramSettings.StackedDiagramMode.Vertical
try:
    QgsDiagramSettings.__attribute_docs__ = {'sizeType': 'Diagram size unit', 'sizeScale': 'Diagram size unit scale', 'lineSizeUnit': 'Line unit index', 'lineSizeScale': 'Line unit scale', 'opacity': 'Opacity, from 0 (transparent) to 1.0 (opaque)', 'rotationOffset': 'Rotation offset, in degrees clockwise from horizontal.', 'maximumScale': 'The maximum map scale (i.e. most "zoomed in" scale) at which the diagrams will be visible.\nThe scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.\nA scale of 0 indicates no maximum scale visibility.\n\n.. seealso:: :py:func:`minimumScale`', 'minimumScale': 'The minimum map scale (i.e. most "zoomed out" scale) at which the diagrams will be visible.\nThe scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.\nA scale of 0 indicates no minimum scale visibility.\n\n.. seealso:: :py:func:`maximumScale`', 'minimumSize': 'Scale diagrams smaller than mMinimumSize to mMinimumSize'}
    QgsDiagramSettings.__annotations__ = {'sizeType': 'Qgis.RenderUnit', 'sizeScale': 'QgsMapUnitScale', 'lineSizeUnit': 'Qgis.RenderUnit', 'lineSizeScale': 'QgsMapUnitScale', 'opacity': float, 'rotationOffset': float, 'maximumScale': float, 'minimumScale': float, 'minimumSize': float}
except (NameError, AttributeError):
    pass
try:
    QgsDiagramInterpolationSettings.__attribute_docs__ = {'classificationField': 'Name of the field for classification'}
    QgsDiagramInterpolationSettings.__annotations__ = {'classificationField': str}
except (NameError, AttributeError):
    pass
try:
    QgsDiagramRenderer.dpiPaintDevice = staticmethod(QgsDiagramRenderer.dpiPaintDevice)
    QgsDiagramRenderer.__virtual_methods__ = ['sizeMapUnits', 'referencedFields', 'renderDiagram', 'legendItems']
    QgsDiagramRenderer.__abstract_methods__ = ['clone', 'rendererName', 'diagramAttributes', 'diagramSettings', 'readXml', 'writeXml', 'diagramSize']
except (NameError, AttributeError):
    pass
try:
    QgsSingleCategoryDiagramRenderer.__overridden_methods__ = ['clone', 'rendererName', 'diagramAttributes', 'diagramSettings', 'readXml', 'writeXml', 'legendItems', 'diagramSize']
except (NameError, AttributeError):
    pass
try:
    QgsLinearlyInterpolatedDiagramRenderer.__overridden_methods__ = ['clone', 'diagramSettings', 'diagramAttributes', 'referencedFields', 'rendererName', 'readXml', 'writeXml', 'legendItems', 'diagramSize']
except (NameError, AttributeError):
    pass
try:
    QgsStackedDiagramRenderer.__overridden_methods__ = ['clone', 'sizeMapUnits', 'renderDiagram', 'diagramSettings', 'diagramAttributes', 'rendererName', 'readXml', 'writeXml', 'legendItems', 'diagramSize']
except (NameError, AttributeError):
    pass
