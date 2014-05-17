# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2013 Tom Kralidis
#
# Authors : Tom Kralidis <tomkralidis@gmail.com>
#
# Contact email: tomkralidis@gmail.com
# =============================================================================

"""
Sensor Observation Service (SOS) methods and metadata. Factory function.
"""

from swe.observation import sos100, sos200

def SensorObservationService(url, version='1.0.0', xml=None):
    """sos factory function, returns a version specific SensorObservationService object"""
    if version in  ['1.0', '1.0.0']:
        return sos100.SensorObservationService_1_0_0.__new__(sos100.SensorObservationService_1_0_0, url, version, xml)
    elif version in ['2.0', '2.0.0']:
        return sos200.SensorObservationService_2_0_0.__new__(sos200.SensorObservationService_2_0_0, url, version, xml)

