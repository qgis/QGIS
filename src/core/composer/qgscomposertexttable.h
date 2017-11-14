/***************************************************************************
                         qgscomposertexttable.h
                         ----------------------
    begin                : April 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERTEXTTABLE_H
#define QGSCOMPOSERTEXTTABLE_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscomposertablev2.h"

/**
 * \ingroup core
 * A text table item that reads text from string lists
 * \since QGIS 2.10
*/
class CORE_EXPORT QgsComposerTextTableV2 : public QgsComposerTableV2
{

    Q_OBJECT

  public:
    QgsComposerTextTableV2( QgsComposition *c SIP_TRANSFERTHIS, bool createUndoCommands );

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
    void setContents( const QList< QStringList > &contents );

    bool getTableContents( QgsComposerTableContents &contents ) override;

    virtual void addFrame( QgsComposerFrame *frame, bool recalcFrameSizes = true ) override;

  private:
    //! One stringlist per row
    QList< QStringList > mRowText;
};

#endif // QGSCOMPOSERTEXTTABLE_H
