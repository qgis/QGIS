/***************************************************************************
  qgspointcloud3dsymbol.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUD3DSYMBOL_H
#define QGSPOINTCLOUD3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"

/**
 * \ingroup 3d
 * 3D symbol that draws point cloud geometries as 3D objects.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.18
 */
class _3D_EXPORT QgsPointCloud3DSymbol : public QgsAbstract3DSymbol
{
  public:
    //! Constructor for QgsPointCloud3DSymbol
    QgsPointCloud3DSymbol();
    //! Destructor for QgsPointCloud3DSymbol
    ~QgsPointCloud3DSymbol() override;

    QString type() const override { return "pointcloud"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    /**
     * Returns whether rendering for this symbol is enabled
     * \see setIsEnabled( bool enabled )
     */
    bool isEnabled() const { return mEnabled; }

    /**
     * Sets whether rendering for this symbol is enabled
     * \see isEnabled()
     */
    void setIsEnabled( bool enabled );

    /**
     * Returns the point size of the point cloud
     * \see setPointSize( float size )
     */
    float pointSize() const { return mPointSize; }

    /**
     * Sets the point size
     * \see pointSize()
     */
    void setPointSize( float size );

  private:
    bool mEnabled = true;
    float mPointSize = 2.0f;
};

#endif // QGSPOINTCLOUD3DSYMBOL_H
