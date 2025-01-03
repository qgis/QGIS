/***************************************************************************
                              qgswfsdescribefeaturetype.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2012 by René-Luc D'Hont    (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QDomDocument>
#include <QDomElement>

#include "qgswfsutils.h"
#include "qgsserverprojectutils.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfsdescribefeaturetypegml.h"
#include "qgswfsdescribefeaturetypejson.h"
#include "qgswfsparameters.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsdatetimefieldformatter.h"

namespace QgsWfs
{
  void writeDescribeFeatureType( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response )
  {
    const QgsWfsParameters wfsParameters( QUrlQuery( request.url() ) );
    const QgsWfsParameters::Format oFormat = wfsParameters.outputFormat();

    // test oFormat
    switch ( oFormat )
    {
      case QgsWfsParameters::Format::GML2:
      case QgsWfsParameters::Format::GML3:
      {
        auto formatter = QgsWfsDescribeFeatureTypeGml( wfsParameters );
        formatter.writeDescribeFeatureType( serverIface, project, version, request, response );
      }
      break;

      case QgsWfsParameters::Format::GeoJSON:
      {
        auto formatter = QgsWfsDescribeFeatureTypeJson( wfsParameters );
        formatter.writeDescribeFeatureType( serverIface, project, version, request, response );
      }
      break;

      default:
        throw QgsBadRequestException( QStringLiteral( "Invalid WFS Parameter" ), QStringLiteral( "OUTPUTFORMAT %1 is not supported" ).arg( wfsParameters.outputFormatAsString() ) );
    }
  }

  QStringList getRequestTypeNames( const QgsServerRequest &request, const QgsWfsParameters &wfsParams )
  {
    QStringList typeNameList;
    QDomDocument queryDoc;
    QString errorMsg;
    if ( queryDoc.setContent( request.data(), true, &errorMsg ) )
    {
      //read doc
      const QDomElement queryDocElem = queryDoc.documentElement();
      const QDomNodeList docChildNodes = queryDocElem.childNodes();
      if ( docChildNodes.size() )
      {
        for ( int i = 0; i < docChildNodes.size(); i++ )
        {
          const QDomElement docChildElem = docChildNodes.at( i ).toElement();
          if ( docChildElem.tagName() == QLatin1String( "TypeName" ) )
          {
            const QString typeName = docChildElem.text().trimmed();
            if ( typeName.contains( ':' ) )
              typeNameList << typeName.section( ':', 1, 1 );
            else
              typeNameList << typeName;
          }
        }
      }
    }
    else
    {
      typeNameList = wfsParams.typeNames();
    }

    return typeNameList;
  }


  void getFieldAttributes( const QgsField &field, QString &fieldName, QString &fieldType )
  {
    fieldName = field.name();

    const thread_local QRegularExpression sCleanTagNameRegExp( QStringLiteral( "[^\\w\\.-_]" ), QRegularExpression::PatternOption::UseUnicodePropertiesOption );
    fieldName.replace( ' ', '_' ).replace( sCleanTagNameRegExp, QString() );

    const QMetaType::Type attributeType = field.type();

    if ( attributeType == QMetaType::Type::Int )
    {
      fieldType = QStringLiteral( "int" );
    }
    else if ( attributeType == QMetaType::Type::UInt )
    {
      fieldType = QStringLiteral( "unsignedInt" );
    }
    else if ( attributeType == QMetaType::Type::LongLong )
    {
      fieldType = QStringLiteral( "long" );
    }
    else if ( attributeType == QMetaType::Type::ULongLong )
    {
      fieldType = QStringLiteral( "unsignedLong" );
    }
    else if ( attributeType == QMetaType::Type::Double )
    {
      if ( field.length() > 0 && field.precision() == 0 )
        fieldType = QStringLiteral( "integer" );
      else
        fieldType = QStringLiteral( "decimal" );
    }
    else if ( attributeType == QMetaType::Type::Bool )
    {
      fieldType = QStringLiteral( "boolean" );
    }
    else if ( attributeType == QMetaType::Type::QDate )
    {
      fieldType = QStringLiteral( "date" );
    }
    else if ( attributeType == QMetaType::Type::QTime )
    {
      fieldType = QStringLiteral( "time" );
    }
    else if ( attributeType == QMetaType::Type::QDateTime )
    {
      fieldType = QStringLiteral( "dateTime" );
    }
    else
    {
      fieldType = QStringLiteral( "string" );
    }

    const QgsEditorWidgetSetup setup = field.editorWidgetSetup();
    if ( setup.type() == QStringLiteral( "DateTime" ) )
    {
      // Get editor widget setup config
      const QVariantMap config = setup.config();
      // Get field format from editor widget setup config
      const QString fieldFormat = config.value(
                                          QStringLiteral( "field_format" ),
                                          QgsDateTimeFieldFormatter::defaultFormat( field.type() )
      )
                                    .toString();
      // Define type from field format
      if ( fieldFormat == QgsDateTimeFieldFormatter::TIME_FORMAT ) // const QgsDateTimeFieldFormatter::TIME_FORMAT
        fieldType = QStringLiteral( "time" );
      else if ( fieldFormat == QgsDateTimeFieldFormatter::DATE_FORMAT ) // const QgsDateTimeFieldFormatter::DATE_FORMAT since QGIS 3.30
        fieldType = QStringLiteral( "date" );
      else if ( fieldFormat == QgsDateTimeFieldFormatter::DATETIME_FORMAT ) // const QgsDateTimeFieldFormatter::DATETIME_FORMAT since QGIS 3.30
        fieldType = QStringLiteral( "dateTime" );
      else if ( fieldFormat == QgsDateTimeFieldFormatter::QT_ISO_FORMAT )
        fieldType = QStringLiteral( "dateTime" );
    }
    else if ( setup.type() == QStringLiteral( "Range" ) )
    {
      const QVariantMap config = setup.config();
      if ( config.contains( QStringLiteral( "Precision" ) ) )
      {
        // if precision in range config is not the same as the attributePrec
        // we need to update type
        bool ok;
        const int configPrec( config[QStringLiteral( "Precision" )].toInt( &ok ) );
        if ( ok && configPrec != field.precision() )
        {
          if ( configPrec == 0 )
            fieldType = QStringLiteral( "integer" );
          else
            fieldType = QStringLiteral( "decimal" );
        }
      }
    }
  }


} // namespace QgsWfs
