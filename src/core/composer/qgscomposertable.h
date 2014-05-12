/***************************************************************************
                         qgscomposertable.h
                         ------------------
    begin                : January 2010
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

#ifndef QGSCOMPOSERTABLE_H
#define QGSCOMPOSERTABLE_H

#include "qgscomposeritem.h"
#include "qgscomposition.h"
#include "qgsfeature.h"
#include <QSet>



/**A class to display feature attributes in the print composer*/
class CORE_EXPORT QgsComposerTable: public QgsComposerItem
{
    Q_OBJECT

  public:
    QgsComposerTable( QgsComposition* composition );
    virtual ~QgsComposerTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerTable; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const = 0;
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) = 0;

    void setLineTextDistance( double d );
    double lineTextDistance() const { return mLineTextDistance; }

    void setHeaderFont( const QFont& f );
    QFont headerFont() const { return mHeaderFont; }

    void setContentFont( const QFont& f );
    QFont contentFont() const { return mContentFont; }

    void setShowGrid( bool show );
    bool showGrid() const { return mShowGrid; }

    void setGridStrokeWidth( double w );
    double gridStrokeWidth() const { return mGridStrokeWidth; }

    void setGridColor( const QColor& c ) { mGridColor = c; }
    QColor gridColor() const { return mGridColor; }

    /**Returns the text used in the column headers for the table.
     * @returns QMap of int to QString, where the int is the column index (starting at 0),
     * and the string is the text to use for the column's header
     * @note added in 2.3
     * @note not available in python bindings
    */
    virtual QMap<int, QString> headerLabels() const { return QMap<int, QString>(); } //= 0;

    //TODO - make this more generic for next API break, eg rename as getRowValues, use QStringList rather than
    //QgsAttributeMap

    /**Fetches the text used for the rows of the table.
     * @returns true if attribute text was successfully retrieved.
     * @param attributeMaps QList of QgsAttributeMap to store retrieved row data in
     * @note not available in python bindings
    */
    virtual bool getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps ) { Q_UNUSED( attributeMaps ); return false; }

  public slots:

    /**Refreshes the attributes shown in the table by querying the vector layer for new data.
     * This also causes the column widths and size of the table to change to accomodate the
     * new data.
     * @note added in 2.3
     * @see adjustFrameToSize
    */
    virtual void refreshAttributes();

    /**Adapts the size of the frame to match the content. First, the optimal width of the columns
     * is recalculated by checking the maximum width of attributes shown in the table. Then, the
     * table is resized to fit its contents. This slot utilises the table's attribute cache so
     * that a re-query of the vector layer is not required.
     * @note added in 2.3
     * @see refreshAttributes
    */
    virtual void adjustFrameToSize();

  protected:
    /**Distance between table lines and text*/
    double mLineTextDistance;

    QFont mHeaderFont;
    QFont mContentFont;

    bool mShowGrid;
    double mGridStrokeWidth;
    QColor mGridColor;

    QList<QgsAttributeMap> mAttributeMaps;
    QMap<int, double> mMaxColumnWidthMap;

    /**Calculate the maximum width values of the vector attributes*/
    virtual bool calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeMaps ) const;
    /**Adapts the size of the item frame to match the content*/
    //! @note not available in python bindings
    void adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeMaps );
    void drawHorizontalGridLines( QPainter* p, int nAttributes );
    //! @note not available in python bindings
    void drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap );

    bool tableWriteXML( QDomElement& itemElem, QDomDocument& doc ) const;
    bool tableReadXML( const QDomElement& itemElem, const QDomDocument& doc );
};

#endif // QGSCOMPOSERTABLE_H
