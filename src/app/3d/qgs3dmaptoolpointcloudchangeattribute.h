/***************************************************************************
    qgs3dmaptoolpointcloudchangeattribute.h
    ---------------------
    begin                : January 2025
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

#ifndef QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTE_H
#define QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTE_H

#include "qgs3dmaptool.h"

/**
 * \ingroup App
 * \brief Base class for map tools editing point clouds on 3D map canvas.
 * \note Not available in Python bindings
 * \note Reworked in QGIS 3.44 to be virtual class for editing tools
 * \since QGIS 3.42
 */
class Qgs3DMapToolPointCloudChangeAttribute : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolPointCloudChangeAttribute( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolPointCloudChangeAttribute() override;

    //! Sets attribute name which will be edited
    void setAttribute( const QString &attribute );
    //! Sets attribute value to which it will be set
    void setNewValue( double value );

  protected:
    //! Calculate selection and edit set attribute to new value
    virtual void run();
    //! Clear selection
    virtual void restart();

    QString mAttributeName;
    double mNewValue = 0;
};

#endif // QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTE_H
