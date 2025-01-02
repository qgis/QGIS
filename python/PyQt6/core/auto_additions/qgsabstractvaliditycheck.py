# The following has been generated automatically from src/core/validity/qgsabstractvaliditycheck.h
QgsValidityCheckResult.Warning = QgsValidityCheckResult.Type.Warning
QgsValidityCheckResult.Critical = QgsValidityCheckResult.Type.Critical
# monkey patching scoped based enum
QgsAbstractValidityCheck.TypeLayoutCheck = QgsAbstractValidityCheck.Type.LayoutCheck
QgsAbstractValidityCheck.Type.TypeLayoutCheck = QgsAbstractValidityCheck.Type.LayoutCheck
QgsAbstractValidityCheck.TypeLayoutCheck.is_monkey_patched = True
QgsAbstractValidityCheck.TypeLayoutCheck.__doc__ = "Print layout validity check, triggered on exporting a print layout"
QgsAbstractValidityCheck.TypeUserCheck = QgsAbstractValidityCheck.Type.UserCheck
QgsAbstractValidityCheck.Type.TypeUserCheck = QgsAbstractValidityCheck.Type.UserCheck
QgsAbstractValidityCheck.TypeUserCheck.is_monkey_patched = True
QgsAbstractValidityCheck.TypeUserCheck.__doc__ = "Starting point for custom user types"
QgsAbstractValidityCheck.Type.__doc__ = """Check types

* ``LayoutCheck``: Print layout validity check, triggered on exporting a print layout

  Available as ``QgsAbstractValidityCheck.TypeLayoutCheck`` in older QGIS releases.

* ``UserCheck``: Starting point for custom user types

  Available as ``QgsAbstractValidityCheck.TypeUserCheck`` in older QGIS releases.


"""
# --
try:
    QgsValidityCheckResult.__attribute_docs__ = {'type': 'Result type', 'title': 'A short, translated string summarising the result. Ideally a single sentence.', 'detailedDescription': 'Detailed description of the result (translated), giving users enough detail for them to resolve\nthe error.', 'checkId': 'ID of the check which generated the result. This is usually automatically populated.'}
    QgsValidityCheckResult.__group__ = ['validity']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractValidityCheck.__group__ = ['validity']
except (NameError, AttributeError):
    pass
