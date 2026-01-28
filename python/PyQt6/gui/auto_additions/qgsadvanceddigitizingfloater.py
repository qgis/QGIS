# The following has been generated automatically from src/gui/qgsadvanceddigitizingfloater.h
# monkey patching scoped based enum
QgsAdvancedDigitizingFloater.FloaterItem.XCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.YCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.MCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.ZCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.Angle.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.CommonAngleSnapping.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.Distance.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.Bearing.__doc__ = ""
QgsAdvancedDigitizingFloater.FloaterItem.Weight.__doc__ = "Weight for NURBSCurve \n.. versionadded:: 4.0"
QgsAdvancedDigitizingFloater.FloaterItem.__doc__ = """Available floater items

* ``XCoordinate``: 
* ``YCoordinate``: 
* ``MCoordinate``: 
* ``ZCoordinate``: 
* ``Angle``: 
* ``CommonAngleSnapping``: 
* ``Distance``: 
* ``Bearing``: 
* ``Weight``: Weight for NURBSCurve

  .. versionadded:: 4.0


"""
# --
QgsAdvancedDigitizingFloater.FloaterItems = lambda flags=0: QgsAdvancedDigitizingFloater.FloaterItem(flags)
QgsAdvancedDigitizingFloater.FloaterItem.baseClass = QgsAdvancedDigitizingFloater
FloaterItem = QgsAdvancedDigitizingFloater  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsAdvancedDigitizingFloater.__overridden_methods__ = ['eventFilter']
except (NameError, AttributeError):
    pass
