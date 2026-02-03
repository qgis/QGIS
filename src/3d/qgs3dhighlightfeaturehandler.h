/***************************************************************************
    qgs3dhighlightfeaturehandler.h
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DHIGHLIGHTFEATUREHANDLER_H
#define QGS3DHIGHLIGHTFEATUREHANDLER_H

#include "qgis_3d.h"
#include "qgsrulebased3drenderer.h"

#include <QMap>
#include <QObject>
#include <QVector>

class QTimer;
class Qgs3DMapScene;
class Qgs3DMapSceneEntity;
class QgsFeature3DHandler;
class QgsFeature;
class QgsMapLayer;
class QgsRubberBand3D;

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Handles the creation of 3D entities used for highlighting identified features
 * \note Not available in Python bindings
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT Qgs3DHighlightFeatureHandler : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    explicit Qgs3DHighlightFeatureHandler( Qgs3DMapScene *scene );
    ~Qgs3DHighlightFeatureHandler() override;

  public slots:
    /**
     * Highlights \a feature of \a layer in the 3d scene
     * When multiple features are identified, this slot is called multiple times. The features are aggregated,
     * and eventually a single highlight entity is added to the 3d scene for each layer.
     */
    void highlightFeature( QgsFeature feature, QgsMapLayer *layer );

    //! Clears all highlights
    void clearHighlights();

  private slots:
    void onRenderer3DChanged();

  private:
    void finalizeAndAddToScene( Qgs3DMapSceneEntity *entity );

    Qgs3DMapScene *mScene;
    //! This holds the rubber bands for highlighting identified point cloud features
    QMap<QgsMapLayer *, QgsRubberBand3D *> mRubberBands;
    //! This holds the entities for highlighting identified vector features
    QVector<Qgs3DMapSceneEntity *> mHighlightEntities;
    //! Per layer feature handlers for vector 3d renderers
    QMap<QgsMapLayer *, QgsFeature3DHandler *> mHighlightHandlers;
    //! Per layer feature handlers for rule based 3d renderers
    QMap<QgsMapLayer *, QgsRuleBased3DRenderer::RuleToHandlerMap> mHighlightRuleBasedHandlers;
    //! Singleshot timer is used to trigger finalizing the 3d entities and adding them to the scene
    std::unique_ptr<QTimer> mHighlightHandlerTimer;
};

#endif // QGS3DHIGHLIGHTFEATUREHANDLER_H
