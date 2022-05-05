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

#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsidentifycontext.h"
#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgsunittypes.h"

#include <QObject>
#include <QPointer>
#include "qgis_gui.h"

class QgsRasterLayer;
class QgsVectorLayer;
class QgsVectorTileLayer;
class QgsMapLayer;
class QgsMapCanvas;
class QgsMeshLayer;
class QgsHighlight;
class QgsIdentifyMenu;
class QgsPointCloudLayer;
class QgsPointCloudLayerElevationProperties;
class QgsFeatureRenderer;
class QgsExpressionContext;

/**
 * \ingroup gui
 * \brief Map tool for identifying features in layers
 *
 * after selecting a point, performs the identification:
 *
 * - for raster layers shows value of underlying pixel
 * - for vector layers shows feature attributes within search radius
 *   (allows editing values when vector layer is in editing mode)
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
    Q_ENUM( IdentifyMode )

    enum Type
    {
      VectorLayer = 1,
      RasterLayer = 2,
      MeshLayer = 4, //!< \since QGIS 3.6
      VectorTileLayer = 8,  //!< \since QGIS 3.14
      PointCloudLayer = 16, //!< \since QGIS 3.18
      AllLayers = VectorLayer | RasterLayer | MeshLayer | VectorTileLayer | PointCloudLayer
    };
    Q_DECLARE_FLAGS( LayerType, Type )
    Q_FLAG( LayerType )

    struct IdentifyResult
    {
      //! Constructor for IdentifyResult
      IdentifyResult() = default;

      IdentifyResult( QgsMapLayer *layer, const QgsFeature &feature, const QMap< QString, QString > &derivedAttributes )
        : mLayer( layer ), mFeature( feature ), mDerivedAttributes( derivedAttributes ) {}

      IdentifyResult( QgsMapLayer *layer, const QString &label, const QMap< QString, QString > &attributes, const QMap< QString, QString > &derivedAttributes )
        : mLayer( layer ), mLabel( label ), mAttributes( attributes ), mDerivedAttributes( derivedAttributes ) {}

      IdentifyResult( QgsMapLayer *layer, const QString &label, const QgsFields &fields, const QgsFeature &feature, const QMap< QString, QString > &derivedAttributes )
        : mLayer( layer ), mLabel( label ), mFields( fields ), mFeature( feature ), mDerivedAttributes( derivedAttributes ) {}

      QgsMapLayer *mLayer = nullptr;
      QString mLabel;
      QgsFields mFields;
      QgsFeature mFeature;
      QMap< QString, QString > mAttributes;
      QMap< QString, QString > mDerivedAttributes;
      QMap< QString, QVariant > mParams;
    };

    //! constructor
    QgsMapToolIdentify( QgsMapCanvas *canvas );

    ~QgsMapToolIdentify() override;

    Flags flags() const override { return QgsMapTool::AllowZoomRect; }
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void activate() override;
    void deactivate() override;

    /**
     * Performs the identification.
     * \param x x coordinates of mouseEvent
     * \param y y coordinates of mouseEvent
     * \param layerList Performs the identification within the given list of layers. Default value is an empty list, i.e. uses all the layers.
     * \param mode Identification mode. Can use QGIS default settings or a defined mode. Default mode is DefaultQgsSetting.
     * \param identifyContext Identify context object.
     * \returns a list of IdentifyResult
    */
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, const QList<QgsMapLayer *> &layerList = QList<QgsMapLayer *>(), IdentifyMode mode = DefaultQgsSetting, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    /**
     * Performs the identification.
     * To avoid being forced to specify IdentifyMode with a list of layers
     * this has been made private and two publics methods are offered
     * \param x x coordinates of mouseEvent
     * \param y y coordinates of mouseEvent
     * \param mode Identification mode. Can use QGIS default settings or a defined mode.
     * \param layerType Only performs identification in a certain type of layers (raster, vector, mesh). Default value is AllLayers.
     * \param identifyContext Identify context object.
     * \returns a list of IdentifyResult
     */
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode, LayerType layerType = AllLayers, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    //! Performs identification based on a geometry (in map coordinates)
    QList<QgsMapToolIdentify::IdentifyResult> identify( const QgsGeometry &geometry, IdentifyMode mode, LayerType layerType, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );
    //! Performs identification based on a geometry (in map coordinates)
    QList<QgsMapToolIdentify::IdentifyResult> identify( const QgsGeometry &geometry, IdentifyMode mode, const QList<QgsMapLayer *> &layerList, LayerType layerType, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );


    /**
     * Returns a pointer to the identify menu which will be used in layer selection mode
     * this menu can also be customized
     */
    QgsIdentifyMenu *identifyMenu() { return mIdentifyMenu; }

    /**
     * Converts point cloud identification results from variant maps to QgsMapToolIdentify::IdentifyResult and apply some formatting
     * \note : the converted variant maps are pushed at the back of \a results without cleaning what's in it previously
     * \since QGIS 3.18
     */
    static void fromPointCloudIdentificationToIdentifyResults( QgsPointCloudLayer *layer, const QVector<QVariantMap> &identified, QList<QgsMapToolIdentify::IdentifyResult> &results ) SIP_SKIP;

    /**
     * Converts elevation profile identification results from variant maps to QgsMapToolIdentify::IdentifyResult and apply some formatting
     * \note Not available in Python bindings
     * \note The converted variant maps are pushed at the back of \a results without cleaning what's in it previously
     * \since QGIS 3.26
     */
    void fromElevationProfileLayerIdentificationToIdentifyResults( QgsMapLayer *layer, const QVector<QVariantMap> &identified, QList<QgsMapToolIdentify::IdentifyResult> &results ) SIP_SKIP;

  public slots:
    void formatChanged( QgsRasterLayer *layer );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( const QString & );
    void changedRasterResults( QList<QgsMapToolIdentify::IdentifyResult> & );

  protected:

    /**
     * Performs the identification.
     * To avoid being forced to specify IdentifyMode with a list of layers
     * this has been made private and two publics methods are offered
     * \param x x coordinates of mouseEvent
     * \param y y coordinates of mouseEvent
     * \param mode Identification mode. Can use QGIS default settings or a defined mode.
     * \param layerList Performs the identification within the given list of layers.
     * \param layerType Only performs identification in a certain type of layers (raster, vector, mesh).
     * \param identifyContext Identify context object.
     * \returns a list of IdentifyResult
     */
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode,  const QList<QgsMapLayer *> &layerList, LayerType layerType = AllLayers, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    QgsIdentifyMenu *mIdentifyMenu = nullptr;

    //! Call the right method depending on layer type
    bool identifyLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMapLayer *layer, const QgsPointXY &point, const QgsRectangle &viewExtent, double mapUnitsPerPixel, QgsMapToolIdentify::LayerType layerType = AllLayers, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    /**
     * Performs the identification against a given raster layer.
     * \param results list of identify results
     * \param layer raster layer to identify from
     * \param point point coordinate to identify
     * \param viewExtent view extent
     * \param mapUnitsPerPixel map units per pixel value
     * \param identifyContext identify context object
     */
    bool identifyRasterLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsRasterLayer *layer, QgsPointXY point, const QgsRectangle &viewExtent, double mapUnitsPerPixel, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    /**
     * Performs the identification against a given vector layer.
     * \param results list of identify results
     * \param layer raster layer to identify from
     * \param point point coordinate to identify
     * \param identifyContext identify context object
     */
    bool identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsPointXY &point, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    /**
     * Identifies data from active scalar and vector dataset from the mesh layer
     *
     * Works only if layer was already rendered (triangular mesh is created)
     * \since QGIS 3.6
     */
    bool identifyMeshLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMeshLayer *layer, const QgsPointXY &point, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    //! Returns derived attributes map for a clicked point in map coordinates. May be 2D or 3D point.
    QMap< QString, QString > derivedAttributesForPoint( const QgsPoint &point );

    /**
     * Overrides some map canvas properties inside the map tool for the upcoming identify requests.
     *
     * This is useful when the identification is triggered by some other piece of GUI like a 3D map view
     * and some properties like search radius need to be adjusted so that identification returns correct
     * results. Currently only search radius may be overridden.
     *
     * When the custom identification has finished, restoreCanvasPropertiesOverrides() should
     * be called to erase any overrides.
     * \see restoreCanvasPropertiesOverrides()
     * \since QGIS 3.4
     */
    void setCanvasPropertiesOverrides( double searchRadiusMapUnits );

    /**
     * Clears canvas properties overrides previously set with setCanvasPropertiesOverrides()
     * \see setCanvasPropertiesOverrides()
     * \since QGIS 3.4
     */
    void restoreCanvasPropertiesOverrides();

  private:

    bool identifyLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMapLayer *layer, const QgsGeometry &geometry, const QgsRectangle &viewExtent, double mapUnitsPerPixel, QgsMapToolIdentify::LayerType layerType = AllLayers, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );
    bool identifyRasterLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsRasterLayer *layer, const QgsGeometry &geometry, const QgsRectangle &viewExtent, double mapUnitsPerPixel, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );
    bool identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );
    int identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsFeatureList &features, QgsFeatureRenderer *renderer, const QMap< QString, QString >  &commonDerivedAttributes, const std::function< QMap< QString, QString > ( const QgsFeature & ) > &derivedAttributes, QgsRenderContext &context );
    bool identifyMeshLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMeshLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );
    bool identifyVectorTileLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorTileLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );
    bool identifyPointCloudLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsPointCloudLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext = QgsIdentifyContext() );

    /**
     * Desired units for distance display.
     * \see displayAreaUnits()
     * \since QGIS 2.14
     */
    virtual QgsUnitTypes::DistanceUnit displayDistanceUnits() const;

    /**
     * Desired units for area display.
     * \see displayDistanceUnits()
     * \since QGIS 2.14
     */
    virtual QgsUnitTypes::AreaUnit displayAreaUnits() const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatArea()
     * \since QGIS 2.14
     */
    QString formatDistance( double distance ) const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatDistance()
     * \since QGIS 2.14
     */
    QString formatArea( double area ) const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatArea()
     */
    QString formatDistance( double distance, QgsUnitTypes::DistanceUnit unit ) const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatDistance()
     */
    QString formatArea( double area, QgsUnitTypes::AreaUnit unit ) const;

    QMap< QString, QString > featureDerivedAttributes( const QgsFeature &feature, QgsMapLayer *layer, const QgsPointXY &layerPoint = QgsPointXY() );

    /**
     * Adds details of the closest vertex to derived attributes
     */
    void closestVertexAttributes( const QgsAbstractGeometry &geometry, QgsVertexId vId, QgsMapLayer *layer, QMap< QString, QString > &derivedAttributes );

    /**
     * Adds details of the closest point to derived attributes
    */
    void closestPointAttributes( const QgsAbstractGeometry &geometry, const QgsPointXY &layerPoint, QMap< QString, QString > &derivedAttributes );

    QString formatCoordinate( const QgsPointXY &canvasPoint ) const;
    QString formatXCoordinate( const QgsPointXY &canvasPoint ) const;
    QString formatYCoordinate( const QgsPointXY &canvasPoint ) const;

    // Last geometry (point or polygon) in map CRS
    QgsGeometry mLastGeometry;

    double mLastMapUnitsPerPixel;

    QgsRectangle mLastExtent;

    int mCoordinatePrecision;

    double mOverrideCanvasSearchRadius = -1;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapToolIdentify::LayerType )

#endif
