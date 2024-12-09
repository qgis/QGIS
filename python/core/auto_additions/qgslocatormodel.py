# The following has been generated automatically from src/core/locator/qgslocatormodel.h
QgsLocatorModel.Role = QgsLocatorModel.CustomRole
# monkey patching scoped based enum
QgsLocatorModel.ResultDataRole = QgsLocatorModel.CustomRole.ResultData
QgsLocatorModel.Role.ResultDataRole = QgsLocatorModel.CustomRole.ResultData
QgsLocatorModel.ResultDataRole.is_monkey_patched = True
QgsLocatorModel.ResultDataRole.__doc__ = "QgsLocatorResult data"
QgsLocatorModel.ResultTypeRole = QgsLocatorModel.CustomRole.ResultType
QgsLocatorModel.Role.ResultTypeRole = QgsLocatorModel.CustomRole.ResultType
QgsLocatorModel.ResultTypeRole.is_monkey_patched = True
QgsLocatorModel.ResultTypeRole.__doc__ = "Result type"
QgsLocatorModel.ResultFilterPriorityRole = QgsLocatorModel.CustomRole.ResultFilterPriority
QgsLocatorModel.Role.ResultFilterPriorityRole = QgsLocatorModel.CustomRole.ResultFilterPriority
QgsLocatorModel.ResultFilterPriorityRole.is_monkey_patched = True
QgsLocatorModel.ResultFilterPriorityRole.__doc__ = "Result priority, used by QgsLocatorProxyModel for sorting roles."
QgsLocatorModel.ResultScoreRole = QgsLocatorModel.CustomRole.ResultScore
QgsLocatorModel.Role.ResultScoreRole = QgsLocatorModel.CustomRole.ResultScore
QgsLocatorModel.ResultScoreRole.is_monkey_patched = True
QgsLocatorModel.ResultScoreRole.__doc__ = "Result match score, used by QgsLocatorProxyModel for sorting roles."
QgsLocatorModel.ResultFilterNameRole = QgsLocatorModel.CustomRole.ResultFilterName
QgsLocatorModel.Role.ResultFilterNameRole = QgsLocatorModel.CustomRole.ResultFilterName
QgsLocatorModel.ResultFilterNameRole.is_monkey_patched = True
QgsLocatorModel.ResultFilterNameRole.__doc__ = "Associated filter name which created the result"
QgsLocatorModel.ResultFilterGroupSortingRole = QgsLocatorModel.CustomRole.ResultFilterGroupSorting
QgsLocatorModel.Role.ResultFilterGroupSortingRole = QgsLocatorModel.CustomRole.ResultFilterGroupSorting
QgsLocatorModel.ResultFilterGroupSortingRole.is_monkey_patched = True
QgsLocatorModel.ResultFilterGroupSortingRole.__doc__ = "Custom value for sorting \n.. deprecated:: 3.40. No longer used."
QgsLocatorModel.ResultFilterGroupTitle = QgsLocatorModel.CustomRole.ResultFilterGroupTitle
QgsLocatorModel.ResultFilterGroupTitle.is_monkey_patched = True
QgsLocatorModel.ResultFilterGroupTitle.__doc__ = "Group title"
QgsLocatorModel.ResultFilterGroupScore = QgsLocatorModel.CustomRole.ResultFilterGroupScore
QgsLocatorModel.ResultFilterGroupScore.is_monkey_patched = True
QgsLocatorModel.ResultFilterGroupScore.__doc__ = "Group score"
QgsLocatorModel.ResultActionsRole = QgsLocatorModel.CustomRole.ResultActions
QgsLocatorModel.Role.ResultActionsRole = QgsLocatorModel.CustomRole.ResultActions
QgsLocatorModel.ResultActionsRole.is_monkey_patched = True
QgsLocatorModel.ResultActionsRole.__doc__ = "The actions to be shown for the given result in a context menu"
QgsLocatorModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsLocatorModel.Role

.. versionadded:: 3.36

* ``ResultData``: QgsLocatorResult data

  Available as ``QgsLocatorModel.ResultDataRole`` in older QGIS releases.

* ``ResultType``: Result type

  Available as ``QgsLocatorModel.ResultTypeRole`` in older QGIS releases.

* ``ResultFilterPriority``: Result priority, used by QgsLocatorProxyModel for sorting roles.

  Available as ``QgsLocatorModel.ResultFilterPriorityRole`` in older QGIS releases.

* ``ResultScore``: Result match score, used by QgsLocatorProxyModel for sorting roles.

  Available as ``QgsLocatorModel.ResultScoreRole`` in older QGIS releases.

* ``ResultFilterName``: Associated filter name which created the result

  Available as ``QgsLocatorModel.ResultFilterNameRole`` in older QGIS releases.

* ``ResultFilterGroupSorting``: Custom value for sorting

  .. deprecated:: 3.40. No longer used.


  Available as ``QgsLocatorModel.ResultFilterGroupSortingRole`` in older QGIS releases.

* ``ResultFilterGroupTitle``: Group title
* ``ResultFilterGroupScore``: Group score
* ``ResultActions``: The actions to be shown for the given result in a context menu

  Available as ``QgsLocatorModel.ResultActionsRole`` in older QGIS releases.


"""
# --
QgsLocatorModel.CustomRole.baseClass = QgsLocatorModel
try:
    QgsLocatorModel.__group__ = ['locator']
except (NameError, AttributeError):
    pass
try:
    QgsLocatorAutomaticModel.__group__ = ['locator']
except (NameError, AttributeError):
    pass
try:
    QgsLocatorProxyModel.__group__ = ['locator']
except (NameError, AttributeError):
    pass
