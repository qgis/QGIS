# The following has been generated automatically from src/gui/qgsdatetimewithtimezonewidget.h
# monkey patching scoped based enum
QgsTimeZoneItemModel.CustomRole.TimeZoneId.__doc__ = "Time zone ID"
QgsTimeZoneItemModel.CustomRole.TimeZone.__doc__ = "QTimeZone object"
QgsTimeZoneItemModel.CustomRole.UtcOffset.__doc__ = "Offset from UTC"
QgsTimeZoneItemModel.CustomRole.__doc__ = """Model roles

* ``TimeZoneId``: Time zone ID
* ``TimeZone``: QTimeZone object
* ``UtcOffset``: Offset from UTC

"""
# --
QgsTimeZoneItemModel.CustomRole.baseClass = QgsTimeZoneItemModel
# monkey patching scoped based enum
QgsTimeZoneComboBox.StandardItem.Invalid.__doc__ = "Invalid/not set option"
QgsTimeZoneComboBox.StandardItem.Current.__doc__ = "Current time zone value"
QgsTimeZoneComboBox.StandardItem.System.__doc__ = "System time zone"
QgsTimeZoneComboBox.StandardItem.__doc__ = """Standard time zone options

* ``Invalid``: Invalid/not set option
* ``Current``: Current time zone value
* ``System``: System time zone

"""
# --
QgsTimeZoneComboBox.StandardItems = lambda flags=0: QgsTimeZoneComboBox.StandardItem(flags)
try:
    QgsTimeZoneWidget.__attribute_docs__ = {'timeZoneChanged': 'Emitted when the selected time zone is changed.\n\n.. seealso:: :py:func:`setTimeZone`\n\n.. seealso:: :py:func:`timeZone`\n', 'timeZoneDoubleClicked': 'Emitted when a time zone is double-clicked in the widget.\n'}
    QgsTimeZoneWidget.__signal_arguments__ = {'timeZoneChanged': ['zone: QTimeZone'], 'timeZoneDoubleClicked': ['zone: QTimeZone']}
except (NameError, AttributeError):
    pass
try:
    QgsTimeZoneComboBox.__attribute_docs__ = {'timeZoneChanged': 'Emitted when the selected time zone is changed.\n\n.. seealso:: :py:func:`setTimeZone`\n\n.. seealso:: :py:func:`timeZone`\n'}
    QgsTimeZoneComboBox.__signal_arguments__ = {'timeZoneChanged': ['zone: QTimeZone']}
except (NameError, AttributeError):
    pass
try:
    QgsDateTimeWithTimeZoneWidget.__attribute_docs__ = {'dateTimeChanged': 'Emitted whenever the date time shown in the widget is changed.\n'}
    QgsDateTimeWithTimeZoneWidget.__signal_arguments__ = {'dateTimeChanged': ['dateTime: QDateTime']}
except (NameError, AttributeError):
    pass
try:
    QgsTimeZoneItemModel.displayStringForTimeZone = staticmethod(QgsTimeZoneItemModel.displayStringForTimeZone)
    QgsTimeZoneItemModel.__overridden_methods__ = ['rowCount', 'columnCount', 'data', 'index', 'parent']
except (NameError, AttributeError):
    pass
try:
    QgsTimeZoneSortFilterModel.__overridden_methods__ = ['filterAcceptsRow', 'lessThan']
except (NameError, AttributeError):
    pass
