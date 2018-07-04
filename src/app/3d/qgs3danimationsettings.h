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

#include <QVector3D>
#include <QQuaternion>

namespace Qt3DCore
{
  class QNode;
}

namespace Qt3DAnimation
{
  class QKeyframeAnimation;
}

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
      QVector3D position;    //!< Position of the camera
      QQuaternion rotation;  //!< Rotation of the camera
    };

    typedef QVector<Keyframe> Keyframes;

    void setKeyframes( const Keyframes &keyframes ) { mKeyframes = keyframes; }
    Keyframes keyFrames() const { return mKeyframes; }

    //! Returns duration of the whole animation in seconds
    float duration() const;

    //! Returns a new object that contains Qt3D animation according to the keyframes
    Qt3DAnimation::QKeyframeAnimation *createAnimation( Qt3DCore::QNode *parent ) const;

    // TODO: read/write routines

  private:
    Keyframes mKeyframes;
};

#endif // QGS3DANIMATIONSETTINGS_H
