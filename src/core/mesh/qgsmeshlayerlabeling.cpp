/***************************************************************************
                         qgsmeshlayerlabeling.cpp
                         ---------------------
    begin                : November 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmeshlayerlabeling.h"

#include "qgspallabeling.h"
#include "qgsmeshlayer.h"
#include "qgis.h"
#include "qgsstyleentityvisitor.h"
#include "qgsmeshlayerlabelprovider.h"

QgsAbstractMeshLayerLabeling *QgsAbstractMeshLayerLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString type = element.attribute( QStringLiteral( "type" ) );
  if ( type == QLatin1String( "simple" ) )
  {
    return QgsMeshLayerSimpleLabeling::create( element, context );
  }
  else
  {
    return nullptr;
  }
}

bool QgsAbstractMeshLayerLabeling::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
}

QgsPalLayerSettings QgsAbstractMeshLayerLabeling::defaultSettingsForLayer( const QgsMeshLayer *layer )
{
  QgsPalLayerSettings settings;
  settings.setFormat( QgsStyle::defaultTextFormatForProject( layer->project() ) );
  return settings;
}


///


QgsMeshLayerSimpleLabeling::QgsMeshLayerSimpleLabeling( const QgsPalLayerSettings &settings, bool labelFaces )
  : mSettings( new QgsPalLayerSettings( settings ) )
  , mLabelFaces( labelFaces )
{
}

QString QgsMeshLayerSimpleLabeling::type() const
{
  return QStringLiteral( "simple" );
}

QgsMeshLayerSimpleLabeling *QgsMeshLayerSimpleLabeling::clone() const
{
  return new QgsMeshLayerSimpleLabeling( *mSettings, mLabelFaces );
}

QgsMeshLayerLabelProvider *QgsMeshLayerSimpleLabeling::provider( QgsMeshLayer *layer ) const
{
  return new QgsMeshLayerLabelProvider( layer, QString(), mSettings.get(), QString(), mLabelFaces );
}

QDomElement QgsMeshLayerSimpleLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "labeling" ) );
  elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "simple" ) );
  elem.setAttribute( QStringLiteral( "labelFaces" ), mLabelFaces ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.appendChild( mSettings->writeXml( doc, context ) );
  return elem;
}

QgsPalLayerSettings QgsMeshLayerSimpleLabeling::settings( const QString &providerId ) const
{
  Q_UNUSED( providerId )
  return *mSettings;
}

bool QgsMeshLayerSimpleLabeling::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mSettings )
  {
    QgsStyleLabelSettingsEntity entity( *mSettings );
    if ( !visitor->visit( &entity ) )
      return false;
  }
  return true;
}

bool QgsMeshLayerSimpleLabeling::requiresAdvancedEffects() const
{
  return mSettings->containsAdvancedEffects();
}

QgsMeshLayerSimpleLabeling *QgsMeshLayerSimpleLabeling::create( const QDomElement &element, const QgsReadWriteContext &context ) // cppcheck-suppress duplInheritedMember
{
  const QDomElement settingsElem = element.firstChildElement( QStringLiteral( "settings" ) );
  if ( !settingsElem.isNull() )
  {
    QgsPalLayerSettings settings;
    settings.readXml( settingsElem, context );
    const bool labelFaces = element.attribute( QStringLiteral( "labelFaces" ), QStringLiteral( "0" ) ).toInt();
    return new QgsMeshLayerSimpleLabeling( settings, labelFaces );
  }

  return new QgsMeshLayerSimpleLabeling( QgsPalLayerSettings() );
}

void QgsMeshLayerSimpleLabeling::multiplyOpacity( double opacityFactor )
{
  QgsTextFormat format { mSettings->format() };
  format.multiplyOpacity( opacityFactor );
  mSettings->setFormat( format );
}

void QgsMeshLayerSimpleLabeling::setSettings( QgsPalLayerSettings *settings, const QString &providerId )
{
  Q_UNUSED( providerId )

  if ( mSettings.get() == settings )
    return;

  mSettings.reset( settings );
}
