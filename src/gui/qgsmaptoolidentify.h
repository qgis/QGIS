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
class QgsMapLayer;
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
      TopDownAll,
      LayerSelection
    };

    enum LayerType
    {
      AllLayers = -1,
      VectorLayer,
      RasterLayer
    };

    struct IdentifyResult
    {
      IdentifyResult() {}

      IdentifyResult( QgsMapLayer * layer, QgsFeature feature, QMap< QString, QString > derivedAttributes ):
          mLayer( layer ), mFeature( feature ), mDerivedAttributes( derivedAttributes )  {}

      IdentifyResult( QgsMapLayer * layer, QString label, QMap< QString, QString > attributes, QMap< QString, QString > derivedAttributes ):
          mLayer( layer ), mLabel( label ), mAttributes( attributes ), mDerivedAttributes( derivedAttributes )  {}

      IdentifyResult( QgsMapLayer * layer, QString label, QgsFields fields, QgsFeature feature, QMap< QString, QString > derivedAttributes ):
          mLayer( layer ), mLabel( label ), mFields( fields ), mFeature( feature ), mDerivedAttributes( derivedAttributes )  {}

      QgsMapLayer* mLayer;
      QString mLabel;
      QgsFields mFields;
      QgsFeature mFeature;
      QMap< QString, QString > mAttributes;
      QMap< QString, QString > mDerivedAttributes;
      QMap< QString, QVariant > mParams;
    };

    //! constructor
    QgsMapToolIdentify( QgsMapCanvas * canvas );

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
    @return a list of IdentifyResult*/
    QList<IdentifyResult> identify( int x, int y, QList<QgsMapLayer*> layerList = QList<QgsMapLayer*>(), IdentifyMode mode = DefaultQgsSetting );

    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerType Only performs identification in a certain type of layers (raster, vector). Default value is AllLayers.
    @return a list of IdentifyResult*/
    QList<IdentifyResult> identify( int x, int y, IdentifyMode mode, LayerType layerType = AllLayers );

  public slots:
    void formatChanged( QgsRasterLayer *layer );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( QString );
    void changedRasterResults( QList<IdentifyResult>& );

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
    QList<IdentifyResult> identify( int x, int y, IdentifyMode mode,  QList<QgsMapLayer*> layerList, LayerType layerType = AllLayers );

    /** call the right method depending on layer type */
    bool identifyLayer( QList<IdentifyResult> *results, QgsMapLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel, LayerType layerType = AllLayers );

    bool identifyRasterLayer( QList<IdentifyResult> *results, QgsRasterLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel );
    bool identifyVectorLayer( QList<IdentifyResult> *results, QgsVectorLayer *layer, QgsPoint point );

    //! Private helper
    virtual void convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea );

    /** Transforms the measurements of derived attributes in the desired units*/
    virtual QGis::UnitType displayUnits();

    QMap< QString, QString > featureDerivedAttributes( QgsFeature *feature, QgsMapLayer *layer );

    // Last point in canvas CRS
    QgsPoint mLastPoint;

    double mLastMapUnitsPerPixel;

    QgsRectangle mLastExtent;
};

#endif
