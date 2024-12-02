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

#ifndef QGSMAPTOOLADDFEATURE_H
#define QGSMAPTOOLADDFEATURE_H

#include "qgsmaptooldigitizefeature.h"
#include "qgis_app.h"

class QgsHighlight;

//! This tool adds new point/line/polygon features to already existing vector layers
class APP_EXPORT QgsMapToolAddFeature : public QgsMapToolDigitizeFeature
{
    Q_OBJECT

  public:
    //! \since QGIS 3.26
    QgsMapToolAddFeature( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );

    /**
     * \deprecated QGIS 3.40. Will be made in QGIS 4.
     */
    QgsMapToolAddFeature( QgsMapCanvas *canvas, CaptureMode mode );

  private slots:

    void featureDigitized( const QgsFeature &feature ) override;

  private:
    bool addFeature( QgsVectorLayer *vlayer, const QgsFeature &f, bool showModal = true );

    /**
     * Creates a highlight corresponding to the captured geometry map tool and transfers
     * ownership to the caller.
     */
    std::unique_ptr<QgsHighlight> createHighlight( QgsVectorLayer *layer, const QgsFeature &f );
};

#endif // QGSMAPTOOLADDFEATURE_H
