# The following has been generated automatically from src/core/qgssunpositioncalculator.h
try:
    QgsSunPositionResult.__attribute_docs__ = {'azimuth': 'Azimuth angle in degrees clockwise from North.', 'apparentElevation': 'Apparent topocentric elevation angle in degrees (corrected for atmospheric refraction).', 'solarMidnightBefore': 'The datetime of the solar midnight preceding the calculation time (in UTC).', 'solarTransit': 'The datetime of solar transit (solar noon) when the sun reaches its highest elevation (in UTC).', 'solarMidnightAfter': 'The datetime of the solar midnight following the calculation time (in UTC).', 'sunrise': "The datetime of sunrise, defined as the moment the upper edge of the sun's disk becomes visible above the horizon (in UTC).", 'sunset': "The datetime of sunset, defined as the moment the upper edge of the sun's disk disappears below the horizon (in UTC).", 'civilDawn': 'The datetime of civil dawn, when the geometric center of the sun is 6 degrees below the horizon in the morning (in UTC).', 'civilDusk': 'The datetime of civil dusk, when the geometric center of the sun is 6 degrees below the horizon in the evening (in UTC).', 'nauticalDawn': 'The datetime of nautical dawn, when the geometric center of the sun is 12 degrees below the horizon in the morning (in UTC).', 'nauticalDusk': 'The datetime of nautical dusk, when the geometric center of the sun is 12 degrees below the horizon in the evening (in UTC).', 'astronomicalDawn': 'The datetime of astronomical dawn, when the geometric center of the sun is 18 degrees below the horizon in the morning (in UTC).', 'astronomicalDusk': 'The datetime of astronomical dusk, when the geometric center of the sun is 18 degrees below the horizon in the evening (in UTC).'}
    QgsSunPositionResult.__annotations__ = {'azimuth': float, 'apparentElevation': float, 'solarMidnightBefore': 'QDateTime', 'solarTransit': 'QDateTime', 'solarMidnightAfter': 'QDateTime', 'sunrise': 'QDateTime', 'sunset': 'QDateTime', 'civilDawn': 'QDateTime', 'civilDusk': 'QDateTime', 'nauticalDawn': 'QDateTime', 'nauticalDusk': 'QDateTime', 'astronomicalDawn': 'QDateTime', 'astronomicalDusk': 'QDateTime'}
    QgsSunPositionResult.__doc__ = """Contains the results of a solar position calculation.

.. versionadded:: 4.2"""
except (NameError, AttributeError):
    pass
try:
    QgsSunPositionCalculator.calculate = staticmethod(QgsSunPositionCalculator.calculate)
except (NameError, AttributeError):
    pass
