/***************************************************************************
                         qgsprocessingmodeloutput.cpp
                         ----------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmodeloutput.h"

///@cond NOT_STABLE

QgsProcessingModelOutput::QgsProcessingModelOutput( const QString &name, const QString &description )
  : QgsProcessingModelComponent( description )
  , mName( name )
{}

QgsProcessingModelOutput *QgsProcessingModelOutput::clone() const
{
  return new QgsProcessingModelOutput( *this );
}

QVariant QgsProcessingModelOutput::toVariant() const
{
  QVariantMap map;
  map.insert( u"name"_s, mName );

  if ( mDefaultValue.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
  {
    QVariantMap defaultMap = mDefaultValue.value<QgsProcessingOutputLayerDefinition>().toVariant().toMap();
    defaultMap.insert( u"class"_s, u"QgsProcessingOutputLayerDefinition"_s );
    map.insert( u"default_value"_s, defaultMap );
  }
  else
  {
    map.insert( u"default_value"_s, mDefaultValue );
  }

  map.insert( u"child_id"_s, mChildId );
  map.insert( u"output_name"_s, mOutputName );
  map.insert( u"mandatory"_s, mMandatory );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelOutput::loadVariant( const QVariantMap &map )
{
  mName = map.value( u"name"_s ).toString();

  const QVariant defaultValue = map.value( u"default_value"_s );
  if ( defaultValue.userType() == QMetaType::Type::QVariantMap )
  {
    QVariantMap defaultMap = defaultValue.toMap();
    if ( defaultMap["class"] == "QgsProcessingOutputLayerDefinition"_L1 )
    {
      QgsProcessingOutputLayerDefinition value;
      value.loadVariant( defaultMap );
      mDefaultValue = QVariant( value );
    }
    else
    {
      mDefaultValue = QVariant();
    }
  }
  else
  {
    mDefaultValue = map.value( u"default_value"_s );
  }

  mChildId = map.value( u"child_id"_s ).toString();
  mOutputName = map.value( u"output_name"_s ).toString();
  mMandatory = map.value( u"mandatory"_s, false ).toBool();
  restoreCommonProperties( map );
  return true;
}


///@endcond
