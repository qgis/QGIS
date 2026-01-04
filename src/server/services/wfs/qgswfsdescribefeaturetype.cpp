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
#include "qgswfsdescribefeaturetype.h"

#include "qgsdatetimefieldformatter.h"
#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgsvectorlayer.h"
#include "qgswfsdescribefeaturetypegml.h"
#include "qgswfsdescribefeaturetypejson.h"
#include "qgswfsparameters.h"
#include "qgswfsutils.h"

#include <QDomDocument>
#include <QDomElement>

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
        throw QgsBadRequestException( u"Invalid WFS Parameter"_s, u"OUTPUTFORMAT %1 is not supported"_s.arg( wfsParameters.outputFormatAsString() ) );
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
          if ( docChildElem.tagName() == "TypeName"_L1 )
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

    const thread_local QRegularExpression sCleanTagNameRegExp( u"[^\\w\\.-_]"_s, QRegularExpression::PatternOption::UseUnicodePropertiesOption );
    fieldName.replace( ' ', '_' ).replace( sCleanTagNameRegExp, QString() );

    const QMetaType::Type attributeType = field.type();

    if ( attributeType == QMetaType::Type::Int )
    {
      fieldType = u"int"_s;
    }
    else if ( attributeType == QMetaType::Type::UInt )
    {
      fieldType = u"unsignedInt"_s;
    }
    else if ( attributeType == QMetaType::Type::LongLong )
    {
      fieldType = u"long"_s;
    }
    else if ( attributeType == QMetaType::Type::ULongLong )
    {
      fieldType = u"unsignedLong"_s;
    }
    else if ( attributeType == QMetaType::Type::Double )
    {
      if ( field.length() > 0 && field.precision() == 0 )
        fieldType = u"integer"_s;
      else
        fieldType = u"decimal"_s;
    }
    else if ( attributeType == QMetaType::Type::Bool )
    {
      fieldType = u"boolean"_s;
    }
    else if ( attributeType == QMetaType::Type::QDate )
    {
      fieldType = u"date"_s;
    }
    else if ( attributeType == QMetaType::Type::QTime )
    {
      fieldType = u"time"_s;
    }
    else if ( attributeType == QMetaType::Type::QDateTime )
    {
      fieldType = u"dateTime"_s;
    }
    else
    {
      fieldType = u"string"_s;
    }

    const QgsEditorWidgetSetup setup = field.editorWidgetSetup();
    if ( setup.type() == "DateTime"_L1 )
    {
      // Get editor widget setup config
      const QVariantMap config = setup.config();
      // Get field format from editor widget setup config
      const QString fieldFormat = config.value(
                                          u"field_format"_s,
                                          QgsDateTimeFieldFormatter::defaultFormat( field.type() )
      )
                                    .toString();
      // Define type from field format
      if ( fieldFormat == QgsDateTimeFieldFormatter::TIME_FORMAT ) // const QgsDateTimeFieldFormatter::TIME_FORMAT
        fieldType = u"time"_s;
      else if ( fieldFormat == QgsDateTimeFieldFormatter::DATE_FORMAT ) // const QgsDateTimeFieldFormatter::DATE_FORMAT since QGIS 3.30
        fieldType = u"date"_s;
      else if ( fieldFormat == QgsDateTimeFieldFormatter::DATETIME_FORMAT ) // const QgsDateTimeFieldFormatter::DATETIME_FORMAT since QGIS 3.30
        fieldType = u"dateTime"_s;
      else if ( fieldFormat == QgsDateTimeFieldFormatter::QT_ISO_FORMAT )
        fieldType = u"dateTime"_s;
    }
    else if ( setup.type() == "Range"_L1 )
    {
      const QVariantMap config = setup.config();
      if ( config.contains( u"Precision"_s ) )
      {
        // if precision in range config is not the same as the attributePrec
        // we need to update type
        bool ok;
        const int configPrec( config[u"Precision"_s].toInt( &ok ) );
        if ( ok && configPrec != field.precision() )
        {
          if ( configPrec == 0 )
            fieldType = u"integer"_s;
          else
            fieldType = u"decimal"_s;
        }
      }
    }
  }


} // namespace QgsWfs
