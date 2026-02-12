# The following has been generated automatically from src/core/qgsexpressioncontext.h
try:
    QgsExpressionContextScope.StaticVariable.__attribute_docs__ = {'name': 'Variable name', 'value': 'Variable value', 'readOnly': 'True if variable should not be editable by users', 'isStatic': 'A static variable can be cached for the lifetime of a context', 'description': 'Translated description of variable, for use within expression builder widgets.'}
    QgsExpressionContextScope.StaticVariable.__annotations__ = {'name': str, 'value': 'object', 'readOnly': bool, 'isStatic': bool, 'description': str}
    QgsExpressionContextScope.StaticVariable.__doc__ = """Single variable definition for use within a QgsExpressionContextScope."""
except (NameError, AttributeError):
    pass
try:
    QgsExpressionContext.__attribute_docs__ = {'EXPR_FIELDS': 'Inbuilt variable name for fields storage', 'EXPR_ORIGINAL_VALUE': 'Inbuilt variable name for value original value variable', 'EXPR_SYMBOL_COLOR': 'Inbuilt variable name for symbol color variable', 'EXPR_SYMBOL_ANGLE': 'Inbuilt variable name for symbol angle variable', 'EXPR_GEOMETRY_PART_COUNT': 'Inbuilt variable name for geometry part count variable', 'EXPR_GEOMETRY_PART_NUM': 'Inbuilt variable name for geometry part number variable', 'EXPR_GEOMETRY_RING_NUM': 'Inbuilt variable name for geometry ring number variable.\n\n.. versionadded:: 3.20', 'EXPR_GEOMETRY_POINT_COUNT': 'Inbuilt variable name for point count variable', 'EXPR_GEOMETRY_POINT_NUM': 'Inbuilt variable name for point number variable', 'EXPR_CLUSTER_SIZE': 'Inbuilt variable name for cluster size variable', 'EXPR_CLUSTER_COLOR': 'Inbuilt variable name for cluster color variable'}
    QgsExpressionContext.__annotations__ = {'EXPR_FIELDS': str, 'EXPR_ORIGINAL_VALUE': str, 'EXPR_SYMBOL_COLOR': str, 'EXPR_SYMBOL_ANGLE': str, 'EXPR_GEOMETRY_PART_COUNT': str, 'EXPR_GEOMETRY_PART_NUM': str, 'EXPR_GEOMETRY_RING_NUM': str, 'EXPR_GEOMETRY_POINT_COUNT': str, 'EXPR_GEOMETRY_POINT_NUM': str, 'EXPR_CLUSTER_SIZE': str, 'EXPR_CLUSTER_COLOR': str}
except (NameError, AttributeError):
    pass
try:
    QgsScopedExpressionFunction.__abstract_methods__ = ['func', 'clone']
    QgsScopedExpressionFunction.__overridden_methods__ = ['func', 'usesGeometry', 'referencedColumns', 'isStatic']
except (NameError, AttributeError):
    pass
