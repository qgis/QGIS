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
QgsLayoutGuideCollection.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsLayoutGuideCollection.Roles

.. versionadded:: 3.36

* ``Orientation``: Guide orientation role

  Available as ``QgsLayoutGuideCollection.OrientationRole`` in older QGIS releases.

* ``Position``: Guide position role

  Available as ``QgsLayoutGuideCollection.PositionRole`` in older QGIS releases.

* ``Units``: Guide position units role

  Available as ``QgsLayoutGuideCollection.UnitsRole`` in older QGIS releases.

* ``Page``: Guide page role

  Available as ``QgsLayoutGuideCollection.PageRole`` in older QGIS releases.

* ``LayoutPosition``: Guide position in layout coordinates

  Available as ``QgsLayoutGuideCollection.LayoutPositionRole`` in older QGIS releases.


"""
# --
QgsLayoutGuideCollection.CustomRole.baseClass = QgsLayoutGuideCollection
try:
    QgsLayoutGuide.__attribute_docs__ = {'positionChanged': "Emitted when the guide's position is changed.\n"}
    QgsLayoutGuide.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutGuideCollection.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutGuideProxyModel.__group__ = ['layout']
except (NameError, AttributeError):
    pass
