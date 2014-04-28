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

#include "qgscomposertable.h"

/**A text table item that reads text from string lists*/
class CORE_EXPORT QgsComposerTextTable: public QgsComposerTable
{
  public:
    QgsComposerTextTable( QgsComposition* c );
    ~QgsComposerTextTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerTextTable; }

    /**Sets the text to use for the header row for the table
     * @param labels list of strings to use for each column's header row
     * @see headerLabels
    */
    void setHeaderLabels( const QStringList& labels ) { mHeaderLabels = labels; }

    /**Adds a row to the table
     * @param row list of strings to use for each cell's value in the newly added row
     * @note If row is shorter than the number of columns in the table than blank cells
     * will be inserted at the end of the row. If row contains more strings then the number
     * of columns in the table then these extra strings will be ignored.
    */
    void addRow( const QStringList& row ) { mRowText.append( row ); }

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    QMap<int, QString> headerLabels() const;

    bool getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps );

  private:
    /**Column titles*/
    QStringList mHeaderLabels;
    /**One stringlist per row*/
    QList< QStringList > mRowText;
};

#endif // QGSCOMPOSERTEXTTABLE_H
