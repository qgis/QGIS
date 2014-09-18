/***************************************************************************
    qgsmaptoolidentifyfeature.h
     --------------------------------------
    Date                 : 22.5.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLIDENTIFYFEATURE_H
#define QGSMAPTOOLIDENTIFYFEATURE_H

#include "qgsmaptoolidentify.h"

/**
 * @brief The QgsMapToolIdentifyFeature class is a map tool to identify a feature on a chosen layer.
 * Once the map tool is enable, user can click on the map canvas to identify a feature.
 * A signal will then be emitted.
 */
class GUI_EXPORT QgsMapToolIdentifyFeature : public QgsMapToolIdentify
{
    Q_OBJECT

  public:
    /**
     * @brief QgsMapToolIdentifyFeature is a map tool to identify a feature on a chosen layer
     * @param canvas the map canvas
     * @param vl the vector layer. The map tool can be initialized without any layer and can be set afterward.
     */
    QgsMapToolIdentifyFeature( QgsMapCanvas* canvas, QgsVectorLayer* vl = 0 );

    ~QgsMapToolIdentifyFeature();

    //! change the layer used by the map tool to identify
    void setLayer( QgsVectorLayer* vl ) { mLayer = vl; }

    virtual void canvasReleaseEvent( QMouseEvent* e );

  signals:
    void featureIdentified( const QgsFeature& );
    void featureIdentified( QgsFeatureId );

  protected:
    virtual void keyPressEvent( QKeyEvent* e );

  private:
    QgsMapCanvas* mCanvas;
    QgsVectorLayer* mLayer;
};

#endif // QGSMAPTOOLIDENTIFYFEATURE_H
