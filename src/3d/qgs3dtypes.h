/***************************************************************************
  qgs3dtypes.h
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DTYPES_H
#define QGS3DTYPES_H

#include "qgis_3d.h"

/**
 * \ingroup 3d
 * \brief Defines enumerations and other auxiliary types for QGIS 3D
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.4
 */
class _3D_EXPORT Qgs3DTypes
{
  public:

    //! Triangle culling mode
    enum CullingMode
    {
      NoCulling,     //!< Will render both front and back faces of triangles
      Front,         //!< Will render only back faces of triangles
      Back,          //!< Will render only front faces of triangles (recommended when input data are consistent)
      FrontAndBack   //!< Will not render anything
    };
};

#endif // QGS3DTYPES_H
