/***************************************************************************
    qgsmaptooladdfeature.h  -  map tool for adding point/line/polygon features
    ---------------------
    begin                : April 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooldigitizefeature.h"

//! This tool adds new point/line/polygon features to already existing vector layers
class APP_EXPORT QgsMapToolAddFeature : public QgsMapToolDigitizeFeature
{
    Q_OBJECT
  public:
    //! \since QGIS 2.12
    QgsMapToolAddFeature( QgsMapCanvas *canvas, CaptureMode mode );

    bool addFeature( QgsVectorLayer *vlayer, QgsFeature *f, bool showModal = true );

    void digitized( const QgsFeature *f ) override;

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
     * Check if CaptureMode matches layer type. Default is true.
     * \since QGIS 2.12 */
    bool mCheckGeometryType;
};
