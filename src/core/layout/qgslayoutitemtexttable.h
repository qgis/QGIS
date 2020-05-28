/***************************************************************************
                         qgslayoutitemtexttable.h
                         ----------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTTEXTTABLE_H
#define QGSLAYOUTTEXTTABLE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayouttable.h"

/**
 * \ingroup core
 * A text table item that reads text from string lists
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutItemTextTable : public QgsLayoutTable
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemTextTable, for the specified \a layout.
     *
     * Ownership is transferred to the layout.
     */
    QgsLayoutItemTextTable( QgsLayout *layout SIP_TRANSFERTHIS );

    int type() const override;
    QString displayName() const override;

    /**
     * Returns a new QgsLayoutItemTextTable for the specified parent \a layout.
     */
    static QgsLayoutItemTextTable *create( QgsLayout *layout ) SIP_FACTORY;

    /**
     * Adds a row to the table
     * \param row list of strings to use for each cell's value in the newly added row
     * \note If row is shorter than the number of columns in the table than blank cells
     * will be inserted at the end of the row. If row contains more strings then the number
     * of columns in the table then these extra strings will be ignored.
     * \note if adding many rows, setContents() is much faster
     */
    void addRow( const QStringList &row );

    /**
     * Sets the contents of the text table.
     * \param contents list of table rows
     * \see addRow
     */
    void setContents( const QVector< QStringList > &contents );

    bool getTableContents( QgsLayoutTableContents &contents ) override;

  private:
    //! One stringlist per row
    QVector< QStringList > mRowText;
};

#endif // QGSLAYOUTTEXTTABLE_H
