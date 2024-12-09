# The following has been generated automatically from src/core/qgscadutils.h
try:
    QgsCadUtils.AlignMapPointConstraint.__attribute_docs__ = {'locked': 'Whether the constraint is active, i.e. should be considered', 'relative': 'Whether the value is relative to previous value', 'value': 'Numeric value of the constraint (coordinate/distance in map units or angle in degrees)'}
except (NameError, AttributeError):
    pass
try:
    QgsCadUtils.AlignMapPointOutput.__attribute_docs__ = {'valid': 'Whether the combination of constraints is actually valid', 'finalMapPoint': 'map point aligned according to the constraints', 'snapMatch': 'Snapped point - only valid if actually used for something\n\n.. versionadded:: 3.14', 'edgeMatch': 'Snapped segment - only valid if actually used for something\n\n.. deprecated:: 3.40\n\n   Will be removed in QGIS 4.0 - use :py:func:`~AlignMapPointOutput.snapMatch` instead.', 'softLockCommonAngle': 'Angle (in degrees) to which we have soft-locked ourselves (if not set it is -1)'}
except (NameError, AttributeError):
    pass
try:
    QgsCadUtils.AlignMapPointContext.__attribute_docs__ = {'snappingUtils': 'Snapping utils that will be used to snap point to map. Must not be ``None``.', 'mapUnitsPerPixel': 'Map units/pixel ratio from map canvas.', 'xConstraint': 'Constraint for X coordinate', 'yConstraint': 'Constraint for Y coordinate', 'zConstraint': 'Constraint for Z coordinate\n\n.. versionadded:: 3.22', 'mConstraint': 'Constraint for M coordinate\n\n.. versionadded:: 3.22', 'distanceConstraint': 'Constraint for distance', 'angleConstraint': 'Constraint for angle', 'commonAngleConstraint': 'Constraint for soft lock to a common angle', 'snappingToFeaturesOverridesCommonAngle': 'Flag to set snapping to features priority over common angle.\n\n.. versionadded:: 3.32'}
except (NameError, AttributeError):
    pass
try:
    QgsCadUtils.alignMapPoint = staticmethod(QgsCadUtils.alignMapPoint)
except (NameError, AttributeError):
    pass
