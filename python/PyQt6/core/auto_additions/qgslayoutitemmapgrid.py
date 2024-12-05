# The following has been generated automatically from src/core/layout/qgslayoutitemmapgrid.h
QgsLayoutItemMapGrid.MapUnit = QgsLayoutItemMapGrid.GridUnit.MapUnit
QgsLayoutItemMapGrid.MM = QgsLayoutItemMapGrid.GridUnit.MM
QgsLayoutItemMapGrid.CM = QgsLayoutItemMapGrid.GridUnit.CM
QgsLayoutItemMapGrid.DynamicPageSizeBased = QgsLayoutItemMapGrid.GridUnit.DynamicPageSizeBased
QgsLayoutItemMapGrid.Solid = QgsLayoutItemMapGrid.GridStyle.Solid
QgsLayoutItemMapGrid.Cross = QgsLayoutItemMapGrid.GridStyle.Cross
QgsLayoutItemMapGrid.Markers = QgsLayoutItemMapGrid.GridStyle.Markers
QgsLayoutItemMapGrid.FrameAnnotationsOnly = QgsLayoutItemMapGrid.GridStyle.FrameAnnotationsOnly
QgsLayoutItemMapGrid.ShowAll = QgsLayoutItemMapGrid.DisplayMode.ShowAll
QgsLayoutItemMapGrid.LatitudeOnly = QgsLayoutItemMapGrid.DisplayMode.LatitudeOnly
QgsLayoutItemMapGrid.LongitudeOnly = QgsLayoutItemMapGrid.DisplayMode.LongitudeOnly
QgsLayoutItemMapGrid.HideAll = QgsLayoutItemMapGrid.DisplayMode.HideAll
QgsLayoutItemMapGrid.InsideMapFrame = QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame
QgsLayoutItemMapGrid.OutsideMapFrame = QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame
QgsLayoutItemMapGrid.Horizontal = QgsLayoutItemMapGrid.AnnotationDirection.Horizontal
QgsLayoutItemMapGrid.Vertical = QgsLayoutItemMapGrid.AnnotationDirection.Vertical
QgsLayoutItemMapGrid.VerticalDescending = QgsLayoutItemMapGrid.AnnotationDirection.VerticalDescending
QgsLayoutItemMapGrid.BoundaryDirection = QgsLayoutItemMapGrid.AnnotationDirection.BoundaryDirection
QgsLayoutItemMapGrid.AboveTick = QgsLayoutItemMapGrid.AnnotationDirection.AboveTick
QgsLayoutItemMapGrid.OnTick = QgsLayoutItemMapGrid.AnnotationDirection.OnTick
QgsLayoutItemMapGrid.UnderTick = QgsLayoutItemMapGrid.AnnotationDirection.UnderTick
QgsLayoutItemMapGrid.Decimal = QgsLayoutItemMapGrid.AnnotationFormat.Decimal
QgsLayoutItemMapGrid.DegreeMinute = QgsLayoutItemMapGrid.AnnotationFormat.DegreeMinute
QgsLayoutItemMapGrid.DegreeMinuteSecond = QgsLayoutItemMapGrid.AnnotationFormat.DegreeMinuteSecond
QgsLayoutItemMapGrid.DecimalWithSuffix = QgsLayoutItemMapGrid.AnnotationFormat.DecimalWithSuffix
QgsLayoutItemMapGrid.DegreeMinuteNoSuffix = QgsLayoutItemMapGrid.AnnotationFormat.DegreeMinuteNoSuffix
QgsLayoutItemMapGrid.DegreeMinutePadded = QgsLayoutItemMapGrid.AnnotationFormat.DegreeMinutePadded
QgsLayoutItemMapGrid.DegreeMinuteSecondNoSuffix = QgsLayoutItemMapGrid.AnnotationFormat.DegreeMinuteSecondNoSuffix
QgsLayoutItemMapGrid.DegreeMinuteSecondPadded = QgsLayoutItemMapGrid.AnnotationFormat.DegreeMinuteSecondPadded
QgsLayoutItemMapGrid.CustomFormat = QgsLayoutItemMapGrid.AnnotationFormat.CustomFormat
QgsLayoutItemMapGrid.Left = QgsLayoutItemMapGrid.BorderSide.Left
QgsLayoutItemMapGrid.Right = QgsLayoutItemMapGrid.BorderSide.Right
QgsLayoutItemMapGrid.Bottom = QgsLayoutItemMapGrid.BorderSide.Bottom
QgsLayoutItemMapGrid.Top = QgsLayoutItemMapGrid.BorderSide.Top
QgsLayoutItemMapGrid.NoFrame = QgsLayoutItemMapGrid.FrameStyle.NoFrame
QgsLayoutItemMapGrid.Zebra = QgsLayoutItemMapGrid.FrameStyle.Zebra
QgsLayoutItemMapGrid.InteriorTicks = QgsLayoutItemMapGrid.FrameStyle.InteriorTicks
QgsLayoutItemMapGrid.ExteriorTicks = QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks
QgsLayoutItemMapGrid.InteriorExteriorTicks = QgsLayoutItemMapGrid.FrameStyle.InteriorExteriorTicks
QgsLayoutItemMapGrid.LineBorder = QgsLayoutItemMapGrid.FrameStyle.LineBorder
QgsLayoutItemMapGrid.LineBorderNautical = QgsLayoutItemMapGrid.FrameStyle.LineBorderNautical
QgsLayoutItemMapGrid.ZebraNautical = QgsLayoutItemMapGrid.FrameStyle.ZebraNautical
QgsLayoutItemMapGrid.OrthogonalTicks = QgsLayoutItemMapGrid.TickLengthMode.OrthogonalTicks
QgsLayoutItemMapGrid.NormalizedTicks = QgsLayoutItemMapGrid.TickLengthMode.NormalizedTicks
QgsLayoutItemMapGrid.FrameLeft = QgsLayoutItemMapGrid.FrameSideFlag.FrameLeft
QgsLayoutItemMapGrid.FrameRight = QgsLayoutItemMapGrid.FrameSideFlag.FrameRight
QgsLayoutItemMapGrid.FrameTop = QgsLayoutItemMapGrid.FrameSideFlag.FrameTop
QgsLayoutItemMapGrid.FrameBottom = QgsLayoutItemMapGrid.FrameSideFlag.FrameBottom
QgsLayoutItemMapGrid.FrameSideFlags = lambda flags=0: QgsLayoutItemMapGrid.FrameSideFlag(flags)
QgsLayoutItemMapGrid.Longitude = QgsLayoutItemMapGrid.AnnotationCoordinate.Longitude
QgsLayoutItemMapGrid.Latitude = QgsLayoutItemMapGrid.AnnotationCoordinate.Latitude
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsLayoutItemMapGrid.FrameSideFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsLayoutItemMapGrid.FrameSideFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsLayoutItemMapGrid.FrameSideFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsLayoutItemMapGrid.FrameSideFlag.__or__ = lambda flag1, flag2: QgsLayoutItemMapGrid.FrameSideFlag(_force_int(flag1) | _force_int(flag2))
try:
    QgsLayoutItemMapGrid.__attribute_docs__ = {'crsChanged': "Emitted whenever the grid's CRS is changed.\n\n.. versionadded:: 3.18\n"}
    QgsLayoutItemMapGrid.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemMapGridStack.__group__ = ['layout']
except (NameError, AttributeError):
    pass
