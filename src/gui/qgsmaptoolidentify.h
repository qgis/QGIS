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

#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsmaptool.h"
#include "qgsmaplayer.h"
#include "qgspoint.h"

#include <QObject>
#include <QPointer>

class QgsRasterLayer;
class QgsVectorLayer;
class QgsMapLayer;
class QgsMapCanvas;
class QgsHighlight;
class QgsIdentifyMenu;

/** \ingroup gui
  \brief Map tool for identifying features in layers

  after selecting a point, performs the identification:
  - for raster layers shows value of underlying pixel
  - for vector layers shows feature attributes within search radius
    (allows editing values when vector layer is in editing mode)
*/
class GUI_EXPORT QgsMapToolIdentify : public QgsMapTool
{
    Q_OBJECT
    Q_FLAGS( LayerType )

  public:

    enum IdentifyMode
    {
      DefaultQgsSetting = -1,
      ActiveLayer,
      TopDownStopAtFirst,
      TopDownAll,
      LayerSelection
    };

    enum Type
    {
      VectorLayer = 1,
      RasterLayer = 2,
      AllLayers = VectorLayer | RasterLayer
    };
    Q_DECLARE_FLAGS( LayerType, Type )

    struct IdentifyResult
    {
      IdentifyResult() : mLayer( nullptr ) {}

      IdentifyResult( QgsMapLayer * layer, const QgsFeature& feature, const QMap< QString, QString >& derivedAttributes ):
          mLayer( layer ), mFeature( feature ), mDerivedAttributes( derivedAttributes ) {}

      IdentifyResult( QgsMapLayer * layer, const QString& label, const QMap< QString, QString >& attributes, const QMap< QString, QString >& derivedAttributes ):
          mLayer( layer ), mLabel( label ), mAttributes( attributes ), mDerivedAttributes( derivedAttributes ) {}

      IdentifyResult( QgsMapLayer * layer, const QString& label, const QgsFields& fields, const QgsFeature& feature, const QMap< QString, QString >& derivedAttributes ):
          mLayer( layer ), mLabel( label ), mFields( fields ), mFeature( feature ), mDerivedAttributes( derivedAttributes ) {}

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

    virtual Flags flags() const override { return QgsMapTool::AllowZoomRect; }

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    virtual void activate() override;

    virtual void deactivate() override;

    /** Performs the identification.
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param layerList Performs the identification within the given list of layers. Default value is an empty list, i.e. uses all the layers.
    @param mode Identification mode. Can use Qgis default settings or a defined mode. Default mode is DefaultQgsSetting.
    @return a list of IdentifyResult*/
    QList<IdentifyResult> identify( int x, int y, const QList<QgsMapLayer*>& layerList = QList<QgsMapLayer*>(), IdentifyMode mode = DefaultQgsSetting );

    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerType Only performs identification in a certain type of layers (raster, vector). Default value is AllLayers.
    @return a list of IdentifyResult*/
    QList<IdentifyResult> identify( int x, int y, IdentifyMode mode, const LayerType& layerType = AllLayers );

    //! return a pointer to the identify menu which will be used in layer selection mode
    //! this menu can also be customized
    QgsIdentifyMenu* identifyMenu() {return mIdentifyMenu;}

  public slots:
    void formatChanged( QgsRasterLayer *layer );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( const QString& );
    void changedRasterResults( QList<IdentifyResult>& );

  protected:
    /** Performs the identification.
    To avoid beeing forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    @param x x coordinates of mouseEvent
    @param y y coordinates of mouseEvent
    @param mode Identification mode. Can use Qgis default settings or a defined mode.
    @param layerList Performs the identification within the given list of layers.
    @param layerType Only performs identification in a certain type of layers (raster, vector).
    @return a list of IdentifyResult*/
    QList<IdentifyResult> identify( int x, int y, IdentifyMode mode,  const QList<QgsMapLayer*>& layerList, const LayerType& layerType = AllLayers );

    QgsIdentifyMenu* mIdentifyMenu;

    /** Call the right method depending on layer type */
    bool identifyLayer( QList<IdentifyResult> *results, QgsMapLayer *layer, const QgsPoint& point, const QgsRectangle& viewExtent, double mapUnitsPerPixel, const QgsMapToolIdentify::LayerType& layerType = AllLayers );

    bool identifyRasterLayer( QList<IdentifyResult> *results, QgsRasterLayer *layer, QgsPoint point, const QgsRectangle& viewExtent, double mapUnitsPerPixel );
    bool identifyVectorLayer( QList<IdentifyResult> *results, QgsVectorLayer *layer, const QgsPoint& point );

  private:

    //! Private helper
    //! @deprecated use displayDistanceUnits() and displayAreaUnits() instead
    Q_DECL_DEPRECATED virtual void convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea );

    /** Transforms the measurements of derived attributes in the desired units
     * @deprecated use displayDistanceUnits() and displayAreaUnits() instead
    */
    Q_DECL_DEPRECATED virtual QGis::UnitType displayUnits();

    /** Desired units for distance display.
     * @note added in QGIS 2.14
     * @see displayAreaUnits()
     */
    virtual QGis::UnitType displayDistanceUnits() const;

    /** Desired units for area display.
     * @note added in QGIS 2.14
     * @see displayDistanceUnits()
     */
    virtual QgsUnitTypes::AreaUnit displayAreaUnits() const;

    /** Format a distance into a suitable string for display to the user
     * @note added in QGIS 2.14
     * @see formatArea()
     */
    QString formatDistance( double distance ) const;

    /** Format a distance into a suitable string for display to the user
     * @note added in QGIS 2.14
     * @see formatDistance()
     */
    QString formatArea( double area ) const;

    QMap< QString, QString > featureDerivedAttributes( QgsFeature *feature, QgsMapLayer *layer, const QgsPoint& layerPoint = QgsPoint() );

    /** Adds details of the closest vertex to derived attributes
     */
    void closestVertexAttributes( const QgsAbstractGeometryV2& geometry, QgsVertexId vId, QgsMapLayer *layer, QMap< QString, QString >& derivedAttributes );

    QString formatCoordinate( const QgsPoint& canvasPoint ) const;
    QString formatXCoordinate( const QgsPoint& canvasPoint ) const;
    QString formatYCoordinate( const QgsPoint& canvasPoint ) const;

    // Last point in canvas CRS
    QgsPoint mLastPoint;

    double mLastMapUnitsPerPixel;

    QgsRectangle mLastExtent;

    int mCoordinatePrecision;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapToolIdentify::LayerType )

#endif
