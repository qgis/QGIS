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

#include <Qt3DAnimation/QKeyframeAnimation>
#include <Qt3DAnimation/QAnimationGroup>

Qgs3DAnimationSettings::Qgs3DAnimationSettings()
{

}

float Qgs3DAnimationSettings::duration() const
{
  return mKeyframes.isEmpty() ? 0 : mKeyframes.constLast().time;
}

Qt3DAnimation::QKeyframeAnimation *Qgs3DAnimationSettings::createAnimation( Qt3DCore::QNode *parent ) const
{
  Qt3DAnimation::QKeyframeAnimation *animation = new Qt3DAnimation::QKeyframeAnimation;

  QVector<float> framePositions;
  QVector<Qt3DCore::QTransform *> transforms;
  for ( const Keyframe &keyframe : mKeyframes )
  {
    framePositions << keyframe.time;
    Qt3DCore::QTransform *t = new Qt3DCore::QTransform( parent );
    t->setTranslation( keyframe.position );
    t->setRotation( keyframe.rotation );
    transforms << t;
  }

  animation->setKeyframes( transforms );
  animation->setFramePositions( framePositions );

  //animation->setTarget(cam->transform());
  // animation->setEasing(QEasingCurve(QEasingCurve::InOutQuad));

  return animation;
}
