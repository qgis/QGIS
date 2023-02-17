/***************************************************************************
  qgsabstractpointcloud3drenderer.h
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Stefanos Natsis
  Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTPOINTCLOUD3DRENDERER_H
#define QGSABSTRACTPOINTCLOUD3DRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstract3drenderer.h"

class QgsPointCloudRenderer;

#ifndef SIP_RUN
namespace Qt3DCore
{
  class QEntity;
}
#endif

/**
 * \ingroup core
 * \brief Base class for point cloud 3D renderers.
 *
 * This class allows for functionality in 3D to be called from core.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractPointCloud3DRenderer : public QgsAbstract3DRenderer SIP_ABSTRACT
{
  public:
    //! Default destructor
    virtual ~QgsAbstractPointCloud3DRenderer() = default;

    /**
     * Updates the 3D renderer's symbol to match that of a given QgsPointCloudRenderer
     *
     * \returns TRUE on success, FALSE otherwise
     */
    virtual bool convertFrom2DRenderer( QgsPointCloudRenderer *renderer ) = 0;

  protected:
    //! Default constructor
    QgsAbstractPointCloud3DRenderer() = default;

  private:
#ifdef SIP_RUN
    QgsAbstractPointCloud3DRenderer( const QgsAbstractPointCloud3DRenderer & );
    QgsAbstractPointCloud3DRenderer &operator=( const QgsAbstractPointCloud3DRenderer & );
#endif

    Q_DISABLE_COPY( QgsAbstractPointCloud3DRenderer )
};


#endif // QGSABSTRACTPOINTCLOUD3DRENDERER_H
