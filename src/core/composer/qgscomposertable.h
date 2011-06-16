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
#include "qgsfeature.h"
#include <QSet>



/**A class to display feature attributes in the print composer*/
class CORE_EXPORT QgsComposerTable: public QgsComposerItem
{
  public:
    QgsComposerTable( QgsComposition* composition );
    virtual ~QgsComposerTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerTable; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const = 0;
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) = 0;

    void setLineTextDistance( double d ) { mLineTextDistance = d; }
    double lineTextDistance() const { return mLineTextDistance; }

    void setHeaderFont( const QFont& f ) { mHeaderFont = f;}
    QFont headerFont() const { return mHeaderFont; }

    void setContentFont( const QFont& f ) { mContentFont = f; }
    QFont contentFont() const { return mContentFont; }

    void setShowGrid( bool show ) { mShowGrid = show;}
    bool showGrid() const { return mShowGrid; }

    void setGridStrokeWidth( double w ) { mGridStrokeWidth = w; }
    double gridStrokeWidth() const { return mGridStrokeWidth; }

    void setGridColor( const QColor& c ) { mGridColor = c; }
    QColor gridColor() const { return mGridColor; }

    /**Adapts the size of the frame to match the content. This is normally done in the paint method, but sometimes \
    it needs to be done before the first render*/
    void adjustFrameToSize();

  protected:
    /**Distance between table lines and text*/
    double mLineTextDistance;

    QFont mHeaderFont;
    QFont mContentFont;

    bool mShowGrid;
    double mGridStrokeWidth;
    QColor mGridColor;

    /**Retrieves feature attributes*/
    virtual bool getFeatureAttributes( QList<QgsAttributeMap>& attributes )
    { Q_UNUSED( attributes ); return false; } //= 0;
    virtual QMap<int, QString> getHeaderLabels() const { return QMap<int, QString>(); } //= 0;
    /**Calculate the maximum width values of the vector attributes*/
    virtual bool calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList ) const;
    /**Adapts the size of the item frame to match the content*/
    void adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList );
    void drawHorizontalGridLines( QPainter* p, int nAttributes );
    void drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap );

    bool tableWriteXML( QDomElement& itemElem, QDomDocument& doc ) const;
    bool tableReadXML( const QDomElement& itemElem, const QDomDocument& doc );
};

#endif // QGSCOMPOSERTABLE_H
