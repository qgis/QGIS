from qgis.core import qgsfunction


@qgsfunction(group="Custom", referenced_columns=[])
def mychoice2(value1, value2):
    return value1 + value2
