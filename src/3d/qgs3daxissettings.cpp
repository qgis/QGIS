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

#include <QDomDocument>

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

Qgs3DAxisSettings::Qgs3DAxisSettings( const Qgs3DAxisSettings &other )
  : mMode( other.mMode )
  , mHorizontalPosition( other.mHorizontalPosition )
  , mVerticalPosition( other.mVerticalPosition )
{

}

Qgs3DAxisSettings &Qgs3DAxisSettings::operator=( Qgs3DAxisSettings const &rhs )
{
  this->mMode = rhs.mMode;
  this->mHorizontalPosition = rhs.mHorizontalPosition;
  this->mVerticalPosition = rhs.mVerticalPosition;
  return *this;
}

bool Qgs3DAxisSettings::operator==( Qgs3DAxisSettings const &rhs ) const
{
  bool out = true;
  out &= this->mMode == rhs.mMode;
  out &= this->mHorizontalPosition == rhs.mHorizontalPosition;
  out &= this->mVerticalPosition == rhs.mVerticalPosition;
  return out;
}

bool Qgs3DAxisSettings::operator!=( Qgs3DAxisSettings const &rhs ) const
{
  return ! this->operator==( rhs );
}

void Qgs3DAxisSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  QString sizeStr = element.attribute( QStringLiteral( "defaultViewportSize" ) );
  if ( !sizeStr.isEmpty() )
    mDefaultViewportSize = sizeStr.toInt();

  sizeStr = element.attribute( QStringLiteral( "minViewportRatio" ) );
  if ( !sizeStr.isEmpty() )
    mMinViewportRatio = sizeStr.toInt();

  sizeStr = element.attribute( QStringLiteral( "maxViewportRatio" ) );
  if ( !sizeStr.isEmpty() )
    mMaxViewportRatio = sizeStr.toInt();

  const QString modeStr = element.attribute( QStringLiteral( "mode" ) );
  if ( modeStr == QLatin1String( "Off" ) )
    mMode = Qgs3DAxisSettings::Mode::Off;
  else if ( modeStr == QLatin1String( "Crs" ) )
    mMode = Qgs3DAxisSettings::Mode::Crs;
  else if ( modeStr == QLatin1String( "Cube" ) )
    mMode = Qgs3DAxisSettings::Mode::Cube;

  const QString horizontalStr = element.attribute( QStringLiteral( "horizontal" ) );
  if ( horizontalStr == QLatin1String( "Left" ) )
    mHorizontalPosition = Qt::AnchorPoint::AnchorLeft;
  else if ( horizontalStr == QLatin1String( "Middle" ) )
    mHorizontalPosition = Qt::AnchorPoint::AnchorHorizontalCenter;
  else if ( horizontalStr == QLatin1String( "Right" ) )
    mHorizontalPosition = Qt::AnchorPoint::AnchorRight;

  const QString verticalStr = element.attribute( QStringLiteral( "vertical" ) );
  if ( verticalStr == QLatin1String( "Top" ) )
    mVerticalPosition = Qt::AnchorPoint::AnchorTop;
  else if ( verticalStr == QLatin1String( "Middle" ) )
    mVerticalPosition = Qt::AnchorPoint::AnchorVerticalCenter;
  else if ( verticalStr == QLatin1String( "Bottom" ) )
    mVerticalPosition = Qt::AnchorPoint::AnchorBottom;
}

void Qgs3DAxisSettings::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  QString str;

  str = QString::number( mDefaultViewportSize );
  element.setAttribute( QStringLiteral( "defaultViewportSize" ), str );

  str = QString::number( mMinViewportRatio );
  element.setAttribute( QStringLiteral( "minViewportRatio" ), str );

  str = QString::number( mMaxViewportRatio );
  element.setAttribute( QStringLiteral( "maxViewportRatio" ), str );

  switch ( mMode )
  {
    case Qgs3DAxisSettings::Mode::Crs:
      str = QLatin1String( "Crs" );
      break;
    case Qgs3DAxisSettings::Mode::Cube:
      str = QLatin1String( "Cube" );
      break;

    case Qgs3DAxisSettings::Mode::Off:
    default:
      str = QLatin1String( "Off" );
      break;
  }
  element.setAttribute( QStringLiteral( "mode" ), str );

  switch ( mHorizontalPosition )
  {
    case Qt::AnchorPoint::AnchorLeft:
      str = QLatin1String( "Left" );
      break;
    case Qt::AnchorPoint::AnchorHorizontalCenter:
      str = QLatin1String( "Middle" );
      break;
    case Qt::AnchorPoint::AnchorRight:
    default:
      str = QLatin1String( "End" );
      break;
  }
  element.setAttribute( QStringLiteral( "horizontal" ), str );

  switch ( mVerticalPosition )
  {
    case Qt::AnchorPoint::AnchorBottom:
      str = QLatin1String( "Bottom" );
      break;
    case Qt::AnchorPoint::AnchorVerticalCenter:
      str = QLatin1String( "Middle" );
      break;
    case Qt::AnchorPoint::AnchorTop:
    default:
      str = QLatin1String( "Top" );
      break;
  }
  element.setAttribute( QStringLiteral( "vertical" ), str );

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
