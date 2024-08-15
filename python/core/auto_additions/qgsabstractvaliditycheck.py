# The following has been generated automatically from src/core/validity/qgsabstractvaliditycheck.h
# monkey patching scoped based enum
QgsAbstractValidityCheck.TypeLayoutCheck = QgsAbstractValidityCheck.Type.LayoutCheck
QgsAbstractValidityCheck.Type.TypeLayoutCheck = QgsAbstractValidityCheck.Type.LayoutCheck
QgsAbstractValidityCheck.TypeLayoutCheck.is_monkey_patched = True
QgsAbstractValidityCheck.TypeLayoutCheck.__doc__ = "Print layout validity check, triggered on exporting a print layout"
QgsAbstractValidityCheck.TypeUserCheck = QgsAbstractValidityCheck.Type.UserCheck
QgsAbstractValidityCheck.Type.TypeUserCheck = QgsAbstractValidityCheck.Type.UserCheck
QgsAbstractValidityCheck.TypeUserCheck.is_monkey_patched = True
QgsAbstractValidityCheck.TypeUserCheck.__doc__ = "Starting point for custom user types"
QgsAbstractValidityCheck.Type.__doc__ = "Check types\n\n" + '* ``TypeLayoutCheck``: ' + QgsAbstractValidityCheck.Type.LayoutCheck.__doc__ + '\n' + '* ``TypeUserCheck``: ' + QgsAbstractValidityCheck.Type.UserCheck.__doc__
# --
try:
    QgsValidityCheckResult.__attribute_docs__ = {'type': 'Result type', 'title': 'A short, translated string summarising the result. Ideally a single sentence.', 'detailedDescription': 'Detailed description of the result (translated), giving users enough detail for them to resolve\nthe error.', 'checkId': 'ID of the check which generated the result. This is usually automatically populated.'}
except NameError:
    pass
try:
    QgsValidityCheckResult.__group__ = ['validity']
except NameError:
    pass
try:
    QgsAbstractValidityCheck.__group__ = ['validity']
except NameError:
    pass
