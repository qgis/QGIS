/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreemapcanvasbridge.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayerTreeMapCanvasBridge : QObject
{
%Docstring
 The QgsLayerTreeMapCanvasBridge class takes care of updates of layer set
 for QgsMapCanvas from a layer tree. The class listens to the updates in the layer tree
 and updates the list of layers for rendering whenever some layers are added, removed,
 or their visibility changes.

 The update of layers is not done immediately - it is postponed, so a series of updates
 to the layer tree will trigger just one update of canvas layers.

 Also allows the client to override the default order of layers. This is useful
 in advanced cases where the grouping in layer tree should be independent from the actual
 order in the canvas.

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreemapcanvasbridge.h"
%End
  public:
    QgsLayerTreeMapCanvasBridge( QgsLayerTree *root, QgsMapCanvas *canvas, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor: does not take ownership of the layer tree nor canvas
%End

    QgsLayerTree *rootGroup() const;
%Docstring
 :rtype: QgsLayerTree
%End
    QgsMapCanvas *mapCanvas() const;
%Docstring
 :rtype: QgsMapCanvas
%End

    void setOvervewCanvas( QgsMapOverviewCanvas *overviewCanvas );
%Docstring
.. versionadded:: 3.0
%End
    QgsMapOverviewCanvas *overviewCanvas() const;
%Docstring
.. versionadded:: 3.0
 :rtype: QgsMapOverviewCanvas
%End

    void setAutoSetupOnFirstLayer( bool enabled );
%Docstring
when first layer(s) are added
%End
    bool autoSetupOnFirstLayer() const;
%Docstring
 :rtype: bool
%End

    void setCanvasLayers();
%Docstring
force update of canvas layers from the layer tree. Normally this should not be needed to be called.
%End

  signals:

    void canvasLayersChanged( const QList< QgsMapLayer * > &layers );
%Docstring
 Emitted when the set of layers (or order of layers) visible in the
 canvas changes.
.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreemapcanvasbridge.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
