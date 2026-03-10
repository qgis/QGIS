# The following has been generated automatically from src/core/project/qgsselectivemaskingsourcesetmanagermodel.h
# monkey patching scoped based enum
QgsSelectiveMaskingSourceSetManagerModel.CustomRole.Object.__doc__ = "Object"
QgsSelectiveMaskingSourceSetManagerModel.CustomRole.IsEmptyObject.__doc__ = "``True`` if row represents the empty object"
QgsSelectiveMaskingSourceSetManagerModel.CustomRole.SetId.__doc__ = "Selective masking source set unique ID"
QgsSelectiveMaskingSourceSetManagerModel.CustomRole.__doc__ = """Custom model roles.

* ``Object``: Object
* ``IsEmptyObject``: ``True`` if row represents the empty object
* ``SetId``: Selective masking source set unique ID

"""
# --
QgsSelectiveMaskingSourceSetManagerModel.CustomRole.baseClass = QgsSelectiveMaskingSourceSetManagerModel
try:
    QgsSelectiveMaskingSourceSetManagerModel.__overridden_methods__ = ['data']
    QgsSelectiveMaskingSourceSetManagerModel.__group__ = ['project']
except (NameError, AttributeError):
    pass
try:
    QgsSelectiveMaskingSourceSetManagerProxyModel.__group__ = ['project']
except (NameError, AttributeError):
    pass
