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

class QgsComposerMap;
class QgsVectorLayer;

/**A class to display feature attributes in the print composer*/
class CORE_EXPORT QgsComposerTable: public QgsComposerItem
{
  public:
    QgsComposerTable( QgsComposition* composition );
    ~QgsComposerTable();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void setVectorLayer( QgsVectorLayer* vl ) { mVectorLayer = vl; }
    const QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const { return mComposerMap; }

    void setMaximumNumberOfFeatures( int nr ) { mMaximumNumberOfFeatures = nr; }
    int maximumNumberOfFeatures() const { return mMaximumNumberOfFeatures; }

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

  private:
    /**Associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Associated composer map (used to display the visible features)*/
    const QgsComposerMap* mComposerMap;
    /**Maximum number of features that is displayed*/
    int mMaximumNumberOfFeatures;
    /**Distance between table lines and text*/
    double mLineTextDistance;

    QFont mHeaderFont;
    QFont mContentFont;

    bool mShowGrid;
    double mGridStrokeWidth;
    QColor mGridColor;

    /**Retrieves feature attributes*/
    bool getFeatureAttributes( QList<QgsAttributeMap>& attributes );
    /**Calculate the maximum width values of the vector attributes*/
    bool calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList ) const;
    /**Adapts the size of the item frame to match the content*/
    void adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList );
    void drawHorizontalGridLines( QPainter* p, int nAttributes );
    void drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap );
};

#endif // QGSCOMPOSERTABLE_H
