# The following has been generated automatically from src/gui/qgsadvanceddigitizingfloater.h
# monkey patching scoped based enum
QgsAdvancedDigitizingFloater.FloaterItem.XCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.XCoordinate = QgsAdvancedDigitizingFloater.FloaterItem.XCoordinate
QgsAdvancedDigitizingFloater.FloaterItem.YCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.YCoordinate = QgsAdvancedDigitizingFloater.FloaterItem.YCoordinate
QgsAdvancedDigitizingFloater.FloaterItem.MCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.MCoordinate = QgsAdvancedDigitizingFloater.FloaterItem.MCoordinate
QgsAdvancedDigitizingFloater.FloaterItem.ZCoordinate.__doc__ = ""
QgsAdvancedDigitizingFloater.ZCoordinate = QgsAdvancedDigitizingFloater.FloaterItem.ZCoordinate
QgsAdvancedDigitizingFloater.FloaterItem.Angle.__doc__ = ""
QgsAdvancedDigitizingFloater.Angle = QgsAdvancedDigitizingFloater.FloaterItem.Angle
QgsAdvancedDigitizingFloater.FloaterItem.CommonAngleSnapping.__doc__ = ""
QgsAdvancedDigitizingFloater.CommonAngleSnapping = QgsAdvancedDigitizingFloater.FloaterItem.CommonAngleSnapping
QgsAdvancedDigitizingFloater.FloaterItem.Distance.__doc__ = ""
QgsAdvancedDigitizingFloater.Distance = QgsAdvancedDigitizingFloater.FloaterItem.Distance
QgsAdvancedDigitizingFloater.FloaterItem.Bearing.__doc__ = ""
QgsAdvancedDigitizingFloater.Bearing = QgsAdvancedDigitizingFloater.FloaterItem.Bearing
QgsAdvancedDigitizingFloater.FloaterItem.__doc__ = "Available floater items\n\n" + '* ``XCoordinate``: ' + QgsAdvancedDigitizingFloater.FloaterItem.XCoordinate.__doc__ + '\n' + '* ``YCoordinate``: ' + QgsAdvancedDigitizingFloater.FloaterItem.YCoordinate.__doc__ + '\n' + '* ``MCoordinate``: ' + QgsAdvancedDigitizingFloater.FloaterItem.MCoordinate.__doc__ + '\n' + '* ``ZCoordinate``: ' + QgsAdvancedDigitizingFloater.FloaterItem.ZCoordinate.__doc__ + '\n' + '* ``Angle``: ' + QgsAdvancedDigitizingFloater.FloaterItem.Angle.__doc__ + '\n' + '* ``CommonAngleSnapping``: ' + QgsAdvancedDigitizingFloater.FloaterItem.CommonAngleSnapping.__doc__ + '\n' + '* ``Distance``: ' + QgsAdvancedDigitizingFloater.FloaterItem.Distance.__doc__ + '\n' + '* ``Bearing``: ' + QgsAdvancedDigitizingFloater.FloaterItem.Bearing.__doc__
# --
QgsAdvancedDigitizingFloater.FloaterItems = lambda flags=0: QgsAdvancedDigitizingFloater.FloaterItem(flags)
QgsAdvancedDigitizingFloater.FloaterItem.baseClass = QgsAdvancedDigitizingFloater
FloaterItem = QgsAdvancedDigitizingFloater  # dirty hack since SIP seems to introduce the flags in module
