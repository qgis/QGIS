/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslegendrenderer.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLegendRenderer
{
%Docstring
The QgsLegendRenderer class handles automatic layout and rendering of legend.
The content is given by QgsLayerTreeModel instance. Various layout properties can be configured
within QgsLegendRenderer.

All spacing and sizes are in millimeters.

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslegendrenderer.h"
%End
  public:

    QgsLegendRenderer( QgsLayerTreeModel *legendModel, const QgsLegendSettings &settings );
%Docstring
Constructor for QgsLegendRenderer. The ownership of the legend model is not changed,
and the model must exist for the lifetime of this renderer.
%End

    QSizeF minimumSize( QgsRenderContext *renderContext = 0 );
%Docstring
Runs the layout algorithm and returns the minimum size required for the legend.

.. seealso:: :py:func:`setLegendSize`

.. seealso:: :py:func:`legendSize`
%End

    void setLegendSize( QSizeF s );
%Docstring
Sets the preferred resulting legend size.

If the size is null, the legend will be drawn with the minimum possible size to fit its content.

.. seealso:: :py:func:`legendSize`

.. seealso:: :py:func:`minimumSize`
%End

    QSizeF legendSize() const;
%Docstring
Returns the preferred legend size set by the client.

If the returned size is null, the legend will be drawn with the minimum possible size to fit its content.

.. seealso:: :py:func:`minimumSize`

.. seealso:: :py:func:`setLegendSize`
%End

    void drawLegend( QPainter *painter );
%Docstring
Draws the legend with given ``painter``. The legend will occupy the area reported in legendSize().
The ``painter`` should be scaled beforehand so that units correspond to millimeters.
%End

    void drawLegend( QgsRenderContext &context );
%Docstring
Draws the legend using a given render ``context``. The legend will occupy the area reported in legendSize().

.. versionadded:: 3.6
%End

    void exportLegendToJson( const QgsRenderContext &context, QJsonObject &json );
%Docstring
Renders the legend in a ``json`` object.

.. versionadded:: 3.8
%End

    static void setNodeLegendStyle( QgsLayerTreeNode *node, QgsLegendStyle::Style style );
%Docstring
Sets the ``style`` of a ``node``.

.. seealso:: :py:func:`nodeLegendStyle`
%End

    static QgsLegendStyle::Style nodeLegendStyle( QgsLayerTreeNode *node, QgsLayerTreeModel *model );
%Docstring
Returns the style for the given ``node``, within the specified ``model``.

.. seealso:: :py:func:`setNodeLegendStyle`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslegendrenderer.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
