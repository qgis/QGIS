# The following has been generated automatically from src/core/layout/qgsmasterlayoutinterface.h
QgsMasterLayoutInterface.PrintLayout = QgsMasterLayoutInterface.Type.PrintLayout
QgsMasterLayoutInterface.Report = QgsMasterLayoutInterface.Type.Report
try:
    QgsMasterLayoutInterface.__virtual_methods__ = ['layoutAccept']
    QgsMasterLayoutInterface.__abstract_methods__ = ['clone', 'layoutType', 'name', 'icon', 'setName', 'layoutProject', 'writeLayoutXml', 'readLayoutXml', 'updateSettings']
    QgsMasterLayoutInterface.__group__ = ['layout']
except (NameError, AttributeError):
    pass
