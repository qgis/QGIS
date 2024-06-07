/***************************************************************************
    qgskeyvaluewidgetfactory.cpp
     --------------------------------------
    Date                 : 08.2016
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

#include "qgskeyvaluewidgetfactory.h"
#include "qgskeyvaluewidgetwrapper.h"
#include "qgsdummyconfigdlg.h"
#include "qgsfields.h"
#include "qgsvectorlayer.h"

#include <QVariant>
#include <QSettings>

QgsKeyValueWidgetFactory::QgsKeyValueWidgetFactory( const QString &name ):
  QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsKeyValueWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsKeyValueWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsKeyValueWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  Q_UNUSED( parent )
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Key/Value field" ) );
}

unsigned int QgsKeyValueWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  if ( field.type() == QMetaType::Type::QVariantMap && ( field.typeName().compare( QStringLiteral( "JSON" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 || field.subType() == QMetaType::Type::QString ) )
  {
    // Fetch the first not-null value and check if it is really a map
    QgsFeatureRequest req;
    req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    req.setSubsetOfAttributes( { fieldIdx } );
    req.setFilterExpression( QStringLiteral( R"("%1" IS NOT NULL)" ).arg( field.name() ) );
    req.setLimit( 1 );
    QgsFeature f;
    QgsFeatureIterator featureIt { vl->getFeatures( req ) };
    if ( featureIt.nextFeature( f ) )
    {
      // Get attribute value and check if it is a valid JSON object
      const QVariant value { f.attribute( fieldIdx ) };
      switch ( value.type() )
      {
        case QVariant::Type::Map:
        {
          return 20;
        }
        default:
        case QVariant::Type::String:
        {
          const QJsonDocument doc = QJsonDocument::fromJson( value.toString().toUtf8() );
          if ( doc.isObject() )
          {
            return 20;
          }
          else
          {
            return 0;
          }
        }
      }
    }
  }
  return field.type() == QMetaType::Type::QVariantMap && field.subType() != QMetaType::Type::UnknownType ? 20 : 0;
}
