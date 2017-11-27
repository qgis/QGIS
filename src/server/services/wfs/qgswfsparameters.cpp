/***************************************************************************
                              qgswfsparameters.cpp
                              --------------------
  begin                : Sept 14, 2017
  copyright            : (C) 2017 by René-Luc Dhont
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

#include "qgswfsparameters.h"
#include "qgsmessagelog.h"
#include <iostream>

namespace QgsWfs
{
  QgsWfsParameters::QgsWfsParameters()
  {
    // Available version number
    mVersions.append( QgsProjectVersion( 1, 0, 0 ) );
    mVersions.append( QgsProjectVersion( 1, 1, 0 ) );

    const Parameter pOutputFormat = { ParameterName::OUTPUTFORMAT,
                                      QVariant::String,
                                      QVariant( "" ),
                                      QVariant()
                                    };
    save( pOutputFormat );

    const Parameter pResultType = { ParameterName::RESULTTYPE,
                                    QVariant::String,
                                    QVariant( "" ),
                                    QVariant()
                                  };
    save( pResultType );

    const Parameter pPropertyName = { ParameterName::PROPERTYNAME,
                                      QVariant::String,
                                      QVariant( "" ),
                                      QVariant()
                                    };
    save( pPropertyName );

    const Parameter pMaxFeatures = { ParameterName::MAXFEATURES,
                                     QVariant::Int,
                                     QVariant( -1 ),
                                     QVariant()
                                   };
    save( pMaxFeatures );

    const Parameter pStartIndex = { ParameterName::STARTINDEX,
                                    QVariant::Int,
                                    QVariant( 0 ),
                                    QVariant()
                                  };
    save( pStartIndex );

    const Parameter pSrsName = { ParameterName::SRSNAME,
                                 QVariant::String,
                                 QVariant( "" ),
                                 QVariant()
                               };
    save( pSrsName );

    const Parameter pTypeName = { ParameterName::TYPENAME,
                                  QVariant::String,
                                  QVariant( "" ),
                                  QVariant()
                                };
    save( pTypeName );

    const Parameter pFeatureId = { ParameterName::FEATUREID,
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
    save( pFeatureId );

    const Parameter pFilter = { ParameterName::FILTER,
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
    save( pFilter );

    const Parameter pBbox = { ParameterName::BBOX,
                              QVariant::String,
                              QVariant( "" ),
                              QVariant()
                            };
    save( pBbox );

    const Parameter pSortBy = { ParameterName::SORTBY,
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
    save( pSortBy );

    const Parameter pExpFilter = { ParameterName::EXP_FILTER,
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
    save( pExpFilter );

    const Parameter pGeometryName = { ParameterName::GEOMETRYNAME,
                                      QVariant::String,
                                      QVariant( "" ),
                                      QVariant()
                                    };
    save( pGeometryName );
  }

  QgsWfsParameters::QgsWfsParameters( const QgsServerRequest::Parameters &parameters ) : QgsWfsParameters()
  {
    load( parameters );
  }

  void QgsWfsParameters::load( const QgsServerRequest::Parameters &parameters )
  {
    mRequestParameters = parameters;

    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
    foreach ( QString key, parameters.keys() )
    {
      const ParameterName name = ( ParameterName ) metaEnum.keyToValue( key.toStdString().c_str() );
      if ( name >= 0 )
      {
        QVariant value( parameters[key] );
        if ( value.canConvert( mParameters[name].mType ) )
        {
          mParameters[name].mValue = value;
        }
        else
        {
          raiseError( name );
        }
      }
    }
  }

  void QgsWfsParameters::dump() const
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );

    log( "WFS Request parameters:" );
    for ( auto parameter : mParameters.toStdMap() )
    {
      const QString value = parameter.second.mValue.toString();

      if ( ! value.isEmpty() )
      {
        const QString name = metaEnum.valueToKey( parameter.first );
        log( " - " + name + " : " + value );
      }
    }

    if ( !version().isEmpty() )
      log( " - VERSION : " + version() );
  }

  void QgsWfsParameters::save( const Parameter &parameter )
  {
    mParameters[ parameter.mName ] = parameter;
  }

  QVariant QgsWfsParameters::value( ParameterName name ) const
  {
    return mParameters[name].mValue;
  }

  QVariant QgsWfsParameters::defaultValue( ParameterName name ) const
  {
    return mParameters[name].mDefaultValue;
  }

  QString QgsWfsParameters::outputFormatAsString() const
  {
    return value( ParameterName::OUTPUTFORMAT ).toString();
  }

  QgsWfsParameters::Format QgsWfsParameters::outputFormat() const
  {
    QString fStr = outputFormatAsString();

    if ( fStr.isEmpty() )
    {
      if ( versionAsNumber() >= QgsProjectVersion( 1, 1, 0 ) )
        return Format::GML3;
      else
        return Format::GML2;
    }

    Format f = Format::NONE;
    if ( fStr.compare( QLatin1String( "text/xml; subtype=gml/2.1.2" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML2;
    else if ( fStr.compare( QLatin1String( "text/xml; subtype=gml/3.1.1" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML3;
    else if ( fStr.compare( QLatin1String( "application/vnd.geo+json" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GeoJSON;
    else if ( fStr.compare( QLatin1String( "gml2" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML2;
    else if ( fStr.compare( QLatin1String( "gml3" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML3;
    else if ( fStr.compare( QLatin1String( "geojson" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GeoJSON;

    if ( f == Format::NONE &&
         request().compare( QLatin1String( "describefeaturetype" ), Qt::CaseInsensitive ) == 0 &&
         fStr.compare( QLatin1String( "xmlschema" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML2;

    return f;
  }

  QString QgsWfsParameters::resultTypeAsString() const
  {
    return value( ParameterName::RESULTTYPE ).toString();
  }

  QgsWfsParameters::ResultType QgsWfsParameters::resultType() const
  {
    QString rtStr = resultTypeAsString();
    if ( rtStr.isEmpty() )
      return ResultType::RESULTS;

    ResultType rt = ResultType::RESULTS;
    if ( rtStr.compare( QLatin1String( "hits" ), Qt::CaseInsensitive ) == 0 )
      rt = ResultType::HITS;
    return rt;
  }

  QStringList QgsWfsParameters::propertyNames() const
  {
    return toStringListWithExp( ParameterName::PROPERTYNAME );
  }

  QString QgsWfsParameters::maxFeatures() const
  {
    return value( ParameterName::MAXFEATURES ).toString();
  }

  int QgsWfsParameters::maxFeaturesAsInt() const
  {
    return toInt( ParameterName::MAXFEATURES );
  }

  QString QgsWfsParameters::startIndex() const
  {
    return value( ParameterName::STARTINDEX ).toString();
  }

  int QgsWfsParameters::startIndexAsInt() const
  {
    return toInt( ParameterName::STARTINDEX );
  }

  QString QgsWfsParameters::srsName() const
  {
    return value( ParameterName::SRSNAME ).toString();
  }

  QStringList QgsWfsParameters::typeNames() const
  {
    return toStringList( ParameterName::TYPENAME );
  }

  QStringList QgsWfsParameters::featureIds() const
  {
    return toStringList( ParameterName::FEATUREID );
  }

  QStringList QgsWfsParameters::filters() const
  {
    return toStringListWithExp( ParameterName::FILTER );
  }

  QString QgsWfsParameters::bbox() const
  {
    return value( ParameterName::BBOX ).toString();
  }

  QgsRectangle QgsWfsParameters::bboxAsRectangle() const
  {
    return toRectangle( ParameterName::BBOX );
  }

  QStringList QgsWfsParameters::sortBy() const
  {
    return toStringListWithExp( ParameterName::SORTBY );
  }

  QStringList QgsWfsParameters::expFilters() const
  {
    return toStringListWithExp( ParameterName::EXP_FILTER );
  }

  QString QgsWfsParameters::geometryNameAsString() const
  {
    return value( ParameterName::GEOMETRYNAME ).toString();
  }

  QString QgsWfsParameters::request() const
  {
    if ( mRequestParameters.contains( "REQUEST" ) )
      return mRequestParameters["REQUEST"];
    else
      return QString();
  }

  QString QgsWfsParameters::version() const
  {
    // VERSION parameter is not managed with other parameters because
    // there's a conflict with qgis VERSION defined in qgsconfig.h
    if ( mRequestParameters.contains( "VERSION" ) )
      return mRequestParameters["VERSION"];
    else
      return QString();
  }

  QgsProjectVersion QgsWfsParameters::versionAsNumber() const
  {
    QString vStr = version();
    QgsProjectVersion version;

    if ( vStr.isEmpty() )
      version = QgsProjectVersion( 1, 1, 0 ); // default value
    else if ( mVersions.contains( QgsProjectVersion( vStr ) ) )
      version = QgsProjectVersion( vStr );

    return version;
  }

  QString QgsWfsParameters::name( ParameterName name ) const
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
    return metaEnum.valueToKey( name );
  }

  int QgsWfsParameters::toInt( const QVariant &value, const QVariant &defaultValue, bool *error ) const
  {
    int val = defaultValue.toInt();
    QString valStr = value.toString();
    bool ok = true;

    if ( !valStr.isEmpty() )
    {
      val = value.toInt( &ok );
    }
    *error = !ok;

    return val;
  }

  int QgsWfsParameters::toInt( ParameterName p ) const
  {
    bool error;
    int val = toInt( value( p ), defaultValue( p ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into int";
      raiseError( msg );
    }

    return val;
  }

  QgsRectangle QgsWfsParameters::toRectangle( const QVariant &value, bool *error ) const
  {
    *error = false;
    QString bbox = value.toString();
    QgsRectangle extent;

    if ( !bbox.isEmpty() )
    {
      QStringList corners = bbox.split( "," );

      if ( corners.size() == 4 )
      {
        double d[4];
        bool ok;

        for ( int i = 0; i < 4; i++ )
        {
          corners[i].replace( QLatin1String( " " ), QLatin1String( "+" ) );
          d[i] = corners[i].toDouble( &ok );
          if ( !ok )
          {
            *error = !ok;
            return extent;
          }
        }

        extent = QgsRectangle( d[0], d[1], d[2], d[3] );
      }
      else
      {
        *error = true;
        return extent;
      }
    }

    return extent;
  }

  QgsRectangle QgsWfsParameters::toRectangle( ParameterName p ) const
  {
    bool error;
    QgsRectangle extent = toRectangle( value( p ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into a rectangle";
      raiseError( msg );
    }

    return extent;
  }

  QStringList QgsWfsParameters::toStringList( ParameterName p, char delimiter ) const
  {
    return value( p ).toString().split( delimiter, QString::SkipEmptyParts );
  }

  QStringList QgsWfsParameters::toStringListWithExp( ParameterName p, const QString &exp ) const
  {
    QStringList theList;

    QString val = value( p ).toString();
    if ( val.isEmpty() )
      return theList;

    QRegExp rx( exp );
    if ( rx.indexIn( val, 0 ) == -1 )
    {
      theList << val;
    }
    else
    {
      int pos = 0;
      while ( ( pos = rx.indexIn( val, pos ) ) != -1 )
      {
        theList << rx.cap( 1 );
        pos += rx.matchedLength();
      }
    }
    return theList;
  }

  void QgsWfsParameters::log( const QString &msg ) const
  {
    QgsMessageLog::logMessage( msg, "Server", QgsMessageLog::INFO );
  }

  void QgsWfsParameters::raiseError( ParameterName paramName ) const
  {
    const QString value = mParameters[paramName].mValue.toString();
    const QString param = name( paramName );
    const QString type = QVariant::typeToName( mParameters[paramName].mType );
    raiseError( param + " ('" + value + "') cannot be converted into " + type );
  }

  void QgsWfsParameters::raiseError( const QString &msg ) const
  {
    throw QgsBadRequestException( QStringLiteral( "Invalid WFS Parameter" ), msg );
  }
}
