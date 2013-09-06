/***************************************************************************
                             qgsatlascomposermap.h
                             ---------------------
    begin                : October 2012
    copyright            : (C) 2005 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATLASCOMPOSITION_H
#define QGSATLASCOMPOSITION_H

#include "qgscoordinatetransform.h"
#include "qgsfeature.h"

#include <memory>
#include <QString>
#include <QDomElement>
#include <QDomDocument>

class QgsComposerMap;
class QgsComposition;
class QgsVectorLayer;
class QgsExpression;

/** \ingroup MapComposer
 * Class used to render an Atlas, iterating over geometry features.
 * prepareForFeature() modifies the atlas map's extent to zoom on the given feature.
 * This class is used for printing, exporting to PDF and images.
 * */
class CORE_EXPORT QgsAtlasComposition : public QObject
{
    Q_OBJECT
  public:
    QgsAtlasComposition( QgsComposition* composition );
    ~QgsAtlasComposition();

    /** Is the atlas generation enabled ? */
    bool enabled() const { return mEnabled; }
    void setEnabled( bool e ) { mEnabled = e; }

    QgsComposerMap* composerMap() const { return mComposerMap; }
    void setComposerMap( QgsComposerMap* map ) { mComposerMap = map; }

    bool hideCoverage() const { return mHideCoverage; }
    void setHideCoverage( bool hide ) { mHideCoverage = hide; }

    bool fixedScale() const { return mFixedScale; }
    void setFixedScale( bool fixed ) { mFixedScale = fixed; }

    float margin() const { return mMargin; }
    void setMargin( float margin ) { mMargin = margin; }

    QString filenamePattern() const { return mFilenamePattern; }
    void setFilenamePattern( const QString& pattern ) { mFilenamePattern = pattern; }

    QgsVectorLayer* coverageLayer() const { return mCoverageLayer; }
    void setCoverageLayer( QgsVectorLayer* lmap );

    bool singleFile() const { return mSingleFile; }
    void setSingleFile( bool single ) { mSingleFile = single; }

    bool sortFeatures() const { return mSortFeatures; }
    void setSortFeatures( bool doSort ) { mSortFeatures = doSort; }

    bool sortAscending() const { return mSortAscending; }
    void setSortAscending( bool ascending ) { mSortAscending = ascending; }

    bool filterFeatures() const { return mFilterFeatures; }
    void setFilterFeatures( bool doFilter ) { mFilterFeatures = doFilter; }

    QString featureFilter() const { return mFeatureFilter; }
    void setFeatureFilter( const QString& expression ) { mFeatureFilter = expression; }

    int sortKeyAttributeIndex() const { return mSortKeyAttributeIdx; }
    void setSortKeyAttributeIndex( int idx ) { mSortKeyAttributeIdx = idx; }

    /** Begins the rendering. */
    void beginRender();
    /** Ends the rendering. Restores original extent */
    void endRender();

    /** Returns the number of features in the coverage layer */
    int numFeatures() const;

    /** Prepare the atlas map for the given feature. Sets the extent and context variables */
    void prepareForFeature( int i );

    /** Returns the current filename. Must be called after prepareForFeature( i ) */
    const QString& currentFilename() const;

    void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    void readXML( const QDomElement& elem, const QDomDocument& doc );

    QgsComposition* composition() { return mComposition; }

  signals:
    /** emitted when one of the parameters changes */
    void parameterChanged();

  private:
    QgsComposition* mComposition;

    bool mEnabled;
    QgsComposerMap* mComposerMap;
    bool mHideCoverage;
    bool mFixedScale;
    double mMargin;
    QString mFilenamePattern;
    QgsVectorLayer* mCoverageLayer;
    bool mSingleFile;

    QgsCoordinateTransform mTransform;
    QString mCurrentFilename;
    // feature ordering
    bool mSortFeatures;
    // sort direction
    bool mSortAscending;
  public:
    typedef QMap< QgsFeatureId, QVariant > SorterKeys;
  private:
    // value of field that is used for ordering of features
    SorterKeys mFeatureKeys;
    // key (attribute index) used for ordering
    int mSortKeyAttributeIdx;

    // feature filtering
    bool mFilterFeatures;
    // feature expression filter
    QString mFeatureFilter;

    // id of each iterated feature (after filtering and sorting)
    QVector<QgsFeatureId> mFeatureIds;

    QgsFeature mCurrentFeature;
    QgsRectangle mOrigExtent;
    bool mRestoreLayer;
    std::auto_ptr<QgsExpression> mFilenameExpr;
};

#endif



