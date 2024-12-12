# The following has been generated automatically from src/core/gps/qgssatelliteinformation.h
try:
    QgsSatelliteInfo.__attribute_docs__ = {'id': 'Contains the satellite identifier number.\n\nThe satellite identifier number can be used to identify a satellite inside the satellite system.\nFor satellite system GPS the satellite identifier number represents the PRN (Pseudo-random noise)\nnumber. For satellite system GLONASS the satellite identifier number represents the slot number.', 'inUse': '``True`` if satellite was used in obtaining the position fix.', 'elevation': 'Elevation of the satellite, in degrees.', 'azimuth': 'The azimuth of the satellite to true north, in degrees.', 'signal': 'Signal strength (0-99dB), or -1 if not available.', 'satType': 'satType value from NMEA message $GxGSV, where x:\nP = GPS; S = SBAS (GPSid> 32 then SBasid = GPSid + 87); N = generic satellite; L = GLONASS; A = GALILEO; B = BEIDOU; Q = QZSS;'}
    QgsSatelliteInfo.__group__ = ['gps']
except (NameError, AttributeError):
    pass
