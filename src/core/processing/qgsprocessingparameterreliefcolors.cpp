/***************************************************************************
  qgsprocessingparameterreliefcolors.cpp
  ---------------------
  Date                 : June 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameterreliefcolors.h"

#include "qgsprocessingcontext.h"

#include <QString>

using namespace Qt::StringLiterals;


QgsProcessingParameterReliefColors::QgsProcessingParameterReliefColors( const QString &name, const QString &description, const QString &parentLayerParameter, bool optional )
  : QgsProcessingParameterString( name, description, QVariant(), false, optional )
  , mParentLayer( parentLayerParameter )
{}

QgsProcessingParameterReliefColors *QgsProcessingParameterReliefColors::clone() const
{
  return new QgsProcessingParameterReliefColors( name(), description(), mParentLayer, flags() & Qgis::ProcessingParameterFlag::Optional );
}

QString QgsProcessingParameterReliefColors::type() const
{
  return typeName();
}

QString QgsProcessingParameterReliefColors::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code
        = u"QgsProcessingParameterReliefColors('%1', %2, %3"_s.arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ), QgsProcessingUtils::stringToPythonLiteral( mParentLayer ) );
      if ( mFlags & Qgis::ProcessingParameterFlag::Optional )
        code += ", optional=True"_L1;
      code += ')'_L1;
      return code;
    }
  }
  return QString();
}

bool QgsProcessingParameterReliefColors::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext * ) const
{
  if ( !input.isValid() && !mDefault.isValid() )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  const QString stringValue = input.toString();
  if ( stringValue.isEmpty() || ( !input.isValid() && mDefault.userType() == QMetaType::Type::QString && mDefault.toString().isEmpty() ) )
    return mFlags & Qgis::ProcessingParameterFlag::Optional;

  const QStringList colorParts = stringValue.split( ';' );
  for ( const QString &part : colorParts )
  {
    if ( part.trimmed().isEmpty() )
      return false;

    const QStringList v = part.split( ',' );
    if ( v.count() < 5 )
    {
      return false;
    }

    bool ok = false;
    ( void ) v.at( 0 ).toDouble( &ok );
    if ( !ok )
      return false;
    ( void ) v.at( 1 ).toDouble( &ok );
    if ( !ok )
      return false;
    ( void ) v.at( 2 ).toInt( &ok );
    if ( !ok )
      return false;
    ( void ) v.at( 3 ).toInt( &ok );
    if ( !ok )
      return false;
    ( void ) v.at( 4 ).toInt( &ok );
    if ( !ok )
      return false;
  }

  return true;
}

QVariantMap QgsProcessingParameterReliefColors::toVariantMap() const
{
  QVariantMap map = QgsProcessingParameterDefinition::toVariantMap(); // NOLINT(bugprone-parent-virtual-call) clazy:exclude=skipped-base-method
  map.insert( u"parent"_s, mParentLayer );
  return map;
}

bool QgsProcessingParameterReliefColors::fromVariantMap( const QVariantMap &map )
{
  QgsProcessingParameterDefinition::fromVariantMap( map ); // NOLINT(bugprone-parent-virtual-call) clazy:exclude=skipped-base-method
  mParentLayer = map.value( u"parent"_s ).toString();
  return true;
}

QList<QgsRasterReliefColor> QgsProcessingParameterReliefColors::valueAsReliefColors( const QVariant &value, const QgsProcessingContext &context ) const
{
  QList<QgsRasterReliefColor> reliefColors;
  const QString colorsString = QgsProcessingParameters::parameterAsString( this, value, context );
  const QStringList colorParts = colorsString.split( ';' );
  for ( const QString &part : colorParts )
  {
    if ( part.trimmed().isEmpty() )
      continue;

    const QStringList v = part.split( ',' );
    if ( v.count() >= 5 )
    {
      const double minElev = v.at( 0 ).toDouble();
      const double maxElev = v.at( 1 ).toDouble();
      const QColor color( v.at( 2 ).toInt(), v.at( 3 ).toInt(), v.at( 4 ).toInt() );
      reliefColors.append( QgsRasterReliefColor( color, minElev, maxElev ) );
    }
  }
  return reliefColors;
}

QVariant QgsProcessingParameterReliefColors::colorsAsVariant( const QList<QgsRasterReliefColor> &colors )
{
  QStringList parts;
  parts.reserve( colors.size() );
  for ( const QgsRasterReliefColor &c : colors )
  {
    parts << u"%1,%2,%3,%4,%5"_s.arg( qgsDoubleToString( c.minElevation ), qgsDoubleToString( c.maxElevation ) ).arg( c.color.red() ).arg( c.color.green() ).arg( c.color.blue() );
  }
  return parts.join( ';' );
}
