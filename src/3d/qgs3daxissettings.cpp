/***************************************************************************
  qgs3daxissettings.cpp
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

#include "qgs3daxissettings.h"

#include "qgsreadwritecontext.h"

#include <QDomDocument>

bool Qgs3DAxisSettings::operator==( Qgs3DAxisSettings const &rhs ) const
{
  bool out = true;
  out &= this->mMode == rhs.mMode;
  out &= this->mHorizontalPosition == rhs.mHorizontalPosition;
  out &= this->mVerticalPosition == rhs.mVerticalPosition;
  out &= this->mDefaultViewportSize == rhs.mDefaultViewportSize;
  out &= this->mMaxViewportRatio == rhs.mMaxViewportRatio;
  out &= this->mMinViewportRatio == rhs.mMinViewportRatio;
  return out;
}

bool Qgs3DAxisSettings::operator!=( Qgs3DAxisSettings const &rhs ) const
{
  return !this->operator==( rhs );
}

void Qgs3DAxisSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  QString sizeStr = element.attribute( u"defaultViewportSize"_s );
  if ( !sizeStr.isEmpty() )
    mDefaultViewportSize = sizeStr.toInt();

  double minViewportRatio = 0.0;
  double maxViewportRatio = 0.0;
  sizeStr = element.attribute( u"minViewportRatio"_s );
  if ( !sizeStr.isEmpty() && sizeStr.toDouble() != 0.0 )
    minViewportRatio = sizeStr.toDouble();

  sizeStr = element.attribute( u"maxViewportRatio"_s );
  if ( !sizeStr.isEmpty() && sizeStr.toDouble() != 0.0 )
    maxViewportRatio = sizeStr.toDouble();

  if ( maxViewportRatio > 0.0 )
    mMaxViewportRatio = maxViewportRatio;

  if ( minViewportRatio > 0.0 && mMaxViewportRatio > minViewportRatio )
    mMinViewportRatio = minViewportRatio;

  const QString modeStr = element.attribute( u"mode"_s );
  if ( modeStr == "Off"_L1 )
    mMode = Qgs3DAxisSettings::Mode::Off;
  else if ( modeStr == "Crs"_L1 )
    mMode = Qgs3DAxisSettings::Mode::Crs;
  else if ( modeStr == "Cube"_L1 )
    mMode = Qgs3DAxisSettings::Mode::Cube;

  const QString horizontalStr = element.attribute( u"horizontal"_s );
  if ( horizontalStr == "Left"_L1 )
    mHorizontalPosition = Qt::AnchorPoint::AnchorLeft;
  else if ( horizontalStr == "Middle"_L1 )
    mHorizontalPosition = Qt::AnchorPoint::AnchorHorizontalCenter;
  else if ( horizontalStr == "Right"_L1 )
    mHorizontalPosition = Qt::AnchorPoint::AnchorRight;

  const QString verticalStr = element.attribute( u"vertical"_s );
  if ( verticalStr == "Top"_L1 )
    mVerticalPosition = Qt::AnchorPoint::AnchorTop;
  else if ( verticalStr == "Middle"_L1 )
    mVerticalPosition = Qt::AnchorPoint::AnchorVerticalCenter;
  else if ( verticalStr == "Bottom"_L1 )
    mVerticalPosition = Qt::AnchorPoint::AnchorBottom;
}

void Qgs3DAxisSettings::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  QString str;

  str = QString::number( mDefaultViewportSize );
  element.setAttribute( u"defaultViewportSize"_s, str );

  str = QString::number( mMinViewportRatio );
  element.setAttribute( u"minViewportRatio"_s, str );

  str = QString::number( mMaxViewportRatio );
  element.setAttribute( u"maxViewportRatio"_s, str );

  switch ( mMode )
  {
    case Qgs3DAxisSettings::Mode::Crs:
      str = "Crs"_L1;
      break;
    case Qgs3DAxisSettings::Mode::Cube:
      str = "Cube"_L1;
      break;

    case Qgs3DAxisSettings::Mode::Off:
    default:
      str = "Off"_L1;
      break;
  }
  element.setAttribute( u"mode"_s, str );

  switch ( mHorizontalPosition )
  {
    case Qt::AnchorPoint::AnchorLeft:
      str = "Left"_L1;
      break;
    case Qt::AnchorPoint::AnchorHorizontalCenter:
      str = "Middle"_L1;
      break;
    case Qt::AnchorPoint::AnchorRight:
    default:
      str = "End"_L1;
      break;
  }
  element.setAttribute( u"horizontal"_s, str );

  switch ( mVerticalPosition )
  {
    case Qt::AnchorPoint::AnchorBottom:
      str = "Bottom"_L1;
      break;
    case Qt::AnchorPoint::AnchorVerticalCenter:
      str = "Middle"_L1;
      break;
    case Qt::AnchorPoint::AnchorTop:
    default:
      str = "Top"_L1;
      break;
  }
  element.setAttribute( u"vertical"_s, str );
}

void Qgs3DAxisSettings::setMinViewportRatio( double ratio )
{
  if ( ratio < mMaxViewportRatio )
  {
    mMinViewportRatio = std::clamp( ratio, 0.0, 1.0 );
  }
}

void Qgs3DAxisSettings::setMaxViewportRatio( double ratio )
{
  if ( ratio > mMinViewportRatio )
  {
    mMaxViewportRatio = std::clamp( ratio, 0.0, 1.0 );
  }
}
