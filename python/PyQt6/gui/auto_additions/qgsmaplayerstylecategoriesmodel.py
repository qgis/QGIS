# The following has been generated automatically from src/gui/qgsmaplayerstylecategoriesmodel.h
# monkey patching scoped based enum
QgsMapLayerStyleCategoriesModel.Role.NameRole.__doc__ = ""
QgsMapLayerStyleCategoriesModel.Role.__doc__ = """Custom model roles

* ``NameRole``: 

"""
# --
try:
    QgsMapLayerStyleCategoriesModel.__overridden_methods__ = ['rowCount', 'columnCount', 'data', 'setData', 'flags']
except (NameError, AttributeError):
    pass
try:
    QgsCategoryDisplayLabelDelegate.__overridden_methods__ = ['drawDisplay', 'sizeHint']
except (NameError, AttributeError):
    pass
