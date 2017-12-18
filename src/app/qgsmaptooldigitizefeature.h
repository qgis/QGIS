/***************************************************************************
  qgsmaptooldigitizegeometry.h

 ---------------------
 begin                : 7.12.2017
 copyright            : (C) 2017 by David Signer
 email                : david@opengis.ch
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

#include "qgsmaptoolcapture.h"
#include "qgis_app.h"

//! This tool digitizes geometry of new point/line/polygon features on already existing vector layers
class APP_EXPORT QgsMapToolDigitizeFeature : public QgsMapToolCapture
{
    Q_OBJECT
  public:
    //! \since QGIS 3.2
    QgsMapToolDigitizeFeature( QgsMapCanvas *canvas, QgsMapLayer *layer, CaptureMode mode );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    virtual void digitized( const QgsFeature *f );

    virtual void activate() override;
    virtual void deactivate() override;

  signals:
    void digitizingCompleted( const QgsFeature & );
    void digitizingFinished( );

  protected:

    /**
     * Check if CaptureMode matches layer type. Default is true.
     * \since QGIS 3.0
     */
    bool checkGeometryType() const;

    /**
     * Check if CaptureMode matches layer type. Default is true.
     * \since QGIS 3.0
     */
    void setCheckGeometryType( bool checkGeometryType );

  private:

    /**
     * individual layer per digitizing session
     * \since QGIS 3.0 */
    QgsMapLayer *mLayer = nullptr;

    /**
     * layer used before digitizing session
     * \since QGIS 3.0 */
    QgsMapLayer *mCurrentLayer = nullptr;

    /**
     * Check if CaptureMode matches layer type. Default is true.
     * \since QGIS 2.12 */
    bool mCheckGeometryType;
};

#endif // QGSMAPTOOLDIGITIZEFEATURE_H
