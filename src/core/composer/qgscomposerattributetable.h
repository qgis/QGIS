/***************************************************************************
                         qgscomposerattributetable.h
                         ---------------------------
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

#ifndef QGSCOMPOSERATTRIBUTETABLE_H
#define QGSCOMPOSERATTRIBUTETABLE_H

#include "qgscomposertable.h"

class QgsComposerMap;
class QgsVectorLayer;

/**Helper class for sorting, takes into account sorting column and ascending / descending*/
class QgsComposerAttributeTableCompare
{
  public:
    QgsComposerAttributeTableCompare();
    bool operator()( const QgsAttributeMap& m1, const QgsAttributeMap& m2 );
    void setSortColumn( int col ) { mCurrentSortColumn = col; }
    void setAscending( bool asc ) { mAscending = asc; }
  private:
    int mCurrentSortColumn;
    bool mAscending;
};

/**A table class that displays a vector attribute table*/
class CORE_EXPORT QgsComposerAttributeTable: public QgsComposerTable
{
    Q_OBJECT
  public:
    QgsComposerAttributeTable( QgsComposition* composition );
    ~QgsComposerAttributeTable();

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void setVectorLayer( QgsVectorLayer* vl );// { mVectorLayer = vl; }
    const QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const { return mComposerMap; }

    void setMaximumNumberOfFeatures( int nr ) { mMaximumNumberOfFeatures = nr; }
    int maximumNumberOfFeatures() const { return mMaximumNumberOfFeatures; }

    void setDisplayOnlyVisibleFeatures( bool b ) { mShowOnlyVisibleFeatures = b; }
    bool displayOnlyVisibleFeatures() const { return mShowOnlyVisibleFeatures; }

    QSet<int> displayAttributes() const { return mDisplayAttributes; }
    void setDisplayAttributes( const QSet<int>& attr ) { mDisplayAttributes = attr;}

    QMap<int, QString> fieldAliasMap() const { return mFieldAliasMap; }
    void setFieldAliasMap( const QMap<int, QString>& map ) { mFieldAliasMap = map; }

    /**Adapts mMaximumNumberOfFeatures depending on the rectangle height*/
    void setSceneRect( const QRectF& rectangle );

    void setSortAttributes( const QList<QPair<int, bool> > att ) { mSortInformation = att; }
    QList<QPair<int, bool> > sortAttributes() const { return mSortInformation; }

  protected:
    /**Retrieves feature attributes*/
    bool getFeatureAttributes( QList<QgsAttributeMap>& attributes );
    QMap<int, QString> getHeaderLabels() const;

  private:
    /**Associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Associated composer map (used to display the visible features)*/
    const QgsComposerMap* mComposerMap;
    /**Maximum number of features that is displayed*/
    int mMaximumNumberOfFeatures;

    /**Shows only the features that are visible in the associated composer map (true by default)*/
    bool mShowOnlyVisibleFeatures;

    /**List of attribute indices to display (or all attributes if list is empty)*/
    QSet<int> mDisplayAttributes;
    /**Map of attribute name aliases. The aliases might be different to those of QgsVectorLayer (but those from the vector layer are the default)*/
    QMap<int, QString> mFieldAliasMap;

    /**Contains information about sort attribute index / ascending (true/false). First entry has the highest priority*/
    QList< QPair<int, bool> > mSortInformation;

    /**Inserts aliases from vector layer as starting configuration to the alias map*/
    void initializeAliasMap();
    /**Returns the attribute name to display in the item (attribute name or an alias if present)*/
    QString attributeDisplayName( int attributeIndex, const QString& name ) const;

  private slots:
    /**Checks if this vector layer will be removed (and sets mVectorLayer to 0 if yes) */
    void removeLayer( QString layerId );

  signals:
    /**This signal is emitted if the maximum number of feature changes (interactively)*/
    void maximumNumerOfFeaturesChanged( int n );
};

#endif // QGSCOMPOSERATTRIBUTETABLE_H
