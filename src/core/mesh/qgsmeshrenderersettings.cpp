/***************************************************************************
                         qgsmeshrenderersettings.cpp
                         ---------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderersettings.h"

bool QgsMeshRendererMeshSettings::isEnabled() const
{
  return mEnabled;
}

void QgsMeshRendererMeshSettings::setEnabled( bool on )
{
  mEnabled = on;
}

double QgsMeshRendererMeshSettings::lineWidth() const
{
  return mLineWidth;
}

void QgsMeshRendererMeshSettings::setLineWidth( double lineWidth )
{
  mLineWidth = lineWidth;
}

QColor QgsMeshRendererMeshSettings::color() const
{
  return mColor;
}

void QgsMeshRendererMeshSettings::setColor( const QColor &color )
{
  mColor = color;
}


QgsColorRampShader QgsMeshRendererScalarSettings::colorRampShader() const
{
  return mColorRampShader;

}

void QgsMeshRendererScalarSettings::setColorRampShader( const QgsColorRampShader &shader )
{
  mColorRampShader = shader;
}

bool QgsMeshRendererScalarSettings::isEnabled() const
{
  return mEnabled;
}

void QgsMeshRendererScalarSettings::setEnabled( bool enabled )
{
  mEnabled = enabled;
}


double QgsMeshRendererVectorSettings::lineWidth() const
{
  return mLineWidth;
}

void QgsMeshRendererVectorSettings::setLineWidth( double lineWidth )
{
  mLineWidth = lineWidth;
}

QColor QgsMeshRendererVectorSettings::color() const
{
  return mColor;
}

void QgsMeshRendererVectorSettings::setColor( const QColor &vectorColor )
{
  mColor = vectorColor;
}

double QgsMeshRendererVectorSettings::filterMin() const
{
  return mFilterMin;
}

void QgsMeshRendererVectorSettings::setFilterMin( double vectorFilterMin )
{
  mFilterMin = vectorFilterMin;
}

double QgsMeshRendererVectorSettings::filterMax() const
{
  return mFilterMax;
}

void QgsMeshRendererVectorSettings::setFilterMax( double vectorFilterMax )
{
  mFilterMax = vectorFilterMax;
}

QgsMeshRendererVectorSettings::ArrowScalingMethod QgsMeshRendererVectorSettings::shaftLengthMethod() const
{
  return mShaftLengthMethod;
}

void QgsMeshRendererVectorSettings::setShaftLengthMethod( QgsMeshRendererVectorSettings::ArrowScalingMethod shaftLengthMethod )
{
  mShaftLengthMethod = shaftLengthMethod;
}

double QgsMeshRendererVectorSettings::minShaftLength() const
{
  return mMinShaftLength;
}

void QgsMeshRendererVectorSettings::setMinShaftLength( double minShaftLength )
{
  mMinShaftLength = minShaftLength;
}

double QgsMeshRendererVectorSettings::maxShaftLength() const
{
  return mMaxShaftLength;
}

void QgsMeshRendererVectorSettings::setMaxShaftLength( double maxShaftLength )
{
  mMaxShaftLength = maxShaftLength;
}

double QgsMeshRendererVectorSettings::scaleFactor() const
{
  return mScaleFactor;
}

void QgsMeshRendererVectorSettings::setScaleFactor( double scaleFactor )
{
  mScaleFactor = scaleFactor;
}

double QgsMeshRendererVectorSettings::fixedShaftLength() const
{
  return mFixedShaftLength;
}

void QgsMeshRendererVectorSettings::setFixedShaftLength( double fixedShaftLength )
{
  mFixedShaftLength = fixedShaftLength;
}

double QgsMeshRendererVectorSettings::arrowHeadWidthRatio() const
{
  return mArrowHeadWidthRatio;
}

void QgsMeshRendererVectorSettings::setArrowHeadWidthRatio( double vectorHeadWidthRatio )
{
  mArrowHeadWidthRatio = vectorHeadWidthRatio;
}

double QgsMeshRendererVectorSettings::arrowHeadLengthRatio() const
{
  return mArrowHeadLengthRatio;
}

void QgsMeshRendererVectorSettings::setArrowHeadLengthRatio( double vectorHeadLengthRatio )
{
  mArrowHeadLengthRatio = vectorHeadLengthRatio;
}
