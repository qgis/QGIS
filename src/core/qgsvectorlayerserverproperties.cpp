/***************************************************************************
                             qgsvectorlayerserverproperties.cpp
                              ------------------
  begin                : August 23, 2019
  copyright            : (C) 2019 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerserverproperties.h"
#include "qgsvectorlayer.h"

#include <QDomNode>

QgsVectorLayerServerProperties::QgsVectorLayerServerProperties( QgsVectorLayer *layer )
  : mLayer( layer )
{
}

QMap<int, QString> QgsVectorLayerServerProperties::wmsDimensionDefaultDisplayLabels()
{
  QMap<int, QString> labels;
  labels[QgsVectorLayerServerProperties::WmsDimensionInfo::AllValues] = QObject::tr( "All values" );
  labels[QgsVectorLayerServerProperties::WmsDimensionInfo::MinValue] = QObject::tr( "Min value" );
  labels[QgsVectorLayerServerProperties::WmsDimensionInfo::MaxValue] = QObject::tr( "Max value" );
  labels[QgsVectorLayerServerProperties::WmsDimensionInfo::ReferenceValue] = QObject::tr( "Reference value" );
  return labels;
}

bool QgsVectorLayerServerProperties::addWmsDimension( const QgsVectorLayerServerProperties::WmsDimensionInfo &wmsDimInfo )
{
  for ( const QgsVectorLayerServerProperties::WmsDimensionInfo &dim : mWmsDimensions )
  {
    if ( dim.name == wmsDimInfo.name )
    {
      return false;
    }
  }
  mWmsDimensions.append( wmsDimInfo );
  return true;
}

bool QgsVectorLayerServerProperties::removeWmsDimension( const QString &wmsDimName )
{
  for ( int i = 0; i < mWmsDimensions.size(); ++i )
  {
    if ( mWmsDimensions[ i ].name == wmsDimName )
    {
      mWmsDimensions.removeAt( i );
      return true;
    }
  }
  return false;
}

const QList< QgsVectorLayerServerProperties::WmsDimensionInfo > QgsVectorLayerServerProperties::wmsDimensions() const
{
  return mWmsDimensions;
}

void QgsVectorLayerServerProperties::readXml( const QDomNode &layer_node )
{
  const QgsFields fields = mLayer->fields();
  // QGIS Server WMS Dimensions
  const QDomNode wmsDimsNode = layer_node.namedItem( QStringLiteral( "wmsDimensions" ) );
  if ( !wmsDimsNode.isNull() )
  {
    const QDomElement wmsDimsElem = wmsDimsNode.toElement();
    QDomNodeList wmsDimsList = wmsDimsElem.elementsByTagName( QStringLiteral( "dimension" ) );
    for ( int i = 0; i < wmsDimsList.size(); ++i )
    {
      QDomElement dimElem = wmsDimsList.at( i ).toElement();
      QString dimName = dimElem.attribute( QStringLiteral( "name" ) );
      QString dimFieldName = dimElem.attribute( QStringLiteral( "fieldName" ) );
      // check field name
      int dimFieldNameIndex = fields.indexOf( dimFieldName );
      if ( dimFieldNameIndex == -1 )
      {
        continue;
      }
      QVariant dimRefValue;
      int dimDefaultDisplayType = dimElem.attribute( QStringLiteral( "defaultDisplayType" ) ).toInt();
      if ( dimDefaultDisplayType == QgsVectorLayerServerProperties::WmsDimensionInfo::AllValues )
      {
        QString dimRefValueStr = dimElem.attribute( QStringLiteral( "referenceValue" ) );
        if ( !dimRefValueStr.isEmpty() )
        {
          QgsField dimField = fields.at( dimFieldNameIndex );
          dimRefValue = QVariant( dimRefValueStr );
          if ( !dimField.convertCompatible( dimRefValue ) )
          {
            continue;
          }
        }
      }
      QgsVectorLayerServerProperties::WmsDimensionInfo dim( dimName, dimFieldName,
          dimElem.attribute( QStringLiteral( "endFieldName" ) ),
          dimElem.attribute( QStringLiteral( "units" ) ),
          dimElem.attribute( QStringLiteral( "unitSymbol" ) ),
          dimDefaultDisplayType, dimRefValue );
      addWmsDimension( dim );
    }
  }
}

void QgsVectorLayerServerProperties::writeXml( QDomNode &layer_node, QDomDocument &document ) const
{
  // save QGIS Server WMS Dimension definitions
  if ( mWmsDimensions.size() > 0 )
  {
    QDomElement wmsDimsElem = document.createElement( QStringLiteral( "wmsDimensions" ) );
    for ( const QgsVectorLayerServerProperties::WmsDimensionInfo &dim : mWmsDimensions )
    {
      QDomElement dimElem = document.createElement( QStringLiteral( "dimension" ) );
      dimElem.setAttribute( QStringLiteral( "name" ), dim.name );
      dimElem.setAttribute( QStringLiteral( "fieldName" ), dim.fieldName );
      dimElem.setAttribute( QStringLiteral( "endFieldName" ), dim.endFieldName );
      dimElem.setAttribute( QStringLiteral( "units" ), dim.units );
      dimElem.setAttribute( QStringLiteral( "unitSymbol" ), dim.unitSymbol );
      dimElem.setAttribute( QStringLiteral( "defaultDisplayType" ), dim.defaultDisplayType );
      dimElem.setAttribute( QStringLiteral( "referenceValue" ), dim.referenceValue.toString() );
      wmsDimsElem.appendChild( dimElem );
    }
    layer_node.appendChild( wmsDimsElem );
  }
}
