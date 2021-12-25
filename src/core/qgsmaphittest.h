/***************************************************************************
    qgsmaphittest.h
    ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPHITTEST_H
#define QGSMAPHITTEST_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmapsettings.h"
#include "qgsgeometry.h"

#include <QSet>

class QgsRenderContext;
class QgsSymbol;
class QgsVectorLayer;
class QgsMapLayer;
class QgsExpression;

/**
 * \ingroup core
 * \brief Class that runs a hit test with given map settings. Based on the hit test it returns which symbols
 * will be visible on the map - this is useful for content based legend.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsMapHitTest
{
  public:
    //! Maps an expression string to a layer id
    typedef QMap<QString, QString> LayerFilterExpression;

    /**
     * \param settings Map settings used to evaluate symbols
     * \param polygon Polygon geometry to refine the hit test
     * \param layerFilterExpression Expression string for each layer id to evaluate in order to refine the symbol selection
     */
    QgsMapHitTest( const QgsMapSettings &settings, const QgsGeometry &polygon = QgsGeometry(), const QgsMapHitTest::LayerFilterExpression &layerFilterExpression = QgsMapHitTest::LayerFilterExpression() );

    //! Constructor version used with only expressions to filter symbols (no extent or polygon intersection)
    QgsMapHitTest( const QgsMapSettings &settings, const QgsMapHitTest::LayerFilterExpression &layerFilterExpression );

    //! Runs the map hit test
    void run();

    /**
     * Tests whether a symbol is visible for a specified layer.
     * \param symbol symbol to find
     * \param layer vector layer
     * \see legendKeyVisible()
     * \since QGIS 2.12
     */
    bool symbolVisible( QgsSymbol *symbol, QgsVectorLayer *layer ) const;

    /**
     * Tests whether a given legend key is visible for a specified layer.
     * \param ruleKey legend rule key
     * \param layer vector layer
     * \see symbolVisible()
     * \since QGIS 2.14
     */
    bool legendKeyVisible( const QString &ruleKey, QgsVectorLayer *layer ) const;

    /**
     * Tests whether a map layer is visible.
     * \param layer QgsMapLayer
     * \since QGIS 3.24
     */
    bool layerVisible( QgsMapLayer *layer ) const;

  private:

    //! \note not available in Python bindings
    typedef QSet<QString> SymbolSet;

    //! \note not available in Python bindings
    typedef QMap<QgsVectorLayer *, SymbolSet> HitTest;

    /**
     * Runs test for visible symbols within a layer
     * \param vl vector layer
     * \param usedSymbols set for storage of visible symbols
     * \param usedSymbolsRuleKey set of storage of visible legend rule keys
     * \param context render context
     * \note not available in Python bindings
     * \since QGIS 2.12
     */
    void runHitTestLayer( QgsVectorLayer *vl, SymbolSet &usedSymbols, SymbolSet &usedSymbolsRuleKey, QgsRenderContext &context );

    //! The initial map settings
    QgsMapSettings mSettings;

    //! The hit test
    HitTest mHitTest;

    //! The hit test, using legend rule keys
    HitTest mHitTestRuleKey;

    //! List of expression filter for each layer
    QgsMapHitTest::LayerFilterExpression mLayerFilterExpression;

    //! Polygon used for filtering items. May be empty
    QgsGeometry mPolygon;

    //! Whether to use only expressions during the filtering
    bool mOnlyExpressions;
};

#endif // QGSMAPHITTEST_H
