/***************************************************************************
  qgs3daxissettings.h
  --------------------------------------
  Date                 : April 2022
  copyright            : (C) 2021 B. De Mezzo
  email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DAXISSETTINGS_H
#define QGS3DAXISSETTINGS_H

#include <QString>
#include <QMap>

#include "qgs3daxis.h"

class QgsReadWriteContext;
class QDomElement;

#define SIP_NO_FILE

/**
 * \brief Contains the configuration of a 3d axis.
 *
 * \ingroup 3d
 * \since QGIS 3.26
 */
class _3D_EXPORT Qgs3DAxisSettings
{
  public:
    //! default constructor
    Qgs3DAxisSettings() = default;
    //! copy constructor
    Qgs3DAxisSettings( const Qgs3DAxisSettings &other );
    //! delete assignment operator
    Qgs3DAxisSettings &operator=( Qgs3DAxisSettings const &rhs );

    //! Returns true if both objects are equal
    bool operator==( Qgs3DAxisSettings const &rhs );

    //! Returns true if objects are not equal
    bool operator!=( Qgs3DAxisSettings const &rhs );

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );
    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    //! Returns the type of the 3daxis
    Qgs3DAxis::Mode mode() const { return mMode; }
    //! Sets the type of the 3daxis
    void setMode( Qgs3DAxis::Mode type ) { mMode = type; }

    //! Returns the horizontal position for the 3d axis
    Qgs3DAxis::AxisViewportPosition horizontalPosition() const { return mHorizontalPosition; }
    //! Sets the horizontal position for the 3d axis
    void setHorizontalPosition( const Qgs3DAxis::AxisViewportPosition &position ) { mHorizontalPosition = position; }

    //! Returns the vertical position for the 3d axis
    Qgs3DAxis::AxisViewportPosition verticalPosition() const { return mVerticalPosition; }
    //! Sets the vertical position for the 3d axis
    void setVerticalPosition( const Qgs3DAxis::AxisViewportPosition &position ) { mVerticalPosition = position; }

  private:
    Qgs3DAxis::Mode mMode = Qgs3DAxis::Mode::Crs;
    Qgs3DAxis::AxisViewportPosition mHorizontalPosition = Qgs3DAxis::AxisViewportPosition::End;
    Qgs3DAxis::AxisViewportPosition mVerticalPosition = Qgs3DAxis::AxisViewportPosition::Begin;
    ;
};

#endif // QGS3DAXISSETTINGS_H
