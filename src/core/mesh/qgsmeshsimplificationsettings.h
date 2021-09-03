/***************************************************************************
                         qgsmeshsimplificationsettings.h
                         ---------------------
    begin                : February 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHSIMPLIFICATONSETTINGS_H
#define QGSMESHSIMPLIFICATONSETTINGS_H

#include <QDomElement>

#include "qgis_core.h"
#include "qgis.h"
#include "qgsreadwritecontext.h"

SIP_NO_FILE

/**
 * \ingroup core
 *
 * \brief Represents an overview renderer settings
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsMeshSimplificationSettings
{
  public:
    //! Returns if the overview is active
    bool isEnabled() const;
    //! Sets if the overview is active
    void setEnabled( bool isEnabled );

    /**
     * Returns the reduction factor used to build simplified mesh.
     */
    double reductionFactor() const;

    /**
     * Sets the reduction factor used to build simplified mesh.
     * The triangles count of the simplified mesh equals approximately the triangles count of base mesh divided by this factor.
     * This reduction factor is used for simplification of each successive simplified mesh. For example, if the base mesh has 5M faces,
     * and the reduction factor is 10, the first simplified mesh will have approximately 500 000 faces, the second 50 000 faces,
     * the third 5000, ...
     * If higher reduction factor leads to simpler meshes, it produces also fewer levels of detail.
     * The reduction factor has to be strictly greater than 1. If not, the simplification processus will render nothing.
     */
    void setReductionFactor( double value );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    //! Returns the mesh resolution i.e., the minimum size (average) of triangles in pixels
    int meshResolution() const;

    /**
     * Sets the mesh resolution i.e., the minimum size (average) of triangles in pixels
     * This value is used during map rendering to choose the most appropriate mesh from the list of simplified meshes.
     * The first mesh which has its average triangle size greater than this value will be chosen.
     */
    void setMeshResolution( int meshResolution );

  private:
    bool mEnabled = false;
    double mReductionFactor = 10;
    int mMeshResolution = 5;
};
#endif // QGSMESHSIMPLIFICATONSETTINGS_H
