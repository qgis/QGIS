# The following has been generated automatically from src/core/callouts/qgscallout.h
# monkey patching scoped based enum
QgsCallout.MinimumCalloutLength = QgsCallout.Property.MinimumCalloutLength
QgsCallout.MinimumCalloutLength.is_monkey_patched = True
QgsCallout.MinimumCalloutLength.__doc__ = "Minimum length of callouts"
QgsCallout.OffsetFromAnchor = QgsCallout.Property.OffsetFromAnchor
QgsCallout.OffsetFromAnchor.is_monkey_patched = True
QgsCallout.OffsetFromAnchor.__doc__ = "Distance to offset lines from anchor points"
QgsCallout.OffsetFromLabel = QgsCallout.Property.OffsetFromLabel
QgsCallout.OffsetFromLabel.is_monkey_patched = True
QgsCallout.OffsetFromLabel.__doc__ = "Distance to offset lines from label area"
QgsCallout.DrawCalloutToAllParts = QgsCallout.Property.DrawCalloutToAllParts
QgsCallout.DrawCalloutToAllParts.is_monkey_patched = True
QgsCallout.DrawCalloutToAllParts.__doc__ = "Whether callout lines should be drawn to all feature parts"
QgsCallout.AnchorPointPosition = QgsCallout.Property.AnchorPointPosition
QgsCallout.AnchorPointPosition.is_monkey_patched = True
QgsCallout.AnchorPointPosition.__doc__ = "Feature's anchor point position"
QgsCallout.LabelAnchorPointPosition = QgsCallout.Property.LabelAnchorPointPosition
QgsCallout.LabelAnchorPointPosition.is_monkey_patched = True
QgsCallout.LabelAnchorPointPosition.__doc__ = "Label's anchor point position"
QgsCallout.OriginX = QgsCallout.Property.OriginX
QgsCallout.OriginX.is_monkey_patched = True
QgsCallout.OriginX.__doc__ = "X-coordinate of callout origin (label anchor) (since QGIS 3.20)"
QgsCallout.OriginY = QgsCallout.Property.OriginY
QgsCallout.OriginY.is_monkey_patched = True
QgsCallout.OriginY.__doc__ = "Y-coordinate of callout origin (label anchor) (since QGIS 3.20)"
QgsCallout.DestinationX = QgsCallout.Property.DestinationX
QgsCallout.DestinationX.is_monkey_patched = True
QgsCallout.DestinationX.__doc__ = "X-coordinate of callout destination (feature anchor) (since QGIS 3.20)"
QgsCallout.DestinationY = QgsCallout.Property.DestinationY
QgsCallout.DestinationY.is_monkey_patched = True
QgsCallout.DestinationY.__doc__ = "Y-coordinate of callout destination (feature anchor) (since QGIS 3.20)"
QgsCallout.Curvature = QgsCallout.Property.Curvature
QgsCallout.Curvature.is_monkey_patched = True
QgsCallout.Curvature.__doc__ = "Curvature of curved line callouts (since QGIS 3.20)"
QgsCallout.Orientation = QgsCallout.Property.Orientation
QgsCallout.Orientation.is_monkey_patched = True
QgsCallout.Orientation.__doc__ = "Orientation of curved line callouts (since QGIS 3.20)"
QgsCallout.Margins = QgsCallout.Property.Margins
QgsCallout.Margins.is_monkey_patched = True
QgsCallout.Margins.__doc__ = "Margin from text (since QGIS 3.20)"
QgsCallout.WedgeWidth = QgsCallout.Property.WedgeWidth
QgsCallout.WedgeWidth.is_monkey_patched = True
QgsCallout.WedgeWidth.__doc__ = "Balloon callout wedge width (since QGIS 3.20)"
QgsCallout.CornerRadius = QgsCallout.Property.CornerRadius
QgsCallout.CornerRadius.is_monkey_patched = True
QgsCallout.CornerRadius.__doc__ = "Balloon callout corner radius (since QGIS 3.20)"
QgsCallout.BlendMode = QgsCallout.Property.BlendMode
QgsCallout.BlendMode.is_monkey_patched = True
QgsCallout.BlendMode.__doc__ = "Callout blend mode (since QGIS 3.20)"
QgsCallout.Property.__doc__ = "Data definable properties.\n\n" + '* ``MinimumCalloutLength``: ' + QgsCallout.Property.MinimumCalloutLength.__doc__ + '\n' + '* ``OffsetFromAnchor``: ' + QgsCallout.Property.OffsetFromAnchor.__doc__ + '\n' + '* ``OffsetFromLabel``: ' + QgsCallout.Property.OffsetFromLabel.__doc__ + '\n' + '* ``DrawCalloutToAllParts``: ' + QgsCallout.Property.DrawCalloutToAllParts.__doc__ + '\n' + '* ``AnchorPointPosition``: ' + QgsCallout.Property.AnchorPointPosition.__doc__ + '\n' + '* ``LabelAnchorPointPosition``: ' + QgsCallout.Property.LabelAnchorPointPosition.__doc__ + '\n' + '* ``OriginX``: ' + QgsCallout.Property.OriginX.__doc__ + '\n' + '* ``OriginY``: ' + QgsCallout.Property.OriginY.__doc__ + '\n' + '* ``DestinationX``: ' + QgsCallout.Property.DestinationX.__doc__ + '\n' + '* ``DestinationY``: ' + QgsCallout.Property.DestinationY.__doc__ + '\n' + '* ``Curvature``: ' + QgsCallout.Property.Curvature.__doc__ + '\n' + '* ``Orientation``: ' + QgsCallout.Property.Orientation.__doc__ + '\n' + '* ``Margins``: ' + QgsCallout.Property.Margins.__doc__ + '\n' + '* ``WedgeWidth``: ' + QgsCallout.Property.WedgeWidth.__doc__ + '\n' + '* ``CornerRadius``: ' + QgsCallout.Property.CornerRadius.__doc__ + '\n' + '* ``BlendMode``: ' + QgsCallout.Property.BlendMode.__doc__
# --
try:
    QgsCallout.QgsCalloutContext.__attribute_docs__ = {'allFeaturePartsLabeled': '``True`` if all parts of associated feature were labeled', 'originalFeatureCrs': 'Contains the CRS of the original feature associated with this callout.\n\n.. versionadded:: 3.20'}
except NameError:
    pass
QgsCallout.propertyDefinitions = staticmethod(QgsCallout.propertyDefinitions)
QgsCallout.encodeAnchorPoint = staticmethod(QgsCallout.encodeAnchorPoint)
QgsCallout.decodeAnchorPoint = staticmethod(QgsCallout.decodeAnchorPoint)
QgsCallout.encodeLabelAnchorPoint = staticmethod(QgsCallout.encodeLabelAnchorPoint)
QgsCallout.decodeLabelAnchorPoint = staticmethod(QgsCallout.decodeLabelAnchorPoint)
QgsSimpleLineCallout.create = staticmethod(QgsSimpleLineCallout.create)
QgsManhattanLineCallout.create = staticmethod(QgsManhattanLineCallout.create)
QgsCurvedLineCallout.create = staticmethod(QgsCurvedLineCallout.create)
QgsBalloonCallout.create = staticmethod(QgsBalloonCallout.create)
