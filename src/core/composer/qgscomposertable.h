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

class QgsComposerMap;
class QgsVectorLayer;

/**A class to display feature attributes in the print composer*/
class CORE_EXPORT QgsComposerTable: public QgsComposerItem
{
    Q_OBJECT
  public:
    QgsComposerTable( QgsComposition* composition );
    ~QgsComposerTable();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void setVectorLayer( QgsVectorLayer* vl );// { mVectorLayer = vl; }
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

    QSet<int> displayAttributes() const { return mDisplayAttributes; }
    void setDisplayAttributes( const QSet<int>& attr ) { mDisplayAttributes = attr;}

    QMap<int, QString> fieldAliasMap() const { return mFieldAliasMap; }
    void setFieldAliasMap( const QMap<int, QString>& map ) { mFieldAliasMap = map; }

    /**Adapts mMaximumNumberOfFeatures depending on the rectangle height*/
    void setSceneRect( const QRectF& rectangle );

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

    /**List of attribute indices to display (or all attributes if list is empty)*/
    QSet<int> mDisplayAttributes;
    /**Map of attribute name aliases. The aliases might be different to those of QgsVectorLayer (but those from the vector layer are the default)*/
    QMap<int, QString> mFieldAliasMap;

    /**Retrieves feature attributes*/
    bool getFeatureAttributes( QList<QgsAttributeMap>& attributes );
    /**Calculate the maximum width values of the vector attributes*/
    bool calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList ) const;
    /**Adapts the size of the item frame to match the content*/
    void adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList<QgsAttributeMap>& attributeList );
    void drawHorizontalGridLines( QPainter* p, int nAttributes );
    void drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap );
    /**Inserts aliases from vector layer as starting configuration to the alias map*/
    void initializeAliasMap();
    /**Returns the attribute name to display in the item (attribute name or an alias if present)*/
    QString attributeDisplayName( int attributeIndex, const QString& name ) const;

  signals:
    /**This signal is emitted if the maximum number of feature changes (interactively)*/
    void maximumNumerOfFeaturesChanged( int n );
};

#endif // QGSCOMPOSERTABLE_H
