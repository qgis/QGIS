/***************************************************************************
  qgs3danimationsettings.h
  --------------------------------------
  Date                 : July 2018
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

#ifndef QGS3DANIMATIONSETTINGS_H
#define QGS3DANIMATIONSETTINGS_H

#include "qgsvector3d.h"

#include <QVector>

class QDomDocument;
class QDomElement;
class QgsReadWriteContext;

/**
 * Class that holds information about animation in 3D view. The animation is defined
 * as a series of keyframes
 */
class Qgs3DAnimationSettings
{
  public:
    Qgs3DAnimationSettings();

    //! keyframe definition
    struct Keyframe
    {
      float time;            //!< Relative time of the keyframe in seconds

      QgsVector3D point;  //!< Point towards which the camera is looking in 3D world coords
      float dist;   //!< Distance of the camera from the focal point
      float pitch;  //!< Tilt of the camera in degrees (0 = looking from the top, 90 = looking from the side, 180 = looking from the bottom)
      float yaw;    //!< Horizontal rotation around the focal point in degrees
    };

    typedef QVector<Keyframe> Keyframes;

    void setKeyframes( const Keyframes &keyframes ) { mKeyframes = keyframes; }
    Keyframes keyFrames() const { return mKeyframes; }

    //! Returns duration of the whole animation in seconds
    float duration() const;

    //! Interpolates camera position and rotation at the given point in time
    Keyframe interpolate( float time ) const;

    //! Reads configuration from a DOM element previously written by writeXml()
    void readXml( const QDomElement &elem );
    //! Writes configuration to a DOM element, to be used later with readXml()
    QDomElement writeXml( QDomDocument &doc ) const;

  private:
    Keyframes mKeyframes;
};

Q_DECLARE_METATYPE( Qgs3DAnimationSettings::Keyframe )

#endif // QGS3DANIMATIONSETTINGS_H
