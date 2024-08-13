# The following has been generated automatically from src/core/layout/qgslayoutguidecollection.h
QgsLayoutGuideCollection.Roles = QgsLayoutGuideCollection.CustomRole
# monkey patching scoped based enum
QgsLayoutGuideCollection.OrientationRole = QgsLayoutGuideCollection.CustomRole.Orientation
QgsLayoutGuideCollection.Roles.OrientationRole = QgsLayoutGuideCollection.CustomRole.Orientation
QgsLayoutGuideCollection.OrientationRole.is_monkey_patched = True
QgsLayoutGuideCollection.OrientationRole.__doc__ = "Guide orientation role"
QgsLayoutGuideCollection.PositionRole = QgsLayoutGuideCollection.CustomRole.Position
QgsLayoutGuideCollection.Roles.PositionRole = QgsLayoutGuideCollection.CustomRole.Position
QgsLayoutGuideCollection.PositionRole.is_monkey_patched = True
QgsLayoutGuideCollection.PositionRole.__doc__ = "Guide position role"
QgsLayoutGuideCollection.UnitsRole = QgsLayoutGuideCollection.CustomRole.Units
QgsLayoutGuideCollection.Roles.UnitsRole = QgsLayoutGuideCollection.CustomRole.Units
QgsLayoutGuideCollection.UnitsRole.is_monkey_patched = True
QgsLayoutGuideCollection.UnitsRole.__doc__ = "Guide position units role"
QgsLayoutGuideCollection.PageRole = QgsLayoutGuideCollection.CustomRole.Page
QgsLayoutGuideCollection.Roles.PageRole = QgsLayoutGuideCollection.CustomRole.Page
QgsLayoutGuideCollection.PageRole.is_monkey_patched = True
QgsLayoutGuideCollection.PageRole.__doc__ = "Guide page role"
QgsLayoutGuideCollection.LayoutPositionRole = QgsLayoutGuideCollection.CustomRole.LayoutPosition
QgsLayoutGuideCollection.Roles.LayoutPositionRole = QgsLayoutGuideCollection.CustomRole.LayoutPosition
QgsLayoutGuideCollection.LayoutPositionRole.is_monkey_patched = True
QgsLayoutGuideCollection.LayoutPositionRole.__doc__ = "Guide position in layout coordinates"
QgsLayoutGuideCollection.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsLayoutGuideCollection.Roles\n\n.. versionadded:: 3.36\n\n" + '* ``OrientationRole``: ' + QgsLayoutGuideCollection.CustomRole.Orientation.__doc__ + '\n' + '* ``PositionRole``: ' + QgsLayoutGuideCollection.CustomRole.Position.__doc__ + '\n' + '* ``UnitsRole``: ' + QgsLayoutGuideCollection.CustomRole.Units.__doc__ + '\n' + '* ``PageRole``: ' + QgsLayoutGuideCollection.CustomRole.Page.__doc__ + '\n' + '* ``LayoutPositionRole``: ' + QgsLayoutGuideCollection.CustomRole.LayoutPosition.__doc__
# --
QgsLayoutGuideCollection.CustomRole.baseClass = QgsLayoutGuideCollection
try:
    QgsLayoutGuide.__attribute_docs__ = {'positionChanged': "Emitted when the guide's position is changed.\n"}
except NameError:
    pass
