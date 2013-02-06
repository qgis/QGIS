/***************************************************************************
    qgsmaptoolidentify.h  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLIDENTIFY_H
#define QGSMAPTOOLIDENTIFY_H


#include "qgsmaptool.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsdistancearea.h"
#include "qgsmaplayer.h"

#include <QObject>
#include <QPointer>

class QgsRasterLayer;
class QgsVectorLayer;
class QgsMapCanvas;

/**
  \brief Map tool for identifying features in layers

  after selecting a point, performs the identification:
  - for raster layers shows value of underlying pixel
  - for vector layers shows feature attributes within search radius
    (allows to edit values when vector layer is in editing mode)
*/
class GUI_EXPORT QgsMapToolIdentify : public QgsMapTool
{
    Q_OBJECT

  public:

    enum IdentifyMode
    {
      DefaultQgsSetting = -1,
      ActiveLayer,
      TopDownStopAtFirst,
      TopDownAll
    };

    enum LayerType
    {
      AllLayers = -1,
      VectorLayer,
      RasterLayer
    };

    struct VectorResult
    {
      VectorResult() {}
      VectorResult( QgsVectorLayer * layer, QgsFeature feature, QMap< QString, QString > derivedAttributes ):
          mLayer( layer ), mFeature( feature ), mDerivedAttributes( derivedAttributes )  {}
      QgsVectorLayer* mLayer;
      QgsFeature mFeature;
      QMap< QString, QString > mDerivedAttributes;
    };

    struct RasterResult
    {
      RasterResult() {}
      RasterResult( QgsRasterLayer * layer, QString label, QMap< QString, QString > attributes, QMap< QString, QString > derivedAttributes ):
          mLayer( layer ), mLabel( label ), mAttributes( attributes ), mDerivedAttributes( derivedAttributes )  {}

      RasterResult( QgsRasterLayer * layer, QString label, QgsFields fields, QgsFeature feature, QMap< QString, QString > derivedAttributes ):
          mLayer( layer ), mLabel( label ), mFields( fields ), mFeature( feature ), mDerivedAttributes( derivedAttributes )  {}
      QgsRasterLayer* mLayer;
      QString mLabel;
      QgsFields mFields;
      QgsFeature mFeature;
      QMap< QString, QString > mAttributes;
      QMap< QString, QString > mDerivedAttributes;
    };

    struct IdentifyResults
    {
      IdentifyResults() {}
      IdentifyResults( QList<VectorResult> vectorResults , QList<RasterResult> rasterResults ) :
          mVectorResults( vectorResults ),
          mRasterResults( rasterResults )
      {}
      QList<VectorResult> mVectorResults;
      QList<RasterResult> mRasterResults;
    };

    //! constructor
    QgsMapToolIdentify( QgsMapCanvas* canvas );

    virtual ~QgsMapToolIdentify();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    virtual void activate();

    virtual void deactivate();

    /** Performs the identification.
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param layerList Performs the identification within the given list of layers. Default value is an empty list, i.e. uses all the layers.
    @param mode Identification mode. Can use Qgis default settings or a defined mode. Default mode is DefaultQgsSetting.
    @return true if identification succeeded and a feature has been found, false otherwise.*/
    bool identify( int x, int y, QList<QgsMapLayer*> layerList = QList<QgsMapLayer*>(), IdentifyMode mode = DefaultQgsSetting );

    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerType Only performs identification in a certain type of layers (raster, vector). Default value is AllLayers.
    @return true if identification succeeded and a feature has been found, false otherwise.*/
    bool identify( int x, int y, IdentifyMode mode, LayerType layerType = AllLayers );

    /** Access to results */
    IdentifyResults &results();

  public slots:
    void formatChanged( QgsRasterLayer *layer );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( QString );
    void changedRasterResults( QList<RasterResult>& );

  private:
    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerList Performs the identification within the given list of layers.
    @param layerType Only performs identification in a certain type of layers (raster, vector).
    @return true if identification succeeded and a feature has been found, false otherwise.*/
    bool identify( int x, int y, IdentifyMode mode,  QList<QgsMapLayer*> layerList, LayerType layerType = AllLayers );

    bool identify( QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel, IdentifyMode mode,  QList<QgsMapLayer*> layerList, LayerType layerType = AllLayers );

    bool identifyLayer( QgsMapLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel, LayerType layerType = AllLayers );
    bool identifyRasterLayer( QgsRasterLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel, QList<RasterResult>& rasterResults );
    bool identifyVectorLayer( QgsVectorLayer *layer, QgsPoint point );

    //! Private helper
    virtual void convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea );

    /** Transforms the measurements of derived attributes in the desired units*/
    virtual QGis::UnitType displayUnits();

    QMap< QString, QString > featureDerivedAttributes( QgsFeature *feature, QgsMapLayer *layer );

    IdentifyResults mResultData;

    // Last point in canvas CRS
    QgsPoint mLastPoint;

    double mLastMapUnitsPerPixel;

    QgsRectangle mLastExtent;
};

#endif
