# The following has been generated automatically from src/core/project/qgsprojectdisplaysettings.h
try:
    QgsProjectDisplaySettings.__attribute_docs__ = {'bearingFormatChanged': 'Emitted when the bearing format changes.\n\n.. seealso:: :py:func:`setBearingFormat`\n\n.. seealso:: :py:func:`bearingFormat`\n', 'geographicCoordinateFormatChanged': 'Emitted when the geographic coordinate format changes.\n\n.. seealso:: :py:func:`setGeographicCoordinateFormat`\n\n.. seealso:: :py:func:`geographicCoordinateFormat`\n', 'coordinateTypeChanged': 'Emitted when the default coordinate format changes.\n\n.. seealso:: :py:func:`setCoordinateType`\n\n.. seealso:: :py:func:`coordinateType`\n\n.. versionadded:: 3.28\n', 'coordinateAxisOrderChanged': 'Emitted when the default coordinate axis order changes.\n\n.. seealso:: :py:func:`setCoordinateAxisOrder`\n\n.. seealso:: :py:func:`coordinateAxisOrder`\n\n.. versionadded:: 3.28\n', 'coordinateCustomCrsChanged': 'Emitted when the coordinate custom CRS changes.\n\n.. seealso:: :py:func:`setCoordinateCustomCrs`\n\n.. seealso:: :py:func:`coordinateCustomCrs`\n\n.. versionadded:: 3.28\n', 'coordinateCrsChanged': 'Emitted when the coordinate CRS changes.\n\n.. seealso:: :py:func:`coordinateCrs`\n\n.. seealso:: :py:func:`coordinateType`\n\n.. versionadded:: 3.28\n'}
    import functools as _functools
    __wrapped_QgsProjectDisplaySettings_setBearingFormat = QgsProjectDisplaySettings.setBearingFormat
    def __QgsProjectDisplaySettings_setBearingFormat_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProjectDisplaySettings_setBearingFormat(self, arg)
    QgsProjectDisplaySettings.setBearingFormat = _functools.update_wrapper(__QgsProjectDisplaySettings_setBearingFormat_wrapper, QgsProjectDisplaySettings.setBearingFormat)

    import functools as _functools
    __wrapped_QgsProjectDisplaySettings_setGeographicCoordinateFormat = QgsProjectDisplaySettings.setGeographicCoordinateFormat
    def __QgsProjectDisplaySettings_setGeographicCoordinateFormat_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProjectDisplaySettings_setGeographicCoordinateFormat(self, arg)
    QgsProjectDisplaySettings.setGeographicCoordinateFormat = _functools.update_wrapper(__QgsProjectDisplaySettings_setGeographicCoordinateFormat_wrapper, QgsProjectDisplaySettings.setGeographicCoordinateFormat)

    QgsProjectDisplaySettings.__group__ = ['project']
except (NameError, AttributeError):
    pass
