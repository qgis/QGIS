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

#include "qgis_3d.h"

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

    /**
     * \brief Axis representation enum
     */
    enum class Mode
    {
      Off = 1, //!< Hide 3d axis
      Crs = 2, //!< Respect CRS directions
      Cube = 3, //!< Abstract cube mode
    };

    //! default constructor
    Qgs3DAxisSettings() = default;
    //! copy constructor
    Qgs3DAxisSettings( const Qgs3DAxisSettings &other );
    //! delete assignment operator
    Qgs3DAxisSettings &operator=( Qgs3DAxisSettings const &rhs );

    //! Returns true if both objects are equal
    bool operator==( Qgs3DAxisSettings const &rhs ) const;

    //! Returns true if objects are not equal
    bool operator!=( Qgs3DAxisSettings const &rhs ) const;

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );
    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    //! Returns the default axis viewport size in millimeters
    int defaultViewportSize() const { return mDefaultViewportSize;}
    //! Sets the defaultl axis viewport size in millimeters
    void setDefaultViewportSize( int size ) { mDefaultViewportSize = size; }

    //! Returns the minimal axis viewport ratio (see Qt3DRender::QViewport::normalizedRect())
    double minViewportRatio() const { return mMinViewportRatio;}
    //! Sets the minimal axis viewport ratio between 0-1
    void setMinViewportRatio( double ratio );

    //! Returns the maximal axis viewport ratio (see Qt3DRender::QViewport::normalizedRect())
    double maxViewportRatio() const { return mMaxViewportRatio;}
    //! Sets the maximal axis viewport ratio between 0-1
    void setMaxViewportRatio( double ratio );

    //! Returns the type of the 3daxis
    Qgs3DAxisSettings::Mode mode() const { return mMode; }
    //! Sets the type of the 3daxis
    void setMode( Qgs3DAxisSettings::Mode type ) { mMode = type; }

    //! Returns the horizontal position for the 3d axis
    Qt::AnchorPoint horizontalPosition() const { return mHorizontalPosition; }
    //! Sets the horizontal position for the 3d axis
    void setHorizontalPosition( Qt::AnchorPoint position ) { mHorizontalPosition = position; }

    //! Returns the vertical position for the 3d axis
    Qt::AnchorPoint verticalPosition() const { return mVerticalPosition; }
    //! Sets the vertical position for the 3d axis
    void setVerticalPosition( Qt::AnchorPoint position ) { mVerticalPosition = position; }

  private:
    double mMinViewportRatio = 0.06;
    double mMaxViewportRatio = 0.5;
    int mDefaultViewportSize = 40;
    Qgs3DAxisSettings::Mode mMode = Qgs3DAxisSettings::Mode::Crs;
    Qt::AnchorPoint mHorizontalPosition = Qt::AnchorPoint::AnchorRight;
    Qt::AnchorPoint mVerticalPosition = Qt::AnchorPoint::AnchorTop;

};

#endif // QGS3DAXISSETTINGS_H
