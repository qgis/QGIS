/***************************************************************************
    qgsvectorlayerlabeling.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectorlayerlabeling.h"

#include "qgspallabeling.h"
#include "qgsrulebasedlabeling.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include "qgis.h"


QgsAbstractVectorLayerLabeling *QgsAbstractVectorLayerLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  QString type = element.attribute( QStringLiteral( "type" ) );
  if ( type == QLatin1String( "rule-based" ) )
  {
    return QgsRuleBasedLabeling::create( element, context );
  }
  else if ( type == QLatin1String( "simple" ) )
  {
    return QgsVectorLayerSimpleLabeling::create( element, context );
  }
  else
  {
    return nullptr;
  }
}

QgsVectorLayerLabelProvider *QgsVectorLayerSimpleLabeling::provider( QgsVectorLayer *layer ) const
{
  return new QgsVectorLayerLabelProvider( layer, QString(), false, mSettings.get() );
}

QgsVectorLayerSimpleLabeling::QgsVectorLayerSimpleLabeling( const QgsPalLayerSettings &settings )
  : mSettings( new QgsPalLayerSettings( settings ) )
{

}

QString QgsVectorLayerSimpleLabeling::type() const
{
  return QStringLiteral( "simple" );
}

QgsAbstractVectorLayerLabeling *QgsVectorLayerSimpleLabeling::clone() const
{
  return new QgsVectorLayerSimpleLabeling( *mSettings );
}

QDomElement QgsVectorLayerSimpleLabeling::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "labeling" ) );
  elem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "simple" ) );
  elem.appendChild( mSettings->writeXml( doc, context ) );
  return elem;
}

QgsPalLayerSettings QgsVectorLayerSimpleLabeling::settings( const QString &providerId ) const
{
  Q_UNUSED( providerId );
  return *mSettings;
}

bool QgsVectorLayerSimpleLabeling::requiresAdvancedEffects() const
{
  return mSettings->format().containsAdvancedEffects();
}

QgsVectorLayerSimpleLabeling *QgsVectorLayerSimpleLabeling::create( const QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement settingsElem = element.firstChildElement( QStringLiteral( "settings" ) );
  if ( !settingsElem.isNull() )
  {
    QgsPalLayerSettings settings;
    settings.readXml( settingsElem, context );
    return new QgsVectorLayerSimpleLabeling( settings );
  }

  return new QgsVectorLayerSimpleLabeling( QgsPalLayerSettings() );
}

void QgsVectorLayerSimpleLabeling::toSld( QDomNode &parent, const QgsStringMap &props ) const
{

  if ( mSettings->drawLabels )
  {
    QDomDocument doc = parent.ownerDocument();

    QDomElement ruleElement = doc.createElement( QStringLiteral( "se:Rule" ) );
    parent.appendChild( ruleElement );

    QDomElement textSymbolizerElement = doc.createElement( QStringLiteral( "se:TextSymbolizer" ) );
    ruleElement.appendChild( textSymbolizerElement );

    // label
    QDomElement labelElement = doc.createElement( QStringLiteral( "se:Label" ) );
    textSymbolizerElement.appendChild( labelElement );

    if ( mSettings->isExpression )
    {
      labelElement.appendChild( doc.createComment( QStringLiteral( "SE Export for %1 not implemented yet" ).arg( mSettings->getLabelExpression()->dump() ) ) );
      labelElement.appendChild( doc.createTextNode( "Placeholder" ) );
    }
    else
    {
      QDomElement propertyNameElement = doc.createElement( QStringLiteral( "ogc:PropertyName" ) );
      propertyNameElement.appendChild( doc.createTextNode( mSettings->fieldName ) );
      labelElement.appendChild( propertyNameElement );
    }

    // font
    QDomElement fontElement = doc.createElement( QStringLiteral( "se:Font" ) );
    textSymbolizerElement.appendChild( fontElement );
    QgsTextFormat format = mSettings->format();
    fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "font-family" ), format.font().family() ) );
    double fontSize = QgsSymbolLayerUtils::rescaleUom( format.size(), format.sizeUnit(), props );
    fontElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "font-size" ), QString::number( fontSize ) ) );


    // fill
    QDomElement fillElement = doc.createElement( QStringLiteral( "se:Fill" ) );
    textSymbolizerElement.appendChild( fillElement );
    fillElement.appendChild( QgsSymbolLayerUtils::createSvgParameterElement( doc, QStringLiteral( "fill" ), format.color().name() ) );
  }


}
