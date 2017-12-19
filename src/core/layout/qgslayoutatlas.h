/***************************************************************************
                             qgslayoutatlas.h
                             ----------------
    begin                : December 2017
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
#ifndef QGSLAYOUTATLAS_H
#define QGSLAYOUTATLAS_H

#include "qgis_core.h"
#include "qgsvectorlayerref.h"
#include <QObject>

class QgsLayout;

/**
 * \ingroup core
 * Class used to render an Atlas, iterating over geometry features.
 * prepareForFeature() modifies the atlas map's extent to zoom on the given feature.
 * This class is used for printing, exporting to PDF and images.
 * \note This class should not be created directly. For the atlas to function correctly
 * the atlasComposition() property for QgsComposition should be used to retrieve a
 * QgsLayoutAtlas which is automatically created and attached to the composition.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutAtlas : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for new QgsLayoutAtlas.
     */
    QgsLayoutAtlas( QgsLayout *layout SIP_TRANSFERTHIS );

    /**
     * Returns whether the atlas generation is enabled
     * \see setEnabled()
     */
    bool enabled() const { return mEnabled; }

    /**
     * Sets whether the atlas is \a enabled.
     * \see enabled()
     */
    void setEnabled( bool enabled );

    /**
     * Returns true if the atlas is set to hide the coverage layer.
     * \see setHideCoverage()
     */
    bool hideCoverage() const { return mHideCoverage; }

    /**
     * Sets whether the coverage layer should be hidden in map items in the layouts.
     * \see hideCoverage()
     */
    void setHideCoverage( bool hide );

    /**
     * Returns the filename expression used for generating output filenames for each
     * atlas page.
     * \see setFilenameExpression()
     * \see filenameExpressionErrorString()
     */
    QString filenameExpression() const { return mFilenameExpressionString; }

    /**
     * Sets the filename \a expression used for generating output filenames for each
     * atlas page.
     * If an invalid expression is passed, false will be returned and \a errorString
     * will be set to the expression error.
     * \see filenameExpression()
     */
    bool setFilenameExpression( const QString &expression, QString &errorString );

    /**
     * Returns the coverage layer used for the atlas features.
     * \see setCoverageLayer()
     */
    QgsVectorLayer *coverageLayer() const { return mCoverageLayer.get(); }

    /**
     * Sets the coverage \a layer to use for the atlas features.
     * \see coverageLayer()
     */
    void setCoverageLayer( QgsVectorLayer *layer );

    /**
     * Returns the expression (or field name) used for calculating the page name.
     * \see setPageNameExpression()
     * \see nameForPage()
     */
    QString pageNameExpression() const { return mPageNameExpression; }

    /**
     * Sets the \a expression (or field name) used for calculating the page name.
     * \see pageNameExpression()
     */
    void setPageNameExpression( const QString &expression ) { mPageNameExpression = expression; }

    /**
     * Returns the calculated name for a specified atlas \a page number. Page numbers start at 0.
     * \see pageNameExpression()
     */
    QString nameForPage( int page ) const;

    /**
     * Returns true if features should be sorted in the atlas.
     * \see setSortFeatures()
     * \see sortAscending()
     * \see sortExpression()
     */
    bool sortFeatures() const { return mSortFeatures; }

    /**
     * Sets whether features should be sorted in the atlas.
     * \see sortFeatures()
     * \see setSortAscending()
     * \see setSortExpression()
     */
    void setSortFeatures( bool enabled ) { mSortFeatures = enabled; }

    /**
     * Returns true if features should be sorted in an ascending order.
     *
     * This property has no effect is sortFeatures() is false.
     *
     * \see sortFeatures()
     * \see setSortAscending()
     * \see sortExpression()
     */
    bool sortAscending() const { return mSortAscending; }

    /**
     * Sets whether features should be sorted in an ascending order.
     *
     * This property has no effect is sortFeatures() is false.
     *
     * \see setSortFeatures()
     * \see sortAscending()
     * \see setSortExpression()
     */
    void setSortAscending( bool ascending ) { mSortAscending = ascending; }

    /**
     * Returns the expression (or field name) to use for sorting features.
     *
     * This property has no effect is sortFeatures() is false.
     *
     * \see sortFeatures()
     * \see sortAscending()
     * \see setSortExpression()
     */
    QString sortExpression() const { return mSortExpression; }

    /**
     * Sets the \a expression (or field name) to use for sorting features.
     *
     * This property has no effect is sortFeatures() is false.
     *
     * \see setSortFeatures()
     * \see setSortAscending()
     * \see sortExpression()
     */
    void setSortExpression( const QString &expression ) { mSortExpression = expression; }

    /**
     * Returns true if features should be filtered in the coverage layer.
     * \see filterExpression()
     * \see setFilterExpression()
     */
    bool filterFeatures() const { return mFilterFeatures; }

    /**
     * Sets whether features should be \a filtered in the coverage layer.
     * \see filterFeatures()
     * \see setFilterExpression()
     */
    void setFilterFeatures( bool filtered ) { mFilterFeatures = filtered; }

    /**
     * Returns the expression used for filtering features in the coverage layer.
     *
     * This property has no effect is filterFeatures() is false.
     *
     * \see setFilterExpression()
     * \see filterFeatures()
     */
    QString filterExpression() const { return mFilterExpression; }

    /**
     * Sets the \a expression used for filtering features in the coverage layer.
     *
     * This property has no effect is filterFeatures() is false.
     *
     * If an invalid expression is passed, false will be returned and \a errorString
     * will be set to the expression error.
     *
     * \see filterExpression()
     * \see setFilterFeatures()
     */
    bool setFilterExpression( const QString &expression, QString &errorString );

  public slots:

  signals:

    //! Emitted when one of the atlas parameters changes.
    void changed();

    //! Emitted when atlas is enabled or disabled.
    void toggled( bool );

    //! Emitted when the coverage layer for the atlas changes.
    void coverageLayerChanged( QgsVectorLayer *layer );

  private slots:
    void removeLayers( const QStringList &layers );

  private:

    /**
     * Updates the filename expression.
     * \returns true if expression was successfully parsed, false if expression is invalid
     */
    bool updateFilenameExpression( QString &error );

    /**
     * Evaluates filename for current feature
     * \returns true if feature filename was successfully evaluated
     */
    bool evalFeatureFilename( const QgsExpressionContext &context );

    QPointer< QgsLayout > mLayout;

    bool mEnabled = false;
    bool mHideCoverage = false;
    QString mFilenameExpressionString;

    QgsExpression mFilenameExpression;
    QgsVectorLayerRef mCoverageLayer;

    QString mCurrentFilename;
    bool mSortFeatures = false;
    bool mSortAscending = true;

    typedef QMap< QgsFeatureId, QVariant > SorterKeys;
    // value of field that is used for ordering of features
    SorterKeys mFeatureKeys;

    QString mSortExpression;

    QString mPageNameExpression;

    bool mFilterFeatures = false;
    QString mFilterExpression;

    QString mFilterParserError;

    QgsExpressionContext createExpressionContext();

    friend class AtlasFieldSorter;
};

#endif //QGSLAYOUTATLAS_H



