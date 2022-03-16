/***************************************************************************
  qgsmaptooldigitizegeometry.h

 ---------------------
 begin                : 7.12.2017
 copyright            : (C) 2017 by David Signer
 email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPTOOLDIGITIZEFEATURE_H
#define QGSMAPTOOLDIGITIZEFEATURE_H

#include "qgsmaptoolcapturelayergeometry.h"
#include "qgis_gui.h"

class QgsFeature;

/**
 * \ingroup gui
 * \brief This tool digitizes geometry of new point/line/polygon features on already existing vector layers
 * Once the map tool is enabled, user can digitize the feature geometry.
 * A signal will then be emitted.
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsMapToolDigitizeFeature : public QgsMapToolCaptureLayerGeometry
{
    Q_OBJECT

  public:

    /**
     * \brief QgsMapToolDigitizeFeature is a map tool to digitize a feature geometry
     * \param canvas the map canvas
     * \param cadDockWidget widget to setup advanced digitizing parameters
     * \param mode type of geometry to capture (point/line/polygon), QgsMapToolCapture::CaptureNone to autodetect geometry
     */
    QgsMapToolDigitizeFeature( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode = QgsMapToolCapture::CaptureNone );

    QgsMapToolCapture::Capabilities capabilities() const override;
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    /**
     * Change the layer edited by the map tool
     * \param vl the layer to be edited by the map tool
     */
    void setLayer( QgsMapLayer *vl );

    void activate() override;
    void deactivate() override;

  signals:

    /**
     * Emitted whenever the digitizing has been successfully completed
     * \param feature the new digitized feature
     */
    void digitizingCompleted( const QgsFeature &feature );

    /**
     * Emitted whenever the digitizing has been ended without digitizing
     * any feature
     */
    void digitizingFinished();

  protected:

    /**
     * Check if CaptureMode matches layer type. Default is TRUE.
     * \since QGIS 3.0
     */
    bool checkGeometryType() const;

    /**
     * Check if CaptureMode matches layer type. Default is TRUE.
     * \since QGIS 3.0
     */
    void setCheckGeometryType( bool checkGeometryType );
    // TODO QGIS 4: remove if GRASS plugin is dropped

  private:

    /**
     * Called when the feature has been digitized.
     * \param geometry the digitized geometry
     */
    void layerGeometryCaptured( const QgsGeometry &geometry ) FINAL;

    /**
     * Called when the feature has been digitized
     * \since QGIS 3.26
     */
    virtual void featureDigitized( const QgsFeature &feature )  {Q_UNUSED( feature )} SIP_FORCE

    /**
     * individual layer per digitizing session
     * \since QGIS 3.0
    */
    QgsMapLayer *mLayer = nullptr;

    /**
     * layer used before digitizing session
     * \since QGIS 3.0
    */
    QgsMapLayer *mCurrentLayer = nullptr;

    /**
     * Check if CaptureMode matches layer type. Default is TRUE.
     * \since QGIS 2.12
    */
    bool mCheckGeometryType;

    friend class TestQgsRelationReferenceWidget;
};

#endif // QGSMAPTOOLDIGITIZEFEATURE_H
