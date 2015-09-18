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
#include "qgscomposertablev2.h"

/** A text table item that reads text from string lists*/
class CORE_EXPORT QgsComposerTextTable: public QgsComposerTable
{
  public:
    QgsComposerTextTable( QgsComposition* c );
    ~QgsComposerTextTable();

    /** Return correct graphics item type. */
    virtual int type() const override { return ComposerTextTable; }

    /** Sets the text to use for the header row for the table
     * @param labels list of strings to use for each column's header row
     * @see headerLabels
    */
    void setHeaderLabels( const QStringList& labels );

    /** Adds a row to the table
     * @param row list of strings to use for each cell's value in the newly added row
     * @note If row is shorter than the number of columns in the table than blank cells
     * will be inserted at the end of the row. If row contains more strings then the number
     * of columns in the table then these extra strings will be ignored.
    */
    void addRow( const QStringList& row ) { mRowText.append( row ); }

    /** Writes properties specific to text tables
     * @param elem an existing QDomElement in which to store the text table's properties.
     * @param doc QDomDocument for the destination xml.
     * @see readXML
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const override;

    /** Reads the properties specific to a text table from xml.
     * @param itemElem a QDomElement holding the text table's desired properties.
     * @param doc QDomDocument for the source xml.
     * @see writeXML
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override;

    /** Queries the text table for text to show in the cells.
     * @param attributeMaps list of QgsAttributeMaps where the cell text will be stored
     * @returns true if attribute values were successfully set from table's text
     * @note not available in python bindings
     */
    bool getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps ) override;

  private:
    /** One stringlist per row*/
    QList< QStringList > mRowText;
};

/** A text table item that reads text from string lists
 * @note added in QGIS 2.10
*/
class CORE_EXPORT QgsComposerTextTableV2 : public QgsComposerTableV2
{

    Q_OBJECT

  public:
    QgsComposerTextTableV2( QgsComposition* c, bool createUndoCommands );
    ~QgsComposerTextTableV2();

    /** Adds a row to the table
     * @param row list of strings to use for each cell's value in the newly added row
     * @note If row is shorter than the number of columns in the table than blank cells
     * will be inserted at the end of the row. If row contains more strings then the number
     * of columns in the table then these extra strings will be ignored.
     * @note if adding many rows, @link setContents @endlink is much faster
    */
    void addRow( const QStringList& row );

    /** Sets the contents of the text table.
     * @param contents list of table rows
     * @see addRow
     */
    void setContents( const QList< QStringList >& contents );

    bool getTableContents( QgsComposerTableContents &contents ) override;

    virtual void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true ) override;

  private:
    /** One stringlist per row*/
    QList< QStringList > mRowText;
};

#endif // QGSCOMPOSERTEXTTABLE_H
