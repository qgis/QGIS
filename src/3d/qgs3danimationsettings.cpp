/***************************************************************************
  qgs3danimationsettings.cpp
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

#include "qgs3danimationsettings.h"

#include <QDomDocument>
#include <QEasingCurve>

Qgs3DAnimationSettings::Qgs3DAnimationSettings() = default;

float Qgs3DAnimationSettings::duration() const
{
  return mKeyframes.isEmpty() ? 0 : mKeyframes.constLast().time;
}

Qgs3DAnimationSettings::Keyframe Qgs3DAnimationSettings::interpolate( float time ) const
{
  if ( mKeyframes.isEmpty() )
    return Keyframe();

  if ( time < mKeyframes.constFirst().time )
  {
    return mKeyframes.first();
  }
  else if ( time >= mKeyframes.constLast().time )
  {
    return mKeyframes.last();
  }
  else
  {
    // TODO: make easing curves configurable.
    // QEasingCurve is probably not flexible enough, we may need more granular
    // control with Bezier curves to allow smooth transition at keyframes

    for ( int i = 0; i < mKeyframes.size() - 1; i++ )
    {
      const Keyframe &k0 = mKeyframes.at( i );
      const Keyframe &k1 = mKeyframes.at( i + 1 );
      if ( time >= k0.time && time <= k1.time )
      {
        const float ip = ( time - k0.time ) / ( k1.time - k0.time );
        const float eIp = mEasingCurve.valueForProgress( ip );
        const float eIip = 1.0f - eIp;

        Keyframe kf;
        kf.time = time;
        kf.point.set( k0.point.x() * eIip + k1.point.x() * eIp, k0.point.y() * eIip + k1.point.y() * eIp, k0.point.z() * eIip + k1.point.z() * eIp );
        kf.dist = k0.dist * eIip + k1.dist * eIp;
        kf.pitch = k0.pitch * eIip + k1.pitch * eIp;

        // always use shorter angle
        float yaw0 = fmod( k0.yaw, 360 ), yaw1 = fmod( k1.yaw, 360 );
        if ( std::abs( yaw0 - yaw1 ) > 180 )
        {
          if ( yaw0 < yaw1 )
            yaw0 += 360;
          else
            yaw1 += 360;
        }

        kf.yaw = yaw0 * eIip + yaw1 * eIp;
        return kf;
      }
    }
  }
  Q_ASSERT( false );
  return Keyframe();
}

void Qgs3DAnimationSettings::readXml( const QDomElement &elem )
{
  mEasingCurve = QEasingCurve( ( QEasingCurve::Type ) elem.attribute( u"interpolation"_s, u"0"_s ).toInt() );

  mKeyframes.clear();

  const QDomElement elemKeyframes = elem.firstChildElement( u"keyframes"_s );
  QDomElement elemKeyframe = elemKeyframes.firstChildElement( u"keyframe"_s );
  while ( !elemKeyframe.isNull() )
  {
    Keyframe kf;
    kf.time = elemKeyframe.attribute( u"time"_s ).toFloat();
    kf.point.set( elemKeyframe.attribute( u"x"_s ).toDouble(), elemKeyframe.attribute( u"y"_s ).toDouble(), elemKeyframe.attribute( u"z"_s ).toDouble() );
    kf.dist = elemKeyframe.attribute( u"dist"_s ).toFloat();
    kf.pitch = elemKeyframe.attribute( u"pitch"_s ).toFloat();
    kf.yaw = elemKeyframe.attribute( u"yaw"_s ).toFloat();
    mKeyframes.append( kf );
    elemKeyframe = elemKeyframe.nextSiblingElement( u"keyframe"_s );
  }
}

QDomElement Qgs3DAnimationSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"animation3d"_s );
  elem.setAttribute( u"interpolation"_s, mEasingCurve.type() );

  QDomElement elemKeyframes = doc.createElement( u"keyframes"_s );

  for ( const Keyframe &keyframe : mKeyframes )
  {
    QDomElement elemKeyframe = doc.createElement( u"keyframe"_s );
    elemKeyframe.setAttribute( u"time"_s, keyframe.time );
    elemKeyframe.setAttribute( u"x"_s, keyframe.point.x() );
    elemKeyframe.setAttribute( u"y"_s, keyframe.point.y() );
    elemKeyframe.setAttribute( u"z"_s, keyframe.point.z() );
    elemKeyframe.setAttribute( u"dist"_s, keyframe.dist );
    elemKeyframe.setAttribute( u"pitch"_s, keyframe.pitch );
    elemKeyframe.setAttribute( u"yaw"_s, keyframe.yaw );
    elemKeyframes.appendChild( elemKeyframe );
  }

  elem.appendChild( elemKeyframes );

  return elem;
}
