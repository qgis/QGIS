/***************************************************************************
    qgsvectorfieldarrowsettings.cpp
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorfieldarrowsettings.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsVectorFieldArrowSettings::ArrowScalingMethod QgsVectorFieldArrowSettings::shaftLengthMethod() const
{
  return mShaftLengthMethod;
}

void QgsVectorFieldArrowSettings::setShaftLengthMethod( QgsVectorFieldArrowSettings::ArrowScalingMethod shaftLengthMethod )
{
  mShaftLengthMethod = shaftLengthMethod;
}

double QgsVectorFieldArrowSettings::minShaftLength() const
{
  return mMinShaftLength;
}

void QgsVectorFieldArrowSettings::setMinShaftLength( double minShaftLength )
{
  mMinShaftLength = minShaftLength;
}

double QgsVectorFieldArrowSettings::maxShaftLength() const
{
  return mMaxShaftLength;
}

void QgsVectorFieldArrowSettings::setMaxShaftLength( double maxShaftLength )
{
  mMaxShaftLength = maxShaftLength;
}

double QgsVectorFieldArrowSettings::scaleFactor() const
{
  return mScaleFactor;
}

void QgsVectorFieldArrowSettings::setScaleFactor( double scaleFactor )
{
  mScaleFactor = scaleFactor;
}

double QgsVectorFieldArrowSettings::fixedShaftLength() const
{
  return mFixedShaftLength;
}

void QgsVectorFieldArrowSettings::setFixedShaftLength( double fixedShaftLength )
{
  mFixedShaftLength = fixedShaftLength;
}

double QgsVectorFieldArrowSettings::arrowHeadWidthRatio() const
{
  return mArrowHeadWidthRatio;
}

void QgsVectorFieldArrowSettings::setArrowHeadWidthRatio( double vectorHeadWidthRatio )
{
  mArrowHeadWidthRatio = vectorHeadWidthRatio;
}

double QgsVectorFieldArrowSettings::arrowHeadLengthRatio() const
{
  return mArrowHeadLengthRatio;
}

void QgsVectorFieldArrowSettings::setArrowHeadLengthRatio( double vectorHeadLengthRatio )
{
  mArrowHeadLengthRatio = vectorHeadLengthRatio;
}

QDomElement QgsVectorFieldArrowSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-arrow-settings"_s );
  elem.setAttribute( u"arrow-head-width-ratio"_s, mArrowHeadWidthRatio );
  elem.setAttribute( u"arrow-head-length-ratio"_s, mArrowHeadLengthRatio );

  QDomElement elemShaft = doc.createElement( u"shaft-length"_s );
  QString methodTxt;
  switch ( mShaftLengthMethod )
  {
    case ArrowScalingMethod::MinMax:
      methodTxt = u"minmax"_s;
      elemShaft.setAttribute( u"min"_s, mMinShaftLength );
      elemShaft.setAttribute( u"max"_s, mMaxShaftLength );
      break;
    case ArrowScalingMethod::Scaled:
      methodTxt = u"scaled"_s;
      elemShaft.setAttribute( u"scale-factor"_s, mScaleFactor );
      break;
    case ArrowScalingMethod::Fixed:
      methodTxt = u"fixed"_s;
      elemShaft.setAttribute( u"fixed-length"_s, mFixedShaftLength );
      break;
  }
  elemShaft.setAttribute( u"method"_s, methodTxt );
  elem.appendChild( elemShaft );
  return elem;
}

void QgsVectorFieldArrowSettings::readXml( const QDomElement &elem )
{
  mArrowHeadWidthRatio = elem.attribute( u"arrow-head-width-ratio"_s ).toDouble();
  mArrowHeadLengthRatio = elem.attribute( u"arrow-head-length-ratio"_s ).toDouble();

  const QDomElement elemShaft = elem.firstChildElement( u"shaft-length"_s );
  const QString methodTxt = elemShaft.attribute( u"method"_s );
  if ( u"minmax"_s == methodTxt )
  {
    mShaftLengthMethod = ArrowScalingMethod::MinMax;
    mMinShaftLength = elemShaft.attribute( u"min"_s ).toDouble();
    mMaxShaftLength = elemShaft.attribute( u"max"_s ).toDouble();
  }
  else if ( u"scaled"_s == methodTxt )
  {
    mShaftLengthMethod = ArrowScalingMethod::Scaled;
    mScaleFactor = elemShaft.attribute( u"scale-factor"_s ).toDouble();
  }
  else // fixed
  {
    mShaftLengthMethod = ArrowScalingMethod::Fixed;
    mFixedShaftLength = elemShaft.attribute( u"fixed-length"_s ).toDouble();
  }
}
