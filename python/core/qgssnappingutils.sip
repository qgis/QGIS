/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgssnappingutils.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSnappingUtils : QObject
{
%Docstring
 This class has all the configuration of snapping and can return answers to snapping queries.
 Internally, it keeps a cache of QgsPointLocator instances for multiple layers.

 Currently it supports the following queries:
 - snapToMap() - has multiple modes of operation
 - snapToCurrentLayer()
 For more complex queries it is possible to use locatorForLayer() method that returns
 point locator instance with layer's indexed data.

 Indexing strategy determines how fast the queries will be and how much memory will be used.

 When working with map canvas, it may be useful to use derived class QgsMapCanvasSnappingUtils
 which keeps the configuration in sync with map canvas (e.g. current view, active layer).

.. versionadded:: 2.8
%End

%TypeHeaderCode
#include "qgssnappingutils.h"
%End
  public:
    QgsSnappingUtils( QObject *parent /TransferThis/ = 0 );
    ~QgsSnappingUtils();


    QgsPointLocator *locatorForLayer( QgsVectorLayer *vl );
%Docstring
Get a point locator for the given layer. If such locator does not exist, it will be created
 :rtype: QgsPointLocator
%End

    QgsPointLocator::Match snapToMap( QPoint point, QgsPointLocator::MatchFilter *filter = 0 );
%Docstring
Snap to map according to the current configuration. Optional filter allows discarding unwanted matches.
 :rtype: QgsPointLocator.Match
%End
    QgsPointLocator::Match snapToMap( const QgsPointXY &pointMap, QgsPointLocator::MatchFilter *filter = 0 );
%Docstring
 :rtype: QgsPointLocator.Match
%End

    QgsPointLocator::Match snapToCurrentLayer( QPoint point, QgsPointLocator::Types type, QgsPointLocator::MatchFilter *filter = 0 );
%Docstring
Snap to current layer
 :rtype: QgsPointLocator.Match
%End


    void setMapSettings( const QgsMapSettings &settings );
%Docstring
Assign current map settings to the utils - used for conversion between screen coords to map coords
%End
    QgsMapSettings mapSettings() const;
%Docstring
 :rtype: QgsMapSettings
%End

    void setCurrentLayer( QgsVectorLayer *layer );
%Docstring
Set current layer so that if mode is SnapCurrentLayer we know which layer to use
%End
    QgsVectorLayer *currentLayer() const;
%Docstring
The current layer used if mode is SnapCurrentLayer
 :rtype: QgsVectorLayer
%End


    enum IndexingStrategy
    {
      IndexAlwaysFull,
      IndexNeverFull,
      IndexHybrid,
      IndexExtent
    };

    void setIndexingStrategy( IndexingStrategy strategy );
%Docstring
Set a strategy for indexing geometry data - determines how fast and memory consuming the data structures will be
%End
    IndexingStrategy indexingStrategy() const;
%Docstring
Find out which strategy is used for indexing - by default hybrid indexing is used
 :rtype: IndexingStrategy
%End

    struct LayerConfig
    {

      LayerConfig( QgsVectorLayer *l, QgsPointLocator::Types t, double tol, QgsTolerance::UnitType u );
%Docstring
 Create a new configuration for a snapping layer.

```py
snapper = QgsMapCanvasSnappingUtils(mapCanvas)

snapping_layer1 = QgsSnappingUtils.LayerConfig(layer1, QgsPointLocator.Vertex, 10, QgsTolerance.Pixels)
snapping_layer2 = QgsSnappingUtils.LayerConfig(layer2, QgsPointLocator.Vertex and QgsPointLocator.Edge, 10, QgsTolerance.Pixels)

snapper.setLayers([snapping_layer1, snapping_layer2])
```

 \param l   The vector layer for which this configuration is
 \param t   Which parts of the geometry should be snappable
 \param tol The tolerance radius in which the snapping will trigger
 \param u   The unit in which the tolerance is specified
%End

      bool operator==( const QgsSnappingUtils::LayerConfig &other ) const;
      bool operator!=( const QgsSnappingUtils::LayerConfig &other ) const;
%Docstring
 :rtype: bool
%End

      QgsVectorLayer *layer;
%Docstring
The layer to configure.
%End
      QgsPointLocator::Types type;
%Docstring
To which geometry properties of this layers a snapping should happen.
%End
      double tolerance;
%Docstring
The range around snapping targets in which snapping should occur.
%End
      QgsTolerance::UnitType unit;
%Docstring
The units in which the tolerance is specified.
%End
    };

    QList<QgsSnappingUtils::LayerConfig> layers() const;
%Docstring
Query layers used for snapping
 :rtype: list of QgsSnappingUtils.LayerConfig
%End

    QString dump();
%Docstring
 Get extra information about the instance
.. versionadded:: 2.14
 :rtype: str
%End

    QgsSnappingConfig config() const;
%Docstring
 The snapping configuration controls the behavior of this object
 :rtype: QgsSnappingConfig
%End

  public slots:

    void setConfig( const QgsSnappingConfig &snappingConfig );
%Docstring
 The snapping configuration controls the behavior of this object
%End

    void toggleEnabled();
%Docstring
 Toggles the state of snapping

.. versionadded:: 3.0
%End

  signals:

    void configChanged( const QgsSnappingConfig &snappingConfig );
%Docstring
 Emitted when the snapping settings object changes.
%End

  protected:
    virtual void prepareIndexStarting( int count );
%Docstring
Called when starting to index - can be overridden and e.g. progress dialog can be provided
%End
    virtual void prepareIndexProgress( int index );
%Docstring
Called when finished indexing a layer. When index == count the indexing is complete
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgssnappingutils.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
