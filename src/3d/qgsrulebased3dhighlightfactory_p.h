/***************************************************************************
  qgsrulebased3dhighlightfactory_p.h
  --------------------------------------
  Date                 : December 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRULEBASED3DHIGHLIGHTFACTORY_H
#define QGSRULEBASED3DHIGHLIGHTFACTORY_H

#define SIP_NO_FILE

///@cond PRIVATE

#include "qgsabstractvectorlayer3dhighlightfactory_p.h"
#include "qgsrulebased3drenderer.h"

namespace Qt3DCore
{
  class QEntity;
}

/**
 * \ingroup qgis_3d
 *
 * \brief 3D highlight factory based on a rule-based 3D renderer.
 *
 * This factory creates 3D highlight entities for vector layer features
 * using a rule-based 3D renderer. It evaluates the feature
 * against the renderer rules and generates highlight entities according
 * to the matching rules.
 *
 * \since QGIS 4.0
 */
class QgsRuleBased3DHighlightFactory : public QgsAbstractVectorLayer3DHighlightFactory
{
  public:
    QgsRuleBased3DHighlightFactory( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vLayer, QgsRuleBased3DRenderer::Rule *rootRule );

    virtual ~QgsRuleBased3DHighlightFactory() override;

    QList<Qt3DCore::QEntity *> create( const QgsFeature &feature, QgsAbstract3DEngine *engine, Qt3DCore::QEntity *parent, const QColor &fillColor, const QColor &edgeColor ) override;

  private:
    std::unique_ptr<QgsRuleBased3DRenderer::Rule> mRootRule = nullptr;
    QgsRuleBased3DRenderer::RuleToHandlerMap mHandlers;
};

/// @endcond

#endif // QGSRULEBASED3DHIGHLIGHTFACTORY_H
