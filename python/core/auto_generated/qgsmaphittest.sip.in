/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaphittest.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapHitTest
{
%Docstring
Class that runs a hit test with given map settings. Based on the hit test it returns which symbols
will be visible on the map - this is useful for content based legend.

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgsmaphittest.h"
%End
  public:
    typedef QMap<QString, QString> LayerFilterExpression;

    QgsMapHitTest( const QgsMapSettings &settings, const QgsGeometry &polygon = QgsGeometry(), const QgsMapHitTest::LayerFilterExpression &layerFilterExpression = QgsMapHitTest::LayerFilterExpression() );
%Docstring

:param settings: Map settings used to evaluate symbols
:param polygon: Polygon geometry to refine the hit test
:param layerFilterExpression: Expression string for each layer id to evaluate in order to refine the symbol selection
%End

    QgsMapHitTest( const QgsMapSettings &settings, const QgsMapHitTest::LayerFilterExpression &layerFilterExpression );
%Docstring
Constructor version used with only expressions to filter symbols (no extent or polygon intersection)
%End

    void run();
%Docstring
Runs the map hit test
%End

    bool symbolVisible( QgsSymbol *symbol, QgsVectorLayer *layer ) const;
%Docstring
Tests whether a symbol is visible for a specified layer.

:param symbol: symbol to find
:param layer: vector layer

.. versionadded:: 2.12

.. seealso:: :py:func:`legendKeyVisible`
%End

    bool legendKeyVisible( const QString &ruleKey, QgsVectorLayer *layer ) const;
%Docstring
Tests whether a given legend key is visible for a specified layer.

:param ruleKey: legend rule key
:param layer: vector layer

.. versionadded:: 2.14

.. seealso:: :py:func:`symbolVisible`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaphittest.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
