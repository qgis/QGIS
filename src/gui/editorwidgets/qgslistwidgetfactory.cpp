/***************************************************************************
    qgslistwidgetfactory.cpp
     --------------------------------------
    Date                 : 09.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslistwidgetfactory.h"

#include "qgseditorwidgetregistry.h"
#include "qgsfields.h"
#include "qgslistconfigdlg.h"
#include "qgslistwidgetwrapper.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QString>
#include <QVariant>
#include <qjsonarray.h>

using namespace Qt::StringLiterals;

QgsListWidgetFactory::QgsListWidgetFactory( const QString &name, const QIcon &icon )
  : QgsEditorWidgetFactory( name, icon )
{}

QgsEditorWidgetWrapper *QgsListWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsListWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsListWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsListConfigDlg( vl, fieldIdx, parent );
}

unsigned int QgsListWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  // Handle the json field
  if ( field.typeName().compare( u"json"_s, Qt::CaseInsensitive ) == 0 || field.typeName().compare( u"jsonb"_s, Qt::CaseInsensitive ) == 0 )
  {
    // Look the first not-null value (limiting to the first 20 features) and check if it is really an array
    const int MAX_FEATURE_LIMIT { 20 };
    QgsFeatureRequest req;
    req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    req.setSubsetOfAttributes( { fieldIdx } );
    req.setLimit( MAX_FEATURE_LIMIT );
    QgsFeature f;
    QgsFeatureIterator featureIt { vl->getFeatures( req ) };
    // The counter is an extra safety measure in case the provider does not respect the limit
    int featureCount = 0;
    bool foundNotNull = false;
    bool foundInvalidValue = false; // An invalid value is any value that is not a JSON list or a list with nested or invalid values
    while ( featureIt.nextFeature( f ) )
    {
      ++featureCount;
      if ( featureCount > MAX_FEATURE_LIMIT )
      {
        break;
      }
      // Get attribute value and check if it is a valid JSON array
      const QVariant value( f.attribute( fieldIdx ) );
      if ( !value.isNull() )
      {
        foundNotNull = true;

        // Read the list from a string or a list
        QJsonArray jsonArray;
        bool validArray = false;
        if ( value.type() == QVariant::Type::List )
        {
          validArray = true;
          jsonArray = QJsonArray::fromVariantList( value.toList() );
        }
        else
        {
          const QJsonDocument doc = QJsonDocument::fromJson( value.toString().toUtf8() );
          if ( doc.isArray() )
          {
            validArray = true;
            jsonArray = doc.array();
          }
        }

        if ( validArray ) // empty array [] counts as valid as well
        {
          // It's a array(list), check the first 20 entries if flat (not nested)
          int count = 0;
          for ( auto it = jsonArray.begin(); it != jsonArray.end(); ++it )
          {
            if ( count >= 20 )
            {
              break;
            }
            count++;

            QJsonValue childValue = *it;
            if ( childValue.isObject() || childValue.isArray() || childValue.isUndefined() )
            {
              foundInvalidValue = true;
              break;
            }
          }
          if ( foundInvalidValue )
          {
            break;
          }
        }
        else
        {
          // Stop when we find the first not-array
          foundInvalidValue = true;
          break;
        }
      }
    }
    if ( foundNotNull )
    {
      if ( !foundInvalidValue )
      {
        return 20;
      }
      else
      {
        return 5;
      }
    }
    else
    {
      return 10;
    }
  }

  return ( field.type() == QMetaType::Type::QVariantList || field.type() == QMetaType::Type::QStringList || field.type() == QMetaType::Type::QVariantMap )
             && field.subType() != QMetaType::Type::UnknownType
           ? 20
           : 0;
}
