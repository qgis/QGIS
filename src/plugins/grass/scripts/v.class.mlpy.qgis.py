#!/usr/bin/env python3
############################################################################
#
# MODULE:    v.class.mlpy
# AUTHOR(S): Vaclav Petras
# PURPOSE:   Classifies features in vecor map.
# COPYRIGHT: (C) 2012 by Vaclav Petras, and the GRASS Development Team
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
############################################################################

# %module
# % description: Vector supervised classification tool which uses attributes as classification parameters (order of columns matters, names not), cat column identifies feature, class_column is excluded from classification parameters.
# % keyword: vector
# % keyword: classification
# % keyword: supervised
# %end
# %option G_OPT_V_MAP
# %  key: input
# %  description: Input vector map (attribute table required)
# %  required: yes
# %  multiple: no
# %end
# %option G_OPT_V_MAP
# %  key: training
# %  description: Training vector map (attribute table required)
# %  required: yes
# %  multiple: no
# %end
# %option G_OPT_V_FIELD
# %  key: class_column
# %  type: string
# %  label: Name of column containing class
# %  description: Used for both input/output and training dataset. If column does not exists in input map attribute table, it will be created.
# %  required: no
# %  multiple: no
# %  answer: class
# %end
# %option
# %  key: columns
# %  type: string
# %  label: Columns to be used in classification
# %  description: Columns to be used in classification. If left empty, all columns will be used for classification except for class_column and cat column.
# %  required: no
# %  multiple: yes
# %end


# TODO: add other classifiers
# TODO: improve doc
# TODO: input/training could be multiple
# TODO: handle layers
# TODO: output to new map (all classes/one class), depens what is faster


import grass.script as grass
import numpy as np


def addColumn(mapName, columnName, columnType):
    """Adds column to the map's table."""
    columnDefinition = columnName + " " + columnType
    grass.run_command("v.db.addcolumn", map=mapName, columns=columnDefinition)


def hasColumn(tableDescription, column):
    """Checks if the column is in the table description

    \todo This should be part of some object in the lib.
    """
    for col in tableDescription["cols"]:
        if col[0] == column:
            return True
    return False


def updateColumn(mapName, column, cats, values=None):
    r"""!Updates column values for rows with a given categories.

    \param cats categories to be updated
    or a list of tuples (cat, value) if \p values is None
    \param values to be set for column (same length as cats) or \c None
    """
    statements = ""
    for i in range(len(cats)):
        if values is None:
            cat = str(cats[i][0])
            val = str(cats[i][1])
        else:
            cat = str(cats[i])
            val = str(values[i])
        statement = "UPDATE " + mapName + " SET "
        statement += column + " = " + val
        statement += " WHERE cat = " + cat
        statements += statement + ";\n"

    grass.write_command("db.execute", input="-", stdin=statements)


class Classifier:
    """!Interface class between mlpy and other code

    It does not uses numpy in the interface bu this may be wrong.
    """

    def __init__(self):
        try:
            import mlpy
        except ImportError:
            grass.fatal(
                _(
                    "Cannot import mlpy (http://mlpy.sourceforge.net)"
                    " library."
                    " Please install it or ensure that it is on path"
                    " (use PYTHONPATH variable)."
                )
            )
        # Pytlit has a problem with this mlpy and v.class.mlpy.py
        # thus, warnings for objects from mlpy has to be disabled
        self.mlclassifier = mlpy.DLDA(delta=0.01)  # pylint: disable=E1101

    def learn(self, values, classes):
        self.mlclassifier.learn(np.array(values), np.array(classes))

    def pred(self, values):
        return self.mlclassifier.pred(np.array(values))


# TODO: raise exception when str can not be float
# TODO: repair those functions, probably create a class
# TODO: use numpy or array
def fromDbTableToSimpleTable(dbTable, columnsDescription, columnWithClass):
    sTable = []
    for row in dbTable:
        sRow = []
        for i, col in enumerate(row):
            columnName = columnsDescription[i][0]
            if columnName != columnWithClass and columnName != "cat":
                sRow.append(float(col))
        sTable.append(sRow)

    return sTable


def extractColumnWithClass(dbTable, columnsDescription, columnWithClass):
    classColumn = []
    for row in dbTable:
        for i, col in enumerate(row):
            columnName = columnsDescription[i][0]
            if columnName == columnWithClass:
                classColumn.append(float(col))

    return classColumn


def extractNthColumn(dbTable, columnNumber):
    classColumn = []
    for row in dbTable:
        for i, col in enumerate(row):
            if columnNumber == i:
                classColumn.append(float(col))

    return classColumn


def extractColumnWithCats(dbTable, columnsDescription):
    column = []
    for row in dbTable:
        for i, col in enumerate(row):
            columnName = columnsDescription[i][0]
            if columnName == "cat":
                column.append(float(col))

    return column


# unused
def fatal_noAttributeTable(mapName):
    grass.fatal(_("Vector map <%s> has no or empty attribute table") % mapName)


def fatal_noEnoughColumns(mapName, ncols, required):
    grass.fatal(
        _(
            "Not enough columns in vector map <%(map)s>"
            " (found %(ncols)s, expected at least %(r)s"
        )
        % {"map": mapName, "ncols": ncols, "r": required}
    )


def fatal_noClassColumn(mapName, columnName):
    grass.fatal(
        _("Vector map <%(map)s> does not have" " the column <%(col)s> containing class")
        % {"map": mapName, "col": columnName}
    )


def fatal_noRows(mapName):
    grass.fatal(_("Empty attribute table for map vector <%(map)s>") % {"map": mapName})


def checkNcols(mapName, tableDescription, requiredNcols):
    ncols = tableDescription["ncols"]
    if ncols < requiredNcols:
        fatal_noEnoughColumns(mapName, ncols, requiredNcols)


def checkNrows(mapName, tableDescription):
    if not tableDescription["nrows"] > 0:
        fatal_noRows(mapName)


def checkDbConnection(mapName):
    """! Checks if vector map has an attribute table.

    \todo check layer
    """
    ret = grass.vector_db(mapName)
    if not ret:
        grass.fatal(_("Vector map <%s> has no attribute table") % mapName)


def main():
    options, unused = grass.parser()

    mapName = options["input"]
    trainingMapName = options["training"]

    columnWithClass = options["class_column"]

    useAllColumns = True
    if options["columns"]:
        # columns as string
        columns = options["columns"].strip()
        useAllColumns = False

    # TODO: allow same input and output map only if --overwrite was specified
    # TODO: is adding column overwriting or overwriting is only updating of existing?

    # variable names connected to training dataset have training prefix
    # variable names connected to classified dataset have no prefix

    # checking database connection (if map has a table)
    # TODO: layer
    checkDbConnection(trainingMapName)
    checkDbConnection(mapName)

    # loading descriptions first to check them

    trainingTableDescription = grass.db_describe(table=trainingMapName)

    if useAllColumns:
        trainingMinNcols = 3
        checkNcols(trainingMapName, trainingTableDescription, trainingMinNcols)
    else:
        pass

    checkNrows(trainingMapName, trainingTableDescription)

    if not hasColumn(trainingTableDescription, columnWithClass):
        fatal_noClassColumn(trainingMapName, columnWithClass)

    tableDescription = grass.db_describe(table=mapName)

    if useAllColumns:
        minNcols = 2
        checkNcols(mapName, tableDescription, minNcols)
    else:
        pass

    checkNrows(mapName, tableDescription)

    # TODO: check same (+-1) number of columns

    # loadnig data

    # TODO: make fun from this
    if useAllColumns:
        dbTable = grass.db_select(table=trainingMapName)
    else:
        # assuming that columns concatenated by comma
        sql = f"SELECT {columnWithClass},{columns} FROM {trainingMapName}"
        dbTable = grass.db_select(sql=sql)

    trainingParameters = fromDbTableToSimpleTable(
        dbTable,
        columnsDescription=trainingTableDescription["cols"],
        columnWithClass=columnWithClass,
    )

    if useAllColumns:
        trainingClasses = extractColumnWithClass(
            dbTable,
            columnsDescription=trainingTableDescription["cols"],
            columnWithClass=columnWithClass,
        )
    else:
        # FIXME: magic num?
        trainingClasses = extractNthColumn(dbTable, 0)

    # TODO: hard coded 'cat'?
    if useAllColumns:
        dbTable = grass.db_select(table=mapName)
    else:
        # assuming that columns concatenated by comma
        sql = "SELECT {},{} FROM {}".format("cat", columns, mapName)
        dbTable = grass.db_select(sql=sql)

    parameters = fromDbTableToSimpleTable(
        dbTable,
        columnsDescription=tableDescription["cols"],
        columnWithClass=columnWithClass,
    )
    if useAllColumns:
        cats = extractColumnWithCats(
            dbTable, columnsDescription=tableDescription["cols"]
        )
    else:
        cats = extractNthColumn(dbTable, 0)

    # since dbTable can be big it is better to avoid to have it in memory twice
    del dbTable
    del trainingTableDescription

    classifier = Classifier()
    classifier.learn(trainingParameters, trainingClasses)
    classes = classifier.pred(parameters)

    # add column only if not exists and the classification was successful
    if not hasColumn(tableDescription, columnWithClass):
        addColumn(mapName, columnWithClass, "int")

    updateColumn(mapName, columnWithClass, cats, classes)

    # TODO: output as a new map (use INSERT, can be faster)
    # TODO: output as a new layer?


if __name__ == "__main__":
    main()
