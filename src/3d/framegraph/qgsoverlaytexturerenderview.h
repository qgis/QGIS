/***************************************************************************
  qgsoverlaytexturerenderview.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOVERLAYTEXTURERENDERVIEW_H
#define QGSOVERLAYTEXTURERENDERVIEW_H

#include "qgsabstractrenderview.h"

namespace Qt3DRender
{
  class QLayer;
} //namespace Qt3DRender

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Simple render view to preview overlay textures in 3D view.
 *
 * \see QgsOverlayTextureEntity
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.44
 */
class QgsOverlayTextureRenderView : public QgsAbstractRenderView
{
  public:
    //! Constructor
    QgsOverlayTextureRenderView( const QString &viewName );

    //! Returns layer in which entities must be added in the in order to be processed by this renderview.
    Qt3DRender::QLayer *overlayLayer() const;

  private:
    Qt3DRender::QLayer *mLayer = nullptr;

    void buildRenderPass();
};

#endif // QGSOVERLAYTEXTURERENDERVIEW_H
