# The following has been generated automatically from src/gui/qgsadvanceddigitizingfloater.h
# monkey patching scoped based enum
QgsAdvancedDigitizingFloater.FloaterItem.XCoordinate.__doc__ = "X coordinate"
QgsAdvancedDigitizingFloater.FloaterItem.YCoordinate.__doc__ = "Y coordinate"
QgsAdvancedDigitizingFloater.FloaterItem.MCoordinate.__doc__ = "M coordinate"
QgsAdvancedDigitizingFloater.FloaterItem.ZCoordinate.__doc__ = "Z coordinate"
QgsAdvancedDigitizingFloater.FloaterItem.Angle.__doc__ = "Angle between segments"
QgsAdvancedDigitizingFloater.FloaterItem.CommonAngleSnapping.__doc__ = "Common angles"
QgsAdvancedDigitizingFloater.FloaterItem.Distance.__doc__ = "Distance (segment length)"
QgsAdvancedDigitizingFloater.FloaterItem.Bearing.__doc__ = "Segment bearing"
QgsAdvancedDigitizingFloater.FloaterItem.Weight.__doc__ = "Weight for NURBSCurve \n.. versionadded:: 4.0"
QgsAdvancedDigitizingFloater.FloaterItem.Area.__doc__ = "Total area \n.. versionadded:: 4.0"
QgsAdvancedDigitizingFloater.FloaterItem.TotalLength.__doc__ = "Total length (or perimeter) \n.. versionadded:: 4.0"
QgsAdvancedDigitizingFloater.FloaterItem.__doc__ = """Available floater items

* ``XCoordinate``: X coordinate
* ``YCoordinate``: Y coordinate
* ``MCoordinate``: M coordinate
* ``ZCoordinate``: Z coordinate
* ``Angle``: Angle between segments
* ``CommonAngleSnapping``: Common angles
* ``Distance``: Distance (segment length)
* ``Bearing``: Segment bearing
* ``Weight``: Weight for NURBSCurve

  .. versionadded:: 4.0

* ``Area``: Total area

  .. versionadded:: 4.0

* ``TotalLength``: Total length (or perimeter)

  .. versionadded:: 4.0


"""
# --
QgsAdvancedDigitizingFloater.FloaterItems = lambda flags=0: QgsAdvancedDigitizingFloater.FloaterItem(flags)
QgsAdvancedDigitizingFloater.FloaterItem.baseClass = QgsAdvancedDigitizingFloater
FloaterItem = QgsAdvancedDigitizingFloater  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsAdvancedDigitizingFloater.itemSupportsMeasurementType = staticmethod(QgsAdvancedDigitizingFloater.itemSupportsMeasurementType)
    QgsAdvancedDigitizingFloater.__overridden_methods__ = ['eventFilter']
except (NameError, AttributeError):
    pass
