/***************************************************************************
                        QgsLayoutTableColumn.h
                         ------------------
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

#ifndef QGSLAYOUTTABLECOLUMN_H
#define QGSLAYOUTTABLECOLUMN_H

#include <QDomDocument>
#include <QDomElement>
#include <QColor>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * \brief Stores properties of a column for a QgsLayoutTable.
 *
 * Some properties of a QgsLayoutTableColumn are applicable only in certain contexts.
 * For instance, the attribute and setAttribute methods only have an effect
 * for QgsLayoutItemAttributeTables, and have no effect for QgsLayoutItemTextTables.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutTableColumn
{
  public:

    /**
     * Constructor for QgsLayoutTableColumn.
     * \param heading column heading
     */
    QgsLayoutTableColumn( const QString &heading = QString() );

    /**
     * Writes the column's properties to xml for storage.
     * \param columnElem an existing QDomElement in which to store the column's properties.
     * \param doc QDomDocument for the destination xml.
     * \see readXml()
     */
    bool writeXml( QDomElement &columnElem, QDomDocument &doc ) const;

    /**
     * Reads the column's properties from xml.
     * \param columnElem a QDomElement holding the column's desired properties.
     * \see writeXml()
     */
    bool readXml( const QDomElement &columnElem );

    /**
     * Returns the width for the column in mm,
     * or 0 if column width is automatically calculated.
     * \see setWidth()
     */
    double width() const { return mWidth; }

    /**
     * Sets the width for a column in mm. Set the \a width to 0 if the column width is to be automatically calculated.
     * \see width()
     */
    void setWidth( const double width ) { mWidth = width; }

    /**
     * Returns the heading for a column, which is the value displayed in the column's
     * header cell.
     * \see setHeading()
     */
    QString heading() const { return mHeading; }

    /**
     * Sets the \a heading for a column, which is the value displayed in the column's
     * header cell.
     * \see heading()
     */
    void setHeading( const QString &heading ) { mHeading = heading; }

    /**
     * Returns the horizontal alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * \see setHAlignment()
     * \see vAlignment()
     */
    Qt::AlignmentFlag hAlignment() const { return mHAlignment; }

    /**
     * Sets the horizontal \a alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * \see hAlignment()
     * \see setVAlignment()
     */
    void setHAlignment( Qt::AlignmentFlag alignment ) { mHAlignment = alignment; }

    /**
     * Returns the vertical alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * \see setVAlignment()
     * \see hAlignment()
     */
    Qt::AlignmentFlag vAlignment() const { return mVAlignment; }

    /**
     * Sets the vertical \a alignment for a column, which controls the alignment
     * used for drawing column values within cells.
     * \see vAlignment()
     * \see setHAlignment()
     */
    void setVAlignment( Qt::AlignmentFlag alignment ) { mVAlignment = alignment; }

    /**
     * Returns the attribute name or expression used for the column's values. This property
     * is only used when the column is part of a QgsLayoutItemAttributeTable.
     * \note only applicable when used in a QgsLayoutItemAttributeTable
     * \see setAttribute()
     */
    QString attribute() const { return mAttribute; }

    /**
     * Sets the \a attribute name or expression used for the column's values. This property
     * is only used when the column is part of a QgsLayoutItemAttributeTable.
     * \note only applicable when used in a QgsLayoutItemAttributeTable
     * \see attribute()
     */
    void setAttribute( const QString &attribute ) { mAttribute = attribute; }

    /**
     * Returns the sort order for the column. This property is only used when the column
     * is part of a QgsLayoutItemAttributeTable and when sortByRank is > 0.
     * \note only applicable when used in a QgsLayoutItemAttributeTable
     * \see setSortOrder()
     * \see sortByRank()
     */
    Qt::SortOrder sortOrder() const { return mSortOrder; }

    /**
     * Sets the sort \a order for the column. This property is only used when the column
     * is part of a QgsLayoutItemAttributeTable and when sortByRank() is > 0.
     * \note only applicable when used in a QgsLayoutItemAttributeTable
     * \see sortOrder()
     * \see setSortByRank()
     */
    void setSortOrder( Qt::SortOrder order ) { mSortOrder = order; }

    /**
     * Returns the sort rank for the column. If the sort rank is > 0 then the column
     * will be sorted in the table. The sort rank specifies the priority given to the
     * column when the table is sorted by multiple columns, with lower sort ranks
     * having higher priority. This property is only used when the column
     * is part of a QgsLayoutItemAttributeTable.
     *
     * If sort rank is <= 0 then the column is not being sorted.
     *
     * \note only applicable when used in a QgsLayoutItemAttributeTable
     * \see setSortByRank()
     * \see sortOrder()
     * \deprecated since QGIS 3.14 the order is now hold in a dedicated model
     */
    Q_DECL_DEPRECATED int sortByRank() const SIP_DEPRECATED { return mSortByRank; }

    /**
     * Sets the sort \a rank for the column. If the sort rank is > 0 then the column
     * will be sorted in the table. The sort rank specifies the priority given to the
     * column when the table is sorted by multiple columns, with lower sort ranks
     * having higher priority. This property is only used when the column
     * is part of a QgsLayoutItemAttributeTable.
     * If the sort \a rank is <= 0 then the column is not being sorted.
     *
     * \note only applicable when used in a QgsLayoutItemAttributeTable
     * \see sortByRank()
     * \see setSortOrder()
     * \deprecated since QGIS 3.14 the order is now hold in a dedicated model
     */
    Q_DECL_DEPRECATED void setSortByRank( int rank ) SIP_DEPRECATED { mSortByRank = rank; }

    /**
     * Creates a duplicate column which is a deep copy of this column.
     * \returns a new QgsLayoutTableColumn with same properties as this column.
     * \deprecated since QGIS 3.14 use a copy instead
     */
    Q_DECL_DEPRECATED QgsLayoutTableColumn *clone() SIP_DEPRECATED SIP_FACTORY {return new QgsLayoutTableColumn( *this );}

    bool operator==( const QgsLayoutTableColumn &other ) const
    {
      return mHeading == other.mHeading
             && mAttribute == other.mAttribute
             && mSortByRank == other.mSortByRank
             && mSortOrder == other.mSortOrder
             && mWidth == other.mWidth
             && mHAlignment == other.mHAlignment
             && mVAlignment == other.mVAlignment;
    }

  private:

    QString mHeading;
    QString mAttribute;
    int mSortByRank = 0;
    Qt::SortOrder mSortOrder = Qt::AscendingOrder;
    double mWidth = 0.0;
    QColor mBackgroundColor = Qt::transparent; //currently unused
    Qt::AlignmentFlag mHAlignment = Qt::AlignLeft;
    Qt::AlignmentFlag mVAlignment = Qt::AlignVCenter;

    friend class QgsCompositionConverter;

};
#endif //QGSLAYOUTTABLECOLUMN_H
