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

namespace QgsWfs
{
  //
  // QgsWfsParameter
  //
  QgsWfsParameter::QgsWfsParameter( const QgsWfsParameter::Name name,
                                    const QVariant::Type type,
                                    const QVariant defaultValue )
    : QgsServerParameterDefinition( type, defaultValue )
    , mName( name )
  {
  }

  int QgsWfsParameter::toInt() const
  {
    bool ok = false;
    const int val = QgsServerParameterDefinition::toInt( ok );

    if ( !ok )
    {
      raiseError();
    }

    return val;
  }

  QgsRectangle QgsWfsParameter::toRectangle() const
  {
    QString value = toString();
    const QStringList corners = mValue.toString().split( ',' );
    if ( corners.size() == 5 )
    {
      value.resize( value.size() - corners[4].size() - 1 );
    }

    QgsServerParameterDefinition param;
    param.mValue = QVariant( value );

    bool ok = false;
    const QgsRectangle rectangle = param.toRectangle( ok );

    if ( !ok )
    {
      const QString msg = QString( "%1 ('%2') cannot be converted into rectangle" ).arg( name( mName ), toString() );
      QgsServerParameterDefinition::raiseError( msg );
    }

    return rectangle;
  }

  QStringList QgsWfsParameter::toStringListWithExp( const QString &exp ) const
  {
    QStringList theList;

    const QString val = mValue.toString();
    if ( val.isEmpty() )
      return theList;

    if ( exp.isEmpty() )
      theList << val;
    else
    {
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
    }

    return theList;
  }

  void QgsWfsParameter::raiseError() const
  {
    const QString msg = QString( "%1 ('%2') cannot be converted into %3" ).arg( name( mName ), toString(), typeName() );
    QgsServerParameterDefinition::raiseError( msg );
  }

  QString QgsWfsParameter::name( const QgsWfsParameter::Name name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWfsParameter::Name>() );
    return metaEnum.valueToKey( name );
  }

  QgsWfsParameter::Name QgsWfsParameter::name( const QString &name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWfsParameter::Name>() );
    return ( QgsWfsParameter::Name ) metaEnum.keyToValue( name.toUpper().toStdString().c_str() );
  }

  //
  // QgsWfsParameters
  //
  QgsWfsParameters::QgsWfsParameters()
    : QgsServerParameters()
  {
    // Available version number
    mVersions.append( QgsProjectVersion( 1, 0, 0 ) );
    mVersions.append( QgsProjectVersion( 1, 1, 0 ) );

    const QgsWfsParameter pOutputFormat = QgsWfsParameter( QgsWfsParameter::OUTPUTFORMAT );
    save( pOutputFormat );

    const QgsWfsParameter pResultType = QgsWfsParameter( QgsWfsParameter::RESULTTYPE );
    save( pResultType );

    const QgsWfsParameter pPropertyName = QgsWfsParameter( QgsWfsParameter::PROPERTYNAME );
    save( pPropertyName );

    const QgsWfsParameter pMaxFeatures = QgsWfsParameter( QgsWfsParameter::MAXFEATURES,
                                         QVariant::Int,
                                         QVariant( -1 ) );
    save( pMaxFeatures );

    const QgsWfsParameter pStartIndex = QgsWfsParameter( QgsWfsParameter::STARTINDEX,
                                        QVariant::Int,
                                        QVariant( 0 ) );
    save( pStartIndex );

    const QgsWfsParameter pSrsName = QgsWfsParameter( QgsWfsParameter::SRSNAME );
    save( pSrsName );

    const QgsWfsParameter pTypeName = QgsWfsParameter( QgsWfsParameter::TYPENAME );
    save( pTypeName );

    const QgsWfsParameter pFeatureId = QgsWfsParameter( QgsWfsParameter::FEATUREID );
    save( pFeatureId );

    const QgsWfsParameter pFilter = QgsWfsParameter( QgsWfsParameter::FILTER );
    save( pFilter );

    const QgsWfsParameter pBbox = QgsWfsParameter( QgsWfsParameter::BBOX );
    save( pBbox );

    const QgsWfsParameter pSortBy = QgsWfsParameter( QgsWfsParameter::SORTBY );
    save( pSortBy );

    const QgsWfsParameter pExpFilter = QgsWfsParameter( QgsWfsParameter::EXP_FILTER );
    save( pExpFilter );

    const QgsWfsParameter pGeometryName = QgsWfsParameter( QgsWfsParameter::GEOMETRYNAME );
    save( pGeometryName );
  }

  QgsWfsParameters::QgsWfsParameters( const QgsServerParameters &parameters )
    : QgsWfsParameters()
  {
    load( parameters.urlQuery() );
  }

  bool QgsWfsParameters::loadParameter( const QString &key, const QString &value )
  {
    bool loaded = false;

    const QgsWfsParameter::Name name = QgsWfsParameter::name( key );
    if ( name >= 0 )
    {
      mWfsParameters[name].mValue = value;
      if ( ! mWfsParameters[name].isValid() )
      {
        mWfsParameters[name].raiseError();
      }

      loaded = true;
    }

    return loaded;
  }

  void QgsWfsParameters::save( const QgsWfsParameter &parameter )
  {
    mWfsParameters[ parameter.mName ] = parameter;
  }

  void QgsWfsParameters::dump() const
  {
    log( "WFS Request parameters:" );
    const auto map = mWfsParameters.toStdMap();
    for ( const auto &parameter : map )
    {
      const QString value = parameter.second.toString();

      if ( ! value.isEmpty() )
      {
        const QString name = QgsWfsParameter::name( parameter.first );
        log( QStringLiteral( " - %1 : %2" ).arg( name, value ) );
      }
    }

    if ( !version().isEmpty() )
      log( QStringLiteral( " - VERSION : %1" ).arg( version() ) );
  }

  QString QgsWfsParameters::outputFormatAsString() const
  {
    return mWfsParameters[ QgsWfsParameter::OUTPUTFORMAT ].toString();
  }

  QgsWfsParameters::Format QgsWfsParameters::outputFormat() const
  {
    const QString fStr = outputFormatAsString();

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
    else if ( fStr.compare( QLatin1String( "application/vnd.geo+json" ), Qt::CaseInsensitive ) == 0 ||
              // Needs to check for space too, because a + sign in the query string is interpreted as a space
              fStr.compare( QLatin1String( "application/vnd.geo json" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "application/geo+json" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "application/geo json" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "application/json" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "geojson" ), Qt::CaseInsensitive ) == 0
            )
      f = Format::GeoJSON;
    else if ( fStr.compare( QLatin1String( "gml2" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML2;
    else if ( fStr.compare( QLatin1String( "gml3" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML3;

    if ( f == Format::NONE &&
         request().compare( QLatin1String( "describefeaturetype" ), Qt::CaseInsensitive ) == 0 &&
         fStr.compare( QLatin1String( "xmlschema" ), Qt::CaseInsensitive ) == 0 )
      f = Format::GML2;

    return f;
  }

  QString QgsWfsParameters::resultTypeAsString() const
  {
    return mWfsParameters[ QgsWfsParameter::RESULTTYPE ].toString();
  }

  QgsWfsParameters::ResultType QgsWfsParameters::resultType() const
  {
    const QString rtStr = resultTypeAsString();
    if ( rtStr.isEmpty() )
      return ResultType::RESULTS;

    ResultType rt = ResultType::RESULTS;
    if ( rtStr.compare( QLatin1String( "hits" ), Qt::CaseInsensitive ) == 0 )
      rt = ResultType::HITS;
    return rt;
  }

  QStringList QgsWfsParameters::propertyNames() const
  {
    return mWfsParameters[ QgsWfsParameter::PROPERTYNAME ].toStringListWithExp();
  }

  QString QgsWfsParameters::maxFeatures() const
  {
    return mWfsParameters[ QgsWfsParameter::MAXFEATURES ].toString();
  }

  int QgsWfsParameters::maxFeaturesAsInt() const
  {
    return mWfsParameters[ QgsWfsParameter::MAXFEATURES ].toInt();
  }

  QString QgsWfsParameters::startIndex() const
  {
    return mWfsParameters[ QgsWfsParameter::STARTINDEX ].toString();
  }

  int QgsWfsParameters::startIndexAsInt() const
  {
    return mWfsParameters[ QgsWfsParameter::STARTINDEX ].toInt();
  }

  QString QgsWfsParameters::srsName() const
  {
    return mWfsParameters[ QgsWfsParameter::SRSNAME ].toString();
  }

  QStringList QgsWfsParameters::typeNames() const
  {
    return mWfsParameters[ QgsWfsParameter::TYPENAME ].toStringList();
  }

  QStringList QgsWfsParameters::featureIds() const
  {
    return mWfsParameters[ QgsWfsParameter::FEATUREID ].toStringList();
  }

  QStringList QgsWfsParameters::filters() const
  {
    return mWfsParameters[ QgsWfsParameter::FILTER ].toStringListWithExp();
  }

  QString QgsWfsParameters::bbox() const
  {
    return mWfsParameters[ QgsWfsParameter::BBOX ].toString();
  }

  QgsRectangle QgsWfsParameters::bboxAsRectangle() const
  {
    return mWfsParameters[ QgsWfsParameter::BBOX ].toRectangle();
  }

  QStringList QgsWfsParameters::sortBy() const
  {
    return mWfsParameters[ QgsWfsParameter::SORTBY ].toStringListWithExp();
  }

  QStringList QgsWfsParameters::expFilters() const
  {
    return mWfsParameters[ QgsWfsParameter::EXP_FILTER ].toExpressionList();
  }

  QString QgsWfsParameters::geometryNameAsString() const
  {
    return mWfsParameters[ QgsWfsParameter::GEOMETRYNAME ].toString();
  }

  QgsProjectVersion QgsWfsParameters::versionAsNumber() const
  {
    const QString vStr = version();
    QgsProjectVersion version;

    if ( vStr.isEmpty() )
      version = QgsProjectVersion( 1, 1, 0 ); // default value
    else if ( mVersions.contains( QgsProjectVersion( vStr ) ) )
      version = QgsProjectVersion( vStr );

    return version;
  }

  void QgsWfsParameters::log( const QString &msg ) const
  {
    QgsMessageLog::logMessage( msg, "Server", Qgis::MessageLevel::Info );
  }
}
