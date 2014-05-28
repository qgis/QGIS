/***************************************************************************
                         qgscomposertablecolumn.h
                         ------------------
    begin                : May 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#ifndef QGSCOMPOSERTABLECOLUMN_H
#define QGSCOMPOSERTABLECOLUMN_H

#include "qgscomposertable.h"
#include <QObject>

/**Stores properties of a column in a QgsComposerTable. Some properties of a QgsComposerTableColumn
are applicable only in certain contexts. For instance, the attribute and setAttribute methods only
have an effect for QgsComposerAttributeTables, and have no effect for QgsComposerTextTables.*/
class CORE_EXPORT QgsComposerTableColumn: public QObject
{
    Q_OBJECT

  public:

    QgsComposerTableColumn();
    virtual ~QgsComposerTableColumn();

    /**Writes the column's properties to xml for storage.
     * @param columnElem an existing QDomElement in which to store the column's properties.
     * @param doc QDomDocument for the destination xml.
     * @note added in 2.3
     * @see readXML
     */
    virtual bool writeXML( QDomElement& columnElem, QDomDocument & doc ) const;

    /**Reads the column's properties from xml.
     * @param columnElem a QDomElement holding the column's desired properties.
     * @note added in 2.3
     * @see writeXML
     */
    virtual bool readXML( const QDomElement& columnElem );

    /**Returns the heading for a column, which is the value displayed in the columns
     * header cell.
     * @returns Heading for column.
     * @note added in 2.3
     * @see setHeading
     */
    QString heading() const { return mHeading; }

    /**Sets the heading for a column, which is the value displayed in the columns
     * header cell.
     * @param heading Heading for column.
     * @note added in 2.3
     * @see heading
     */
    void setHeading( QString heading ) { mHeading = heading; }

    /**Returns the horizontal alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * @returns horizontal alignment.
     * @note added in 2.3
     * @see setHAlignment
     */
    Qt::AlignmentFlag hAlignment() const { return mHAlignment; }

    /**Sets the horizontal alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * @param alignment horizontal alignment for cell.
     * @note added in 2.3
     * @see hAlignment
     */
    void setHAlignment( Qt::AlignmentFlag alignment ) { mHAlignment = alignment; }

    /**Returns the attribute name or expression used for the column's values. This property
     * is only used when the column is part of a QgsComposerAttributeTable.
     * @returns attribute name or expression text for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see setAttribute
     */
    QString attribute() const { return mAttribute; }

    /**Sets the attribute name or expression used for the column's values. This property
     * is only used when the column is part of a QgsComposerAttributeTable.
     * @param attribute attribute name or expression text for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see attribute
     */
    void setAttribute( QString attribute ) { mAttribute = attribute; }

    /**Returns the sort order for the column. This property is only used when the column
     * is part of a QgsComposerAttributeTable and when sortByRank is > 0.
     * @returns sort order for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see setSortOrder
     * @see sortByRank
     */
    Qt::SortOrder sortOrder() const { return mSortOrder; }

    /**Sets the sort order for the column. This property is only used when the column
     * is part of a QgsComposerAttributeTable and when sortByRank is > 0.
     * @param sortOrder sort order for column
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see sortOrder
     * @see setSortByRank
     */
    void setSortOrder( Qt::SortOrder sortOrder ) { mSortOrder = sortOrder; }

    /**Returns the sort rank for the column. If the sort rank is > 0 then the column
     * will be sorted in the table. The sort rank specifies the priority given to the
     * column when the table is sorted by multiple columns, with lower sort ranks
     * having higher priority. This property is only used when the column
     * is part of a QgsComposerAttributeTable.
     * @returns sort rank for column. If sort rank is <= 0 then the column is not being
     * sorted.
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see setSortByRank
     * @see sortOrder
     */
    int sortByRank() const { return mSortByRank; }

    /**Sets the sort rank for the column. If the sort rank is > 0 then the column
     * will be sorted in the table. The sort rank specifies the priority given to the
     * column when the table is sorted by multiple columns, with lower sort ranks
     * having higher priority. This property is only used when the column
     * is part of a QgsComposerAttributeTable.
     * @param sortByRank sort rank for column. If sort rank is <= 0 then the column is not being
     * sorted.
     * @note added in 2.3
     * @note only applicable when used in a QgsComposerAttributeTable
     * @see sortByRank
     * @see setSortOrder
     */
    void setSortByRank( int sortByRank ) { mSortByRank = sortByRank; }

    /**Creates a duplicate column which is a deep copy of this column.
     * @returns a new QgsComposerTableColumn with same properties as this column.
     * @note added in 2.3
     */
    QgsComposerTableColumn* clone();

  private:

    QColor mBackgroundColor; //curently unused
    Qt::AlignmentFlag mHAlignment;
    QString mHeading;
    QString mAttribute;
    int mSortByRank;
    Qt::SortOrder mSortOrder;

};

#endif // QGSCOMPOSERTABLECOLUMN_H
