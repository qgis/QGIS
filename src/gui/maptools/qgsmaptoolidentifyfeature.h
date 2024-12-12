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
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief The QgsMapToolIdentifyFeature class is a map tool to identify a feature on a chosen layer.
 * Once the map tool is enable, user can click on the map canvas to identify a feature.
 * A signal will then be emitted.
 */
class GUI_EXPORT QgsMapToolIdentifyFeature : public QgsMapToolIdentify
{
    Q_OBJECT

  public:
    /**
     * \brief QgsMapToolIdentifyFeature is a map tool to identify a feature on a chosen layer
     * \param canvas the map canvas
     * \param vl the vector layer. The map tool can be initialized without any layer and can be set afterward.
     */
    QgsMapToolIdentifyFeature( QgsMapCanvas *canvas, QgsVectorLayer *vl = nullptr );

    //! change the layer used by the map tool to identify
    void setLayer( QgsVectorLayer *vl ) { mLayer = vl; }

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

  signals:

    /**
     * Emitted when a \a feature has been identified
     */
    void featureIdentified( const QgsFeature &feature );

    /**
     * Emitted when a feature has been identified by its \a id.
     *
     * \deprecated QGIS 3.40. Use the signal with a QgsFeature argument instead.
     */
    Q_DECL_DEPRECATED void featureIdentified( QgsFeatureId id ) SIP_DEPRECATED;

  protected:
    void keyPressEvent( QKeyEvent *e ) override;

  private:
    QgsVectorLayer *mLayer = nullptr;
};

#endif // QGSMAPTOOLIDENTIFYFEATURE_H
