/***************************************************************************
  qgsssaosettings.h
  --------------------------------------
  Date                 : Juin 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSSAOSETTINGS_H
#define QGSSSAOSETTINGS_H

#include <QString>
#include <QMap>

#include "qgis_3d.h"

class QgsReadWriteContext;
class QDomElement;

#define SIP_NO_FILE

class _3D_EXPORT QgsSsaoSettings
{
  public:
    //! Default constructor
    QgsSsaoSettings() = default;
    //! Copy constructor
    QgsSsaoSettings( const QgsSsaoSettings &other );
    //! delete assignment operator
    QgsSsaoSettings &operator=( QgsSsaoSettings const &rhs );

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );
    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    void setSsaoEnabled( bool enabled ) { mEnabled = enabled; }

    bool ssaoEnabled() const { return mEnabled; }

    void setShadingFactor( float factor ) { mShadingFactor = factor; }

    float shadingFactor() const { return mShadingFactor; }

    void setDistanceAttenuationFactor( float factor ) { mDistanceAttenuationFactor = factor; }

    float distanceAttenuationFactor() const { return mDistanceAttenuationFactor; }

    void setRadiusParameter( float radius ) { mRadiusParameter = radius; }

    float radiusParameter() const { return mRadiusParameter; }

  private:
    bool mEnabled = false;
    float mShadingFactor = 300.0f;
    float mDistanceAttenuationFactor = 500.0f;
    float mRadiusParameter = 0.05f;
};

#endif // QGSSKYBOXSETTINGS_H
