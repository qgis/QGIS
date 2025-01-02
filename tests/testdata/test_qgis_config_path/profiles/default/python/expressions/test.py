from qgis.core import qgsfunction


@qgsfunction(group="Custom", referenced_columns=[])
def mychoice(value1, value2):
    return value1
