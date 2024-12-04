# The following has been generated automatically from src/gui/proj/qgsprojectionselectionwidget.h
QgsProjectionSelectionWidget.Invalid = QgsProjectionSelectionWidget.CrsOption.Invalid
QgsProjectionSelectionWidget.LayerCrs = QgsProjectionSelectionWidget.CrsOption.LayerCrs
QgsProjectionSelectionWidget.ProjectCrs = QgsProjectionSelectionWidget.CrsOption.ProjectCrs
QgsProjectionSelectionWidget.CurrentCrs = QgsProjectionSelectionWidget.CrsOption.CurrentCrs
QgsProjectionSelectionWidget.DefaultCrs = QgsProjectionSelectionWidget.CrsOption.DefaultCrs
QgsProjectionSelectionWidget.RecentCrs = QgsProjectionSelectionWidget.CrsOption.RecentCrs
QgsProjectionSelectionWidget.CrsNotSet = QgsProjectionSelectionWidget.CrsOption.CrsNotSet
QgsProjectionSelectionWidget.CrsOptions = lambda flags=0: QgsProjectionSelectionWidget.CrsOption(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsProjectionSelectionWidget.CrsOption.__bool__ = lambda flag: bool(_force_int(flag))
QgsProjectionSelectionWidget.CrsOption.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProjectionSelectionWidget.CrsOption.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProjectionSelectionWidget.CrsOption.__or__ = lambda flag1, flag2: QgsProjectionSelectionWidget.CrsOption(_force_int(flag1) | _force_int(flag2))
try:
    QgsProjectionSelectionWidget.__attribute_docs__ = {'crsChanged': 'Emitted when the selected CRS is changed\n', 'cleared': 'Emitted when the not set option is selected.\n'}
    QgsProjectionSelectionWidget.__signal_arguments__ = {'crsChanged': ['crs: QgsCoordinateReferenceSystem']}
    QgsProjectionSelectionWidget.__group__ = ['proj']
except (NameError, AttributeError):
    pass
