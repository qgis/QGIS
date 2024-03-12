/***************************************************************************
                              qgswmsparameters.cpp
                              --------------------
  begin                : March 17, 2017
  copyright            : (C) 2017 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsparameters.h"
#include "qgsdatasourceuri.h"
#include "qgsmaplayerserverproperties.h"
#include "qgsmessagelog.h"
#include "qgswmsserviceexception.h"
#include "qgsfontutils.h"

#include <QRegularExpression>

const QString EXTERNAL_LAYER_PREFIX = QStringLiteral( "EXTERNAL_WMS:" );

namespace QgsWms
{
  //
  // QgsWmsParameter
  //
  QgsWmsParameter::QgsWmsParameter( const QgsWmsParameter::Name name,
                                    const QVariant::Type type,
                                    const QVariant defaultValue )
    : QgsServerParameterDefinition( type, defaultValue )
    , mName( name )
  {
  }

  bool QgsWmsParameter::isValid() const
  {
    return ( mName != QgsWmsParameter::UNKNOWN ) && QgsServerParameterDefinition::isValid();
  }

  void QgsWmsParameter::raiseError() const
  {
    const QString msg = QString( "%1 ('%2') cannot be converted into %3" ).arg( name( mName ), toString(), typeName() );
    QgsServerParameterDefinition::raiseError( msg );
  }

  QStringList QgsWmsParameter::toStyleList( const char delimiter ) const
  {
    return QgsServerParameterDefinition::toStringList( delimiter, false );
  }

  QList<QgsGeometry> QgsWmsParameter::toGeomList( const char delimiter ) const
  {
    bool ok = true;
    const QList<QgsGeometry> geoms = QgsServerParameterDefinition::toGeomList( ok, delimiter );

    if ( !ok )
    {
      const QString msg = QString( "%1 ('%2') cannot be converted into a list of geometries" ).arg( name( mName ), toString() );
      QgsServerParameterDefinition::raiseError( msg );
    }

    return geoms;
  }

  QgsRectangle QgsWmsParameter::toRectangle() const
  {
    bool ok = true;
    const QgsRectangle rect = QgsServerParameterDefinition::toRectangle( ok );

    if ( !ok )
    {
      const QString msg = QString( "%1 ('%2') cannot be converted into a rectangle" ).arg( name( mName ), toString() );
      QgsServerParameterDefinition::raiseError( msg );
    }

    return rect;
  }

  int QgsWmsParameter::toInt() const
  {
    bool ok = false;
    const int val = QgsServerParameterDefinition::toInt( ok );

    if ( !ok )
    {
      raiseError();
    }

    return val;
  }

  QString QgsWmsParameter::loadUrl() const
  {
    // Check URL -- it will be used in error messages
    const QUrl url = toUrl();

    bool ok = false;
    const QString content = QgsServerParameterDefinition::loadUrl( ok );

    if ( !ok )
    {
      const QString msg = QString( "%1 request error for %2" ).arg( name( mName ), url.toString() );
      QgsServerParameterDefinition::raiseError( msg );
    }

    return content;
  }

  QUrl QgsWmsParameter::toUrl() const
  {
    bool ok = false;
    const QUrl url = QgsServerParameterDefinition::toUrl( ok );

    if ( !ok )
    {
      raiseError();
    }

    return url;
  }

  QColor QgsWmsParameter::toColor() const
  {
    bool ok = false;
    const QColor col = QgsServerParameterDefinition::toColor( ok );

    if ( !ok )
    {
      raiseError();
    }

    return col;
  }

  QList<QColor> QgsWmsParameter::toColorList( const char delimiter ) const
  {
    bool ok = false;
    const QList<QColor> vals = QgsServerParameterDefinition::toColorList( ok, delimiter );

    if ( !ok )
    {
      const QString msg = QString( "%1 ('%2') cannot be converted into a list of colors" ).arg( name( mName ), toString() );
      QgsServerParameterDefinition::raiseError( msg );
    }

    return vals;
  }

  QList<int> QgsWmsParameter::toIntList( const char delimiter ) const
  {
    bool ok = false;
    const QList<int> vals = QgsServerParameterDefinition::toIntList( ok, delimiter );

    if ( !ok )
    {
      const QString msg = QString( "%1 ('%2') cannot be converted into a list of int" ).arg( name( mName ), toString() );
      QgsServerParameterDefinition::raiseError( msg );
    }

    return vals;
  }

  QList<double> QgsWmsParameter::toDoubleList( const char delimiter ) const
  {
    bool ok = false;
    const QList<double> vals = QgsServerParameterDefinition::toDoubleList( ok, delimiter );

    if ( !ok )
    {
      const QString msg = QString( "%1 ('%2') cannot be converted into a list of float" ).arg( name( mName ), toString() );
      QgsServerParameterDefinition::raiseError( msg );
    }

    return vals;
  }

  double QgsWmsParameter::toDouble() const
  {
    bool ok = false;
    const double val = QgsServerParameterDefinition::toDouble( ok );

    if ( !ok )
    {
      raiseError();
    }

    return val;
  }

  QString QgsWmsParameter::name() const
  {
    return QgsWmsParameter::name( mName );
  }

  QString QgsWmsParameter::name( const QgsWmsParameter::Name name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWmsParameter::Name>() );
    return metaEnum.valueToKey( name );
  }

  QgsWmsParameter::Name QgsWmsParameter::name( const QString &name )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWmsParameter::Name>() );
    return ( QgsWmsParameter::Name ) metaEnum.keyToValue( name.toUpper().toStdString().c_str() );
  }

  //
  // QgsWmsParameters
  //
  QgsWmsParameters::QgsWmsParameters()
    : QgsServerParameters()
  {
    // Available version number
    mVersions.append( QgsProjectVersion( 1, 1, 1 ) );
    mVersions.append( QgsProjectVersion( 1, 3, 0 ) );

    // WMS parameters definition
    const QgsWmsParameter pQuality( QgsWmsParameter::IMAGE_QUALITY,
                                    QVariant::Int,
                                    QVariant( 0 ) );
    save( pQuality );

    const QgsWmsParameter pTiled( QgsWmsParameter::TILED,
                                  QVariant::Bool,
                                  QVariant( false ) );
    save( pTiled );

    const QgsWmsParameter pBoxSpace( QgsWmsParameter::BOXSPACE,
                                     QVariant::Double,
                                     QVariant( 2.0 ) );
    save( pBoxSpace );

    const QgsWmsParameter pSymbSpace( QgsWmsParameter::SYMBOLSPACE,
                                      QVariant::Double,
                                      QVariant( 2.0 ) );
    save( pSymbSpace );

    const QgsWmsParameter pLayerSpace( QgsWmsParameter::LAYERSPACE,
                                       QVariant::Double,
                                       QVariant( 3.0 ) );
    save( pLayerSpace );

    const QgsWmsParameter pTitleSpace( QgsWmsParameter::LAYERTITLESPACE,
                                       QVariant::Double,
                                       QVariant( 3.0 ) );
    save( pTitleSpace );

    const QgsWmsParameter pSymbHeight( QgsWmsParameter::SYMBOLHEIGHT,
                                       QVariant::Double,
                                       QVariant( 4.0 ) );
    save( pSymbHeight );

    const QgsWmsParameter pSymbWidth( QgsWmsParameter::SYMBOLWIDTH,
                                      QVariant::Double,
                                      QVariant( 7.0 ) );
    save( pSymbWidth );

    const QgsWmsParameter pIcLabelSpace( QgsWmsParameter::ICONLABELSPACE,
                                         QVariant::Double,
                                         QVariant( 2.0 ) );
    save( pIcLabelSpace );

    const QgsWmsParameter pItFontFamily( QgsWmsParameter::ITEMFONTFAMILY );
    save( pItFontFamily );

    const QgsWmsParameter pItFontBold( QgsWmsParameter::ITEMFONTBOLD,
                                       QVariant::Bool,
                                       QVariant( false ) );
    save( pItFontBold );

    const QgsWmsParameter pItFontItalic( QgsWmsParameter::ITEMFONTITALIC,
                                         QVariant::Bool,
                                         QVariant( false ) );
    save( pItFontItalic );

    const QgsWmsParameter pItFontSize( QgsWmsParameter::ITEMFONTSIZE,
                                       QVariant::Double,
                                       QVariant( -1 ) );
    save( pItFontSize );

    const QgsWmsParameter pItFontColor( QgsWmsParameter::ITEMFONTCOLOR,
                                        QVariant::String,
                                        QVariant( "black" ) );
    save( pItFontColor );

    const QgsWmsParameter pHighlightGeom( QgsWmsParameter::HIGHLIGHT_GEOM );
    save( pHighlightGeom );

    const QgsWmsParameter pShowFeatureCount( QgsWmsParameter::SHOWFEATURECOUNT,
        QVariant::Bool,
        QVariant( false ) );
    save( pShowFeatureCount );

    const QgsWmsParameter pHighlightSymbol( QgsWmsParameter::HIGHLIGHT_SYMBOL );
    save( pHighlightSymbol );

    const QgsWmsParameter pHighlightLabel( QgsWmsParameter::HIGHLIGHT_LABELSTRING );
    save( pHighlightLabel );

    const QgsWmsParameter pHighlightColor( QgsWmsParameter::HIGHLIGHT_LABELCOLOR,
                                           QVariant::String,
                                           QVariant( "black" ) );
    save( pHighlightColor );

    const QgsWmsParameter pHighlightFontSize( QgsWmsParameter::HIGHLIGHT_LABELSIZE );
    save( pHighlightFontSize );

    const QgsWmsParameter pHighlightFontWeight( QgsWmsParameter::HIGHLIGHT_LABELWEIGHT );
    save( pHighlightFontWeight );

    const QgsWmsParameter pHighlightFont( QgsWmsParameter::HIGHLIGHT_LABELFONT );
    save( pHighlightFont );

    const QgsWmsParameter pHighlightBufferColor( QgsWmsParameter::HIGHLIGHT_LABELBUFFERCOLOR,
        QVariant::String,
        QVariant( "black" ) );
    save( pHighlightBufferColor );

    const QgsWmsParameter pHighlightBufferSize( QgsWmsParameter::HIGHLIGHT_LABELBUFFERSIZE );
    save( pHighlightBufferSize );

    const QgsWmsParameter pLabelRotation( QgsWmsParameter::HIGHLIGHT_LABEL_ROTATION, QVariant::Double );
    save( pLabelRotation );

    const QgsWmsParameter pLabelDistance( QgsWmsParameter::HIGHLIGHT_LABEL_DISTANCE, QVariant::Double );
    save( pLabelDistance );

    const QgsWmsParameter pLabelHali( QgsWmsParameter::HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT );
    save( pLabelHali );

    const QgsWmsParameter pLabelVali( QgsWmsParameter::HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT );
    save( pLabelVali );

    const QgsWmsParameter pCRS( QgsWmsParameter::CRS );
    save( pCRS );

    const QgsWmsParameter pSRS( QgsWmsParameter::SRS );
    save( pSRS );

    const QgsWmsParameter pFormat( QgsWmsParameter::FORMAT,
                                   QVariant::String,
                                   QVariant( "png" ) );
    save( pFormat );

    const QgsWmsParameter pInfoFormat( QgsWmsParameter::INFO_FORMAT );
    save( pInfoFormat );

    const QgsWmsParameter pI( QgsWmsParameter::I,
                              QVariant::Int,
                              QVariant( -1 ) );
    save( pI );

    const QgsWmsParameter pJ( QgsWmsParameter::J,
                              QVariant::Int,
                              QVariant( -1 ) );
    save( pJ );

    const QgsWmsParameter pX( QgsWmsParameter::X,
                              QVariant::Int,
                              QVariant( -1 ) );
    save( pX );

    const QgsWmsParameter pY( QgsWmsParameter::Y,
                              QVariant::Int,
                              QVariant( -1 ) );
    save( pY );

    const QgsWmsParameter pRule( QgsWmsParameter::RULE );
    save( pRule );

    const QgsWmsParameter pRuleLabel( QgsWmsParameter::RULELABEL,
                                      QVariant::Bool,
                                      QVariant( true ) );
    save( pRuleLabel );

    const QgsWmsParameter pShowRuleDetails( QgsWmsParameter::SHOWRULEDETAILS,
                                            QVariant::Bool,
                                            QVariant( false ) );
    save( pShowRuleDetails );

    const QgsWmsParameter pScale( QgsWmsParameter::SCALE,
                                  QVariant::Double,
                                  QVariant( -1 ) );
    save( pScale );

    const QgsWmsParameter pHeight( QgsWmsParameter::HEIGHT,
                                   QVariant::Int,
                                   QVariant( 0 ) );
    save( pHeight );

    const QgsWmsParameter pWidth( QgsWmsParameter::WIDTH,
                                  QVariant::Int,
                                  QVariant( 0 ) );
    save( pWidth );

    const QgsWmsParameter pSrcHeight( QgsWmsParameter::SRCHEIGHT,
                                      QVariant::Int,
                                      QVariant( 0 ) );
    save( pSrcHeight );

    const QgsWmsParameter pSrcWidth( QgsWmsParameter::SRCWIDTH,
                                     QVariant::Int,
                                     QVariant( 0 ) );
    save( pSrcWidth );

    const QgsWmsParameter pBbox( QgsWmsParameter::BBOX );
    save( pBbox );

    const QgsWmsParameter pSld( QgsWmsParameter::SLD );
    save( pSld );

    const QgsWmsParameter pSldBody( QgsWmsParameter::SLD_BODY );
    save( pSldBody );

    const QgsWmsParameter pLayer( QgsWmsParameter::LAYER );
    save( pLayer );

    const QgsWmsParameter pLayers( QgsWmsParameter::LAYERS );
    save( pLayers );

    const QgsWmsParameter pQueryLayers( QgsWmsParameter::QUERY_LAYERS );
    save( pQueryLayers );

    const QgsWmsParameter pFeatureCount( QgsWmsParameter::FEATURE_COUNT,
                                         QVariant::Int,
                                         QVariant( 1 ) );
    save( pFeatureCount );

    const QgsWmsParameter pLayerTitle( QgsWmsParameter::LAYERTITLE,
                                       QVariant::Bool,
                                       QVariant( true ) );
    save( pLayerTitle );

    const QgsWmsParameter pLayerFtFamily( QgsWmsParameter::LAYERFONTFAMILY );
    save( pLayerFtFamily );

    const QgsWmsParameter pLayerFtBold( QgsWmsParameter::LAYERFONTBOLD,
                                        QVariant::Bool,
                                        QVariant( false ) );
    save( pLayerFtBold );

    const QgsWmsParameter pLayerFtItalic( QgsWmsParameter::LAYERFONTITALIC,
                                          QVariant::Bool,
                                          QVariant( false ) );
    save( pLayerFtItalic );

    const QgsWmsParameter pLayerFtSize( QgsWmsParameter::LAYERFONTSIZE,
                                        QVariant::Double,
                                        QVariant( -1 ) );
    save( pLayerFtSize );

    const QgsWmsParameter pLayerFtColor( QgsWmsParameter::LAYERFONTCOLOR,
                                         QVariant::String,
                                         QVariant( "black" ) );
    save( pLayerFtColor );

    const QgsWmsParameter pStyle( QgsWmsParameter::STYLE );
    save( pStyle );

    const QgsWmsParameter pStyles( QgsWmsParameter::STYLES );
    save( pStyles );

    const QgsWmsParameter pOpacities( QgsWmsParameter::OPACITIES );
    save( pOpacities );

    const QgsWmsParameter pFilter( QgsWmsParameter::FILTER );
    save( pFilter );

    const QgsWmsParameter pFilterGeom( QgsWmsParameter::FILTER_GEOM );
    save( pFilterGeom );

    const QgsWmsParameter pPolygTol( QgsWmsParameter::FI_POLYGON_TOLERANCE,
                                     QVariant::Double,
                                     QVariant( 0.0 ) );
    save( pPolygTol );

    const QgsWmsParameter pLineTol( QgsWmsParameter::FI_LINE_TOLERANCE,
                                    QVariant::Double,
                                    QVariant( 0.0 ) );
    save( pLineTol );

    const QgsWmsParameter pPointTol( QgsWmsParameter::FI_POINT_TOLERANCE,
                                     QVariant::Double,
                                     QVariant( 0.0 ) );
    save( pPointTol );

    const QgsWmsParameter pSelection( QgsWmsParameter::SELECTION );
    save( pSelection );

    const QgsWmsParameter pWmsPrecision( QgsWmsParameter::WMS_PRECISION,
                                         QVariant::Int,
                                         QVariant( -1 ) );
    save( pWmsPrecision );

    const QgsWmsParameter pTransparent( QgsWmsParameter::TRANSPARENT,
                                        QVariant::Bool,
                                        QVariant( false ) );
    save( pTransparent );

    const QgsWmsParameter pBgColor( QgsWmsParameter::BGCOLOR,
                                    QVariant::String,
                                    QVariant( "white" ) );
    save( pBgColor );

    const QgsWmsParameter pDpi( QgsWmsParameter::DPI,
                                QVariant::Int,
                                QVariant( -1 ) );
    save( pDpi );

    const QgsWmsParameter pTemplate( QgsWmsParameter::TEMPLATE );
    save( pTemplate );

    const QgsWmsParameter pExtent( QgsWmsParameter::EXTENT );
    save( pExtent );

    const QgsWmsParameter pRotation( QgsWmsParameter::ROTATION,
                                     QVariant::Double,
                                     QVariant( 0.0 ) );
    save( pRotation );

    const QgsWmsParameter pGridX( QgsWmsParameter::GRID_INTERVAL_X,
                                  QVariant::Double,
                                  QVariant( 0.0 ) );
    save( pGridX );

    const QgsWmsParameter pGridY( QgsWmsParameter::GRID_INTERVAL_Y,
                                  QVariant::Double,
                                  QVariant( 0.0 ) );
    save( pGridY );

    const QgsWmsParameter pWithGeometry( QgsWmsParameter::WITH_GEOMETRY,
                                         QVariant::Bool,
                                         QVariant( false ) );
    save( pWithGeometry );

    const QgsWmsParameter pWithMapTip( QgsWmsParameter::WITH_MAPTIP,
                                       QVariant::String );
    save( pWithMapTip );

    const QgsWmsParameter pWithDisplayName( QgsWmsParameter::WITH_DISPLAY_NAME,
                                            QVariant::Bool,
                                            QVariant( false ) );
    save( pWithDisplayName );

    const QgsWmsParameter pWmtver( QgsWmsParameter::WMTVER );
    save( pWmtver );

    const QgsWmsParameter pAtlasPk( QgsWmsParameter::ATLAS_PK,
                                    QVariant::StringList );
    save( pAtlasPk );

    const QgsWmsParameter pFormatOpts( QgsWmsParameter::FORMAT_OPTIONS,
                                       QVariant::String );
    save( pFormatOpts );

    const QgsWmsParameter pAddLayerGroups( QgsWmsParameter::ADDLAYERGROUPS,
                                           QVariant::Bool,
                                           QVariant( false ) );
    save( pAddLayerGroups );
  }

  QgsWmsParameters::QgsWmsParameters( const QgsServerParameters &parameters )
    : QgsWmsParameters()
  {
    load( parameters.urlQuery() );

    auto it = mWmsParameters.constFind( QgsWmsParameter::SLD );
    if ( it != mWmsParameters.constEnd() && !it->toString().isEmpty() )
    {
      const QString sldBody = it->loadUrl();
      if ( !sldBody.isEmpty() )
      {
        loadParameter( QgsWmsParameter::name( QgsWmsParameter::SLD_BODY ), sldBody );
      }
    }
  }

  QgsWmsParameter QgsWmsParameters::operator[]( QgsWmsParameter::Name name ) const
  {
    return mWmsParameters.value( name );
  }

  void QgsWmsParameters::set( QgsWmsParameter::Name name, const QVariant &value )
  {
    auto it = mWmsParameters.find( name );
    if ( it == mWmsParameters.end() )
    {
      it = mWmsParameters.insert( name, QgsWmsParameter() );
    }

    it->mValue = value;
  }

  bool QgsWmsParameters::loadParameter( const QString &key, const QString &value )
  {
    bool loaded = false;

    const thread_local QRegularExpression composerParamRegExp( QStringLiteral( "^MAP\\d+:" ), QRegularExpression::CaseInsensitiveOption );
    if ( key.contains( composerParamRegExp ) )
    {
      const int mapId = QStringView {key} .mid( 3, key.indexOf( ':' ) - 3 ).toInt();
      const QString theKey = key.mid( key.indexOf( ':' ) + 1 );
      const QgsWmsParameter::Name name = QgsWmsParameter::name( theKey );

      if ( name >= 0 )
      {
        QgsWmsParameter param = mWmsParameters.value( name );
        param.mValue = value;
        param.mMapId = mapId;

        if ( ! param.isValid() )
        {
          param.raiseError();
        }

        save( param, true ); // multi MAP parameters for composer
        loaded = true;
      }
    }
    else
    {
      const QgsWmsParameter::Name name = QgsWmsParameter::name( key );
      if ( name >= 0 )
      {
        auto it = mWmsParameters.find( name );
        if ( it == mWmsParameters.end() )
          it = mWmsParameters.insert( name, QgsWmsParameter() );

        it->mValue = value;
        if ( !it->isValid() )
        {
          it->raiseError();
        }

        loaded = true;
      }
      else //maybe an external wms parameter?
      {
        int separator = key.indexOf( QLatin1Char( ':' ) );
        if ( separator >= 1 )
        {
          QString id = key.left( separator );
          QString param = key.right( key.length() - separator - 1 );
          mExternalWMSParameters[id].insert( param, value );

          loaded = true;
        }
      }
    }

    return loaded;
  }

  void QgsWmsParameters::dump() const
  {
    log( QStringLiteral( "WMS Request parameters:" ) );
    for ( auto it = mWmsParameters.constBegin(); it != mWmsParameters.constEnd(); ++it )
    {
      const QString value = it->toString();

      if ( ! value.isEmpty() )
      {
        QString name = QgsWmsParameter::name( it.key() );

        if ( it->mMapId >= 0 )
        {
          name = QStringLiteral( "%1:%2" ).arg( QString::number( it->mMapId ), name );
        }

        log( QStringLiteral( " - %1 : %2" ).arg( name, value ) );
      }
    }

    if ( !version().isEmpty() )
      log( QStringLiteral( " - VERSION : %1" ).arg( version() ) );
  }

  void QgsWmsParameters::save( const QgsWmsParameter &parameter, bool multi )
  {
    if ( multi )
    {
      mWmsParameters.insert( parameter.mName, parameter );
    }
    else
    {
      mWmsParameters.replace( parameter.mName, parameter );
    }
  }

  QStringList QgsWmsParameters::highlightGeom() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_GEOM ).toStringList( ';' );
  }

  QList<QgsGeometry> QgsWmsParameters::highlightGeomAsGeom() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_GEOM ).toGeomList( ';' );
  }

  QStringList QgsWmsParameters::highlightSymbol() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_SYMBOL ).toStringList( ';' );
  }

  QString QgsWmsParameters::crs() const
  {
    QString rs;
    const QString srs = mWmsParameters.value( QgsWmsParameter::SRS ).toString();
    const QString crs = mWmsParameters.value( QgsWmsParameter::CRS ).toString();

    // both SRS/CRS are supported but there's a priority according to the
    // specified version when both are defined in the request
    if ( !srs.isEmpty() && crs.isEmpty() )
      rs = srs;
    else if ( srs.isEmpty() && !crs.isEmpty() )
      rs = crs;
    else if ( !srs.isEmpty() && !crs.isEmpty() )
    {
      if ( versionAsNumber() >= QgsProjectVersion( 1, 3, 0 ) )
        rs = crs;
      else
        rs = srs;
    }

    return rs;
  }

  QString QgsWmsParameters::bbox() const
  {
    return mWmsParameters.value( QgsWmsParameter::BBOX ).toString();
  }

  QgsRectangle QgsWmsParameters::bboxAsRectangle() const
  {
    return mWmsParameters.value( QgsWmsParameter::BBOX ).toRectangle();
  }

  QString QgsWmsParameters::height() const
  {
    return mWmsParameters.value( QgsWmsParameter::HEIGHT ).toString();
  }

  QString QgsWmsParameters::width() const
  {
    return mWmsParameters.value( QgsWmsParameter::WIDTH ).toString();
  }

  int QgsWmsParameters::heightAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::HEIGHT ).toInt();
  }

  int QgsWmsParameters::widthAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::WIDTH ).toInt();
  }

  QString QgsWmsParameters::srcHeight() const
  {
    return mWmsParameters.value( QgsWmsParameter::SRCHEIGHT ).toString();
  }

  QString QgsWmsParameters::srcWidth() const
  {
    return mWmsParameters.value( QgsWmsParameter::SRCWIDTH ).toString();
  }

  int QgsWmsParameters::srcHeightAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::SRCHEIGHT ).toInt();
  }

  int QgsWmsParameters::srcWidthAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::SRCWIDTH ).toInt();
  }

  QString QgsWmsParameters::dpi() const
  {
    return mWmsParameters.value( QgsWmsParameter::DPI ).toString();
  }

  double QgsWmsParameters::dpiAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::DPI ).toDouble();
  }

  QString QgsWmsParameters::version() const
  {
    QString version = QgsServerParameters::version();

    if ( QgsServerParameters::request().compare( QLatin1String( "GetProjectSettings" ), Qt::CaseInsensitive ) == 0 )
    {
      version = QStringLiteral( "1.3.0" );
    }
    else if ( version.isEmpty() )
    {
      if ( ! wmtver().isEmpty() )
      {
        version = wmtver();
      }
      else
      {
        version = QStringLiteral( "1.3.0" );
      }
    }
    else if ( !mVersions.contains( QgsProjectVersion( version ) ) )
    {
      // WMS 1.3.0 specification: If a version lower than any of those
      // known to the server is requested, then the server shall send the
      // lowest version it supports.
      if ( QgsProjectVersion( 1, 1, 1 ) > QgsProjectVersion( version ) )
      {
        version = QStringLiteral( "1.1.1" );
      }
      else
      {
        version = QStringLiteral( "1.3.0" );
      }
    }

    return version;
  }

  QString QgsWmsParameters::request() const
  {
    QString req = QgsServerParameters::request();

    if ( version().compare( QLatin1String( "1.1.1" ) ) == 0
         && req.compare( QLatin1String( "capabilities" ), Qt::CaseInsensitive ) == 0 )
    {
      req = QStringLiteral( "GetCapabilities" );
    }

    return req;
  }

  QgsProjectVersion QgsWmsParameters::versionAsNumber() const
  {
    return QgsProjectVersion( version() );
  }

  bool QgsWmsParameters::versionIsValid( const QString version ) const
  {
    return mVersions.contains( QgsProjectVersion( version ) );
  }

  QString QgsWmsParameters::formatAsString() const
  {
    return mWmsParameters.value( QgsWmsParameter::FORMAT ).toString( true );
  }

  QString QgsWmsParameters::formatAsString( const QgsWmsParameters::Format format )
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsWmsParameters::Format>() );
    return metaEnum.valueToKey( format );
  }

  QgsWmsParameters::Format QgsWmsParameters::format() const
  {
    const QString fStr = formatAsString();

    Format f = Format::NONE;
    if ( fStr.compare( QLatin1String( "image/png" ), Qt::CaseInsensitive ) == 0 ||
         fStr.compare( QLatin1String( "png" ), Qt::CaseInsensitive ) == 0 )
    {
      f = Format::PNG;
    }
    else if ( fStr.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0
              || fStr.compare( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) == 0
              || fStr.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0 )
    {
      f = Format::JPG;
    }
    else if ( fStr.compare( QLatin1String( "image/svg" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "image/svg+xml" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
    {
      f = Format::SVG;
    }
    else if ( fStr.compare( QLatin1String( "application/pdf" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "pdf" ), Qt::CaseInsensitive ) == 0 )
    {
      f = Format::PDF;
    }
    else if ( fStr.compare( QLatin1String( "application/json" ), Qt::CaseInsensitive ) == 0 ||
              fStr.compare( QLatin1String( "json" ), Qt::CaseInsensitive ) == 0 )
    {
      f = Format::JSON;
    }
    return f;
  }

  QString QgsWmsParameters::infoFormatAsString() const
  {
    return mWmsParameters.value( QgsWmsParameter::INFO_FORMAT ).toString();
  }

  bool QgsWmsParameters::infoFormatIsImage() const
  {
    return infoFormat() == Format::PNG || infoFormat() == Format::JPG;
  }

  QgsWmsParameters::Format QgsWmsParameters::infoFormat() const
  {
    QString fStr = infoFormatAsString();

    Format f = Format::TEXT;
    if ( fStr.isEmpty() )
      return f;

    if ( fStr.startsWith( QLatin1String( "text/xml" ), Qt::CaseInsensitive ) )
      f = Format::XML;
    else if ( fStr.startsWith( QLatin1String( "text/html" ), Qt::CaseInsensitive ) )
      f = Format::HTML;
    else if ( fStr.startsWith( QLatin1String( "text/plain" ), Qt::CaseInsensitive ) )
      f = Format::TEXT;
    else if ( fStr.startsWith( QLatin1String( "application/vnd.ogc.gml" ), Qt::CaseInsensitive ) )
      f = Format::GML;
    else if ( fStr.startsWith( QLatin1String( "application/json" ), Qt::CaseInsensitive )
              || fStr.startsWith( QLatin1String( "application/geo+json" ), Qt::CaseInsensitive ) )
      f = Format::JSON;
    else
      f = Format::NONE;

    return f;
  }

  int QgsWmsParameters::infoFormatVersion() const
  {
    if ( infoFormat() != Format::GML )
      return -1;

    QString fStr = infoFormatAsString();
    if ( fStr.startsWith( QLatin1String( "application/vnd.ogc.gml/3" ), Qt::CaseInsensitive ) )
      return 3;
    else
      return 2;
  }

  QString QgsWmsParameters::i() const
  {
    return mWmsParameters.value( QgsWmsParameter::I ).toString();
  }

  QString QgsWmsParameters::j() const
  {
    return mWmsParameters.value( QgsWmsParameter::J ).toString();
  }

  int QgsWmsParameters::iAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::I ).toInt();
  }

  int QgsWmsParameters::jAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::J ).toInt();
  }

  QString QgsWmsParameters::x() const
  {
    return mWmsParameters.value( QgsWmsParameter::X ).toString();
  }

  QString QgsWmsParameters::y() const
  {
    return mWmsParameters.value( QgsWmsParameter::Y ).toString();
  }

  int QgsWmsParameters::xAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::X ).toInt();
  }

  int QgsWmsParameters::yAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::Y ).toInt();
  }

  QString QgsWmsParameters::rule() const
  {
    return mWmsParameters.value( QgsWmsParameter::RULE ).toString();
  }

  bool QgsWmsParameters::showRuleDetailsAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::SHOWRULEDETAILS ).toBool();
  }

  QString QgsWmsParameters::ruleLabel() const
  {
    return mWmsParameters.value( QgsWmsParameter::RULELABEL ).toString();
  }

  bool QgsWmsParameters::ruleLabelAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::RULELABEL ).toBool();
  }

  QString QgsWmsParameters::transparent() const
  {
    return mWmsParameters.value( QgsWmsParameter::TRANSPARENT ).toString();
  }

  bool QgsWmsParameters::transparentAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::TRANSPARENT ).toBool();
  }

  QString QgsWmsParameters::scale() const
  {
    return mWmsParameters.value( QgsWmsParameter::SCALE ).toString();
  }

  double QgsWmsParameters::scaleAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::SCALE ).toDouble();
  }

  QString QgsWmsParameters::imageQuality() const
  {
    return mWmsParameters.value( QgsWmsParameter::IMAGE_QUALITY ).toString();
  }

  int QgsWmsParameters::imageQualityAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::IMAGE_QUALITY ).toInt();
  }

  QString QgsWmsParameters::tiled() const
  {
    return mWmsParameters.value( QgsWmsParameter::TILED ).toString();
  }

  bool QgsWmsParameters::tiledAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::TILED ).toBool();
  }

  bool QgsWmsParameters::addLayerGroups() const
  {
    return mWmsParameters.value( QgsWmsParameter::ADDLAYERGROUPS ).toBool();
  }

  QString QgsWmsParameters::showFeatureCount() const
  {
    return mWmsParameters.value( QgsWmsParameter::SHOWFEATURECOUNT ).toString();
  }

  bool QgsWmsParameters::showFeatureCountAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::SHOWFEATURECOUNT ).toBool();
  }

  QString QgsWmsParameters::featureCount() const
  {
    return mWmsParameters.value( QgsWmsParameter::FEATURE_COUNT ).toString();
  }

  int QgsWmsParameters::featureCountAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::FEATURE_COUNT ).toInt();
  }

  QString QgsWmsParameters::boxSpace() const
  {
    return mWmsParameters.value( QgsWmsParameter::BOXSPACE ).toString();
  }

  double QgsWmsParameters::boxSpaceAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::BOXSPACE ).toDouble();
  }

  QString QgsWmsParameters::layerSpace() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERSPACE ).toString();
  }

  double QgsWmsParameters::layerSpaceAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERSPACE ).toDouble();
  }

  QString QgsWmsParameters::layerTitleSpace() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERTITLESPACE ).toString();
  }

  double QgsWmsParameters::layerTitleSpaceAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERTITLESPACE ).toDouble();
  }

  QString QgsWmsParameters::symbolSpace() const
  {
    return mWmsParameters.value( QgsWmsParameter::SYMBOLSPACE ).toString();
  }

  double QgsWmsParameters::symbolSpaceAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::SYMBOLSPACE ).toDouble();
  }

  QString QgsWmsParameters::symbolHeight() const
  {
    return mWmsParameters.value( QgsWmsParameter::SYMBOLHEIGHT ).toString();
  }

  double QgsWmsParameters::symbolHeightAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::SYMBOLHEIGHT ).toDouble();
  }

  QString QgsWmsParameters::symbolWidth() const
  {
    return mWmsParameters.value( QgsWmsParameter::SYMBOLWIDTH ).toString();
  }

  double QgsWmsParameters::symbolWidthAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::SYMBOLWIDTH ).toDouble();
  }

  QString QgsWmsParameters::iconLabelSpace() const
  {
    return mWmsParameters.value( QgsWmsParameter::ICONLABELSPACE ).toString();
  }

  double QgsWmsParameters::iconLabelSpaceAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::ICONLABELSPACE ).toDouble();
  }

  QString QgsWmsParameters::layerFontFamily() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTFAMILY ).toString();
  }

  QString QgsWmsParameters::itemFontFamily() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTFAMILY ).toString();
  }

  QString QgsWmsParameters::layerFontBold() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTBOLD ).toString();
  }

  bool QgsWmsParameters::layerFontBoldAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTBOLD ).toBool();
  }

  QString QgsWmsParameters::itemFontBold() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTBOLD ).toString();
  }

  QString QgsWmsParameters::polygonTolerance() const
  {
    return mWmsParameters.value( QgsWmsParameter::FI_POLYGON_TOLERANCE ).toString();
  }

  QString QgsWmsParameters::lineTolerance() const
  {
    return mWmsParameters.value( QgsWmsParameter::FI_LINE_TOLERANCE ).toString();
  }

  QString QgsWmsParameters::pointTolerance() const
  {
    return mWmsParameters.value( QgsWmsParameter::FI_POINT_TOLERANCE ).toString();
  }

  int QgsWmsParameters::polygonToleranceAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::FI_POLYGON_TOLERANCE ).toInt();
  }

  int QgsWmsParameters::lineToleranceAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::FI_LINE_TOLERANCE ).toInt();
  }

  int QgsWmsParameters::pointToleranceAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::FI_POINT_TOLERANCE ).toInt();
  }

  bool QgsWmsParameters::itemFontBoldAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTBOLD ).toBool();
  }

  QString QgsWmsParameters::layerFontItalic() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTITALIC ).toString();
  }

  bool QgsWmsParameters::layerFontItalicAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTITALIC ).toBool();
  }

  QString QgsWmsParameters::itemFontItalic() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTITALIC ).toString();
  }

  bool QgsWmsParameters::itemFontItalicAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTITALIC ).toBool();
  }

  QString QgsWmsParameters::layerFontSize() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTSIZE ).toString();
  }

  double QgsWmsParameters::layerFontSizeAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTSIZE ).toDouble();
  }

  QString QgsWmsParameters::layerFontColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTCOLOR ).toString();
  }

  QColor QgsWmsParameters::layerFontColorAsColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERFONTCOLOR ).toColor();
  }

  QString QgsWmsParameters::itemFontSize() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTSIZE ).toString();
  }

  double QgsWmsParameters::itemFontSizeAsDouble() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTSIZE ).toDouble();
  }

  QString QgsWmsParameters::itemFontColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTCOLOR ).toString();
  }

  QColor QgsWmsParameters::itemFontColorAsColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::ITEMFONTCOLOR ).toColor();
  }

  QFont QgsWmsParameters::layerFont() const
  {
    QFont font;
    font.fromString( "" );
    font.setBold( layerFontBoldAsBool() );
    font.setItalic( layerFontItalicAsBool() );

    if ( ! layerFontSize().isEmpty() )
      font.setPointSizeF( layerFontSizeAsDouble() );

    if ( !layerFontFamily().isEmpty() )
      QgsFontUtils::setFontFamily( font, layerFontFamily() );

    return font;
  }

  QFont QgsWmsParameters::itemFont() const
  {
    QFont font;
    font.fromString( "" );

    font.setBold( itemFontBoldAsBool() );
    font.setItalic( itemFontItalicAsBool() );

    if ( ! itemFontSize().isEmpty() )
      font.setPointSizeF( itemFontSizeAsDouble() );

    if ( !itemFontFamily().isEmpty() )
      QgsFontUtils::setFontFamily( font, itemFontFamily() );

    return font;
  }

  QString QgsWmsParameters::layerTitle() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERTITLE ).toString();
  }

  bool QgsWmsParameters::layerTitleAsBool() const
  {
    return mWmsParameters.value( QgsWmsParameter::LAYERTITLE ).toBool();
  }

  QgsLegendSettings QgsWmsParameters::legendSettings() const
  {
    QgsLegendSettings settings;
    settings.setTitle( QString() );
    settings.setBoxSpace( boxSpaceAsDouble() );
    settings.setSymbolSize( QSizeF( symbolWidthAsDouble(), symbolHeightAsDouble() ) );

    settings.rstyle( QgsLegendStyle::Style::Subgroup ).setMargin( QgsLegendStyle::Side::Top, layerSpaceAsDouble() );
    settings.rstyle( QgsLegendStyle::Style::Subgroup ).setMargin( QgsLegendStyle::Side::Bottom, layerTitleSpaceAsDouble() );

    // text format must be set before setting the format's colors
    settings.rstyle( QgsLegendStyle::Style::Subgroup ).setTextFormat( QgsTextFormat::fromQFont( layerFont() ) );
    settings.rstyle( QgsLegendStyle::Style::SymbolLabel ).setTextFormat( QgsTextFormat::fromQFont( itemFont() ) );

    if ( !itemFontColor().isEmpty() )
    {
      settings.rstyle( QgsLegendStyle::Title ).textFormat().setColor( itemFontColorAsColor() );
      settings.rstyle( QgsLegendStyle::Group ).textFormat().setColor( itemFontColorAsColor() );
      settings.rstyle( QgsLegendStyle::Subgroup ).textFormat().setColor( itemFontColorAsColor() );
      settings.rstyle( QgsLegendStyle::SymbolLabel ).textFormat().setColor( itemFontColorAsColor() );
    }

    // Ok, this is tricky: because QgsLegendSettings's layerFontColor was added to the API after
    // fontColor, to fix regressions #21871 and #21870 and the previous behavior was to use fontColor
    // for the whole legend we need to preserve that behavior.
    // But, the 2.18 server parameters ITEMFONTCOLOR did not have effect on the layer titles too, so
    // we set explicitly layerFontColor to black if it's not overridden by LAYERFONTCOLOR argument.
    settings.rstyle( QgsLegendStyle::Group ).textFormat().setColor( layerFontColor().isEmpty() ? QColor( Qt::black ) : layerFontColorAsColor() );
    settings.rstyle( QgsLegendStyle::Subgroup ).textFormat().setColor( layerFontColor().isEmpty() ? QColor( Qt::black ) : layerFontColorAsColor() );

    settings.rstyle( QgsLegendStyle::Style::Symbol ).setMargin( QgsLegendStyle::Side::Top, symbolSpaceAsDouble() );
    settings.rstyle( QgsLegendStyle::Style::SymbolLabel ).setMargin( QgsLegendStyle::Side::Left, iconLabelSpaceAsDouble() );

    return settings;
  }

  QString QgsWmsParameters::layoutParameter( const QString &id, bool &ok ) const
  {
    QString label;
    ok = false;

    if ( mUnmanagedParameters.contains( id.toUpper() ) )
    {
      label = mUnmanagedParameters[id.toUpper()];
      ok = true;
    }

    return label;
  }

  QStringList QgsWmsParameters::atlasPk() const
  {
    return mWmsParameters.value( QgsWmsParameter::ATLAS_PK ).toStringList();
  }

  QStringList QgsWmsParameters::highlightLabelString() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELSTRING ).toStringList( ';', false );
  }

  QStringList QgsWmsParameters::highlightLabelSize() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELSIZE ).toStringList( ';' );
  }

  QList<int> QgsWmsParameters::highlightLabelSizeAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELSIZE ).toIntList( ';' );
  }

  QStringList QgsWmsParameters::highlightLabelColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELCOLOR ).toStringList( ';' );
  }

  QList<QColor> QgsWmsParameters::highlightLabelColorAsColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELCOLOR ).toColorList( ';' );
  }

  QStringList QgsWmsParameters::highlightLabelWeight() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELWEIGHT ).toStringList( ';' );
  }

  QList<int> QgsWmsParameters::highlightLabelWeightAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELWEIGHT ).toIntList( ';' );
  }

  QStringList QgsWmsParameters::highlightLabelFont() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELFONT ).toStringList( ';' );
  }

  QStringList QgsWmsParameters::highlightLabelBufferColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELBUFFERCOLOR ).toStringList( ';' );
  }

  QList<QColor> QgsWmsParameters::highlightLabelBufferColorAsColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELBUFFERCOLOR ).toColorList( ';' );
  }

  QStringList QgsWmsParameters::highlightLabelBufferSize() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELBUFFERSIZE ).toStringList( ';' );
  }

  QList<double> QgsWmsParameters::highlightLabelBufferSizeAsFloat() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABELBUFFERSIZE ).toDoubleList( ';' );
  }

  QList<double> QgsWmsParameters::highlightLabelRotation() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABEL_ROTATION ).toDoubleList( ';' );
  }

  QList<double> QgsWmsParameters::highlightLabelDistance() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABEL_DISTANCE ).toDoubleList( ';' );
  }

  QStringList QgsWmsParameters::highlightLabelHorizontalAlignment() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT ).toStringList( ';' );
  }

  QStringList QgsWmsParameters::highlightLabelVerticalAlignment() const
  {
    return mWmsParameters.value( QgsWmsParameter::HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT ).toStringList( ';' );
  }

  QString QgsWmsParameters::wmsPrecision() const
  {
    return mWmsParameters.value( QgsWmsParameter::WMS_PRECISION ).toString();
  }

  int QgsWmsParameters::wmsPrecisionAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::WMS_PRECISION ).toInt();
  }

  QString QgsWmsParameters::sldBody() const
  {
    return mWmsParameters.value( QgsWmsParameter::SLD_BODY ).toString();
  }

  QStringList QgsWmsParameters::filters() const
  {
    QStringList filters = mWmsParameters.value( QgsWmsParameter::FILTER ).toOgcFilterList();
    if ( filters.isEmpty() )
      filters = mWmsParameters.value( QgsWmsParameter::FILTER ).toExpressionList();
    return filters;
  }

  QString QgsWmsParameters::filterGeom() const
  {
    return mWmsParameters.value( QgsWmsParameter::FILTER_GEOM ).toString();
  }

  QStringList QgsWmsParameters::selections() const
  {
    return mWmsParameters.value( QgsWmsParameter::SELECTION ).toStringList( ';' );
  }

  QStringList QgsWmsParameters::opacities() const
  {
    return mWmsParameters.value( QgsWmsParameter::OPACITIES ).toStringList();
  }

  QList<int> QgsWmsParameters::opacitiesAsInt() const
  {
    return mWmsParameters.value( QgsWmsParameter::OPACITIES ).toIntList();
  }

  QStringList QgsWmsParameters::allLayersNickname() const
  {
    // We don't want duplicates but order does matter, so no QSet
    QStringList result;

    // LAYER
    QList<QgsWmsParameter> cLayer { mWmsParameters.values( QgsWmsParameter::LAYER ) };
    // Sort by map id
    std::sort( cLayer.begin(), cLayer.end(), []( const QgsWmsParameter & a, const QgsWmsParameter & b ) -> bool { return a.mMapId < b.mMapId; } );
    for ( const QgsWmsParameter &param : std::as_const( cLayer ) )
    {
      const QStringList layersList { param.toStringList() };
      for ( const QString &layerName : std::as_const( layersList ) )
      {
        if ( ! result.contains( layerName ) )
          result.append( layerName );
      }
    }

    // LAYERS
    QList<QgsWmsParameter> cLayers { mWmsParameters.values( QgsWmsParameter::LAYERS ) };
    // Sort by map id
    std::sort( cLayers.begin(), cLayers.end(), []( const QgsWmsParameter & a, const QgsWmsParameter & b ) -> bool { return a.mMapId < b.mMapId; } );
    for ( const QgsWmsParameter &param : std::as_const( cLayers ) )
    {
      const QStringList layersList { param.toStringList() };
      for ( const QString &layerName : std::as_const( layersList ) )
      {
        if ( ! result.contains( layerName ) )
          result.append( layerName );
      }
    }
    return result;
  }

  QStringList QgsWmsParameters::queryLayersNickname() const
  {
    return mWmsParameters.value( QgsWmsParameter::QUERY_LAYERS ).toStringList();
  }

  QStringList QgsWmsParameters::allStyles() const
  {
    QStringList style = mWmsParameters.value( QgsWmsParameter::STYLE ).toStyleList();
    const QStringList styles = mWmsParameters.value( QgsWmsParameter::STYLES ).toStyleList();
    return style << styles;
  }

  QMultiMap<QString, QgsWmsParametersFilter> QgsWmsParameters::layerFilters( const QStringList &layers ) const
  {
    const QString nsWfs2 = QStringLiteral( "http://www.opengis.net/fes/2.0" );
    const QString prefixWfs2 = QStringLiteral( "<fes:" );

    const QStringList rawFilters = filters();
    QMultiMap<QString, QgsWmsParametersFilter> filters;
    for ( int i = 0; i < rawFilters.size(); i++ )
    {
      const QString f = rawFilters[i];
      if ( f.startsWith( QLatin1Char( '<' ) ) \
           && f.endsWith( QLatin1String( "Filter>" ) ) \
           &&  i < layers.size() )
      {
        QgsWmsParametersFilter filter;
        filter.mFilter = f;
        filter.mType = QgsWmsParametersFilter::OGC_FE;
        filter.mVersion = QgsOgcUtils::FILTER_OGC_1_0;

        if ( filter.mFilter.contains( nsWfs2 ) \
             || filter.mFilter.contains( prefixWfs2 ) )
        {
          filter.mVersion = QgsOgcUtils::FILTER_FES_2_0;
        }

        filters.insert( layers[i], filter );
      }
      else if ( !f.isEmpty() )
      {
        // filter format: "LayerName,LayerName2:filterString;LayerName3:filterString2;..."
        // several filters can be defined for one layer
        const int colonIndex = f.indexOf( ':' );
        if ( colonIndex != -1 )
        {
          const QString layers = f.section( ':', 0, 0 );
          const QString filter = f.section( ':', 1 );
          const QStringList layersList = layers.split( ',' );
          for ( const QString &layer : layersList )
          {
            QgsWmsParametersFilter parametersFilter;
            parametersFilter.mFilter = filter;
            parametersFilter.mType = QgsWmsParametersFilter::SQL;
            filters.insert( layer, parametersFilter );
          }
        }
        else
        {
          QString filterStr = mWmsParameters.value( QgsWmsParameter::FILTER ).toString();
          raiseError( QStringLiteral( "FILTER ('" ) + filterStr + QStringLiteral( "') is not properly formatted" ) );
        }
      }
    }
    return filters;
  }

  bool QgsWmsParameters::isForce2D() const
  {
    bool force2D = false;
    const QMap<DxfFormatOption, QString> options = formatOptions<QgsWmsParameters::DxfFormatOption>();

    if ( options.contains( DxfFormatOption::FORCE_2D ) )
    {
      force2D = QVariant( options[ DxfFormatOption::FORCE_2D ] ).toBool();
    }

    return force2D;
  }

  bool QgsWmsParameters::noMText() const
  {
    bool noMText = false;
    const QMap<DxfFormatOption, QString> options = formatOptions<QgsWmsParameters::DxfFormatOption>();

    if ( options.contains( DxfFormatOption::NO_MTEXT ) )
    {
      noMText = QVariant( options[ DxfFormatOption::NO_MTEXT ] ).toBool();
    }

    return noMText;
  }


  QList<QgsWmsParametersLayer> QgsWmsParameters::layersParameters() const
  {
    const QStringList layers = allLayersNickname();
    const QStringList styles = allStyles();
    const QStringList selection = selections();
    const QList<int> opacities = opacitiesAsInt();
    const QMultiMap<QString, QgsWmsParametersFilter> filters = layerFilters( layers );

    // selection format: "LayerName:id0,id1;LayerName2:id0,id1;..."
    // several filters can be defined for one layer
    QMultiMap<QString, QString> layerSelections;
    for ( const QString &s : selection )
    {
      const QStringList splits = s.split( ':' );
      if ( splits.size() == 2 )
      {
        layerSelections.insert( splits[0], splits[1] );
      }
      else
      {
        QString selStr = mWmsParameters.value( QgsWmsParameter::SELECTION ).toString();
        raiseError( QStringLiteral( "SELECTION ('" ) + selStr + QStringLiteral( "') is not properly formatted" ) );
      }
    }

    QList<QgsWmsParametersLayer> parameters;
    for ( int i = 0; i < layers.size(); i++ )
    {
      QString layer = layers[i];

      QgsWmsParametersLayer param;
      param.mNickname = layer;

      if ( i < opacities.count() )
        param.mOpacity = opacities[i];

      if ( isExternalLayer( layer ) )
      {
        const QgsWmsParametersExternalLayer extParam = externalLayerParameter( layer );
        param.mNickname = extParam.mName;
        param.mExternalUri = extParam.mUri;
      }
      else
      {
        if ( i < styles.count() )
          param.mStyle = styles[i];

        if ( filters.contains( layer ) )
        {
          auto it = filters.find( layer );
          while ( it != filters.end() && it.key() == layer )
          {
            param.mFilter.append( it.value() );
            ++it;
          }
        }

        if ( layerSelections.contains( layer ) )
        {
          QMultiMap<QString, QString>::const_iterator it;
          it = layerSelections.constFind( layer );
          while ( it != layerSelections.constEnd() && it.key() == layer )
          {
            param.mSelection << it.value().split( ',' );
            ++it;
          }
        }
      }

      parameters.append( param );
    }

    return parameters;
  }

  QList<QgsWmsParametersHighlightLayer> QgsWmsParameters::highlightLayersParameters() const
  {
    QList<QgsWmsParametersHighlightLayer> params;
    const QList<QgsGeometry> geoms = highlightGeomAsGeom();
    const QStringList slds = highlightSymbol();
    const QStringList labels = highlightLabelString();
    const QList<QColor> colors = highlightLabelColorAsColor();
    const QList<int> sizes = highlightLabelSizeAsInt();
    const QList<int> weights = highlightLabelWeightAsInt();
    const QStringList fonts = highlightLabelFont();
    const QList<QColor> bufferColors = highlightLabelBufferColorAsColor();
    const QList<double> bufferSizes = highlightLabelBufferSizeAsFloat();
    const QList<double> rotation = highlightLabelRotation();
    const QList<double> distance = highlightLabelDistance();
    const QStringList hali = highlightLabelHorizontalAlignment();
    const QStringList vali = highlightLabelVerticalAlignment();

    int nLayers = std::min( geoms.size(), slds.size() );
    for ( int i = 0; i < nLayers; i++ )
    {
      QgsWmsParametersHighlightLayer param;
      param.mName = QStringLiteral( "highlight_" ) + QString::number( i );
      param.mGeom = geoms[i];
      param.mSld = slds[i];

      if ( i < labels.count() )
        param.mLabel = labels[i];

      if ( i < colors.count() )
        param.mColor = colors[i];

      if ( i < sizes.count() )
        param.mSize = sizes[i];

      if ( i < weights.count() )
        param.mWeight = weights[i];

      if ( i < fonts.count() )
        param.mFont = fonts[ i ];

      if ( i < bufferColors.count() )
        param.mBufferColor = bufferColors[i];

      if ( i < bufferSizes.count() )
        param.mBufferSize = bufferSizes[i];

      if ( i < rotation.count() )
        param.mLabelRotation = rotation[i];

      if ( i < distance.count() )
        param.mLabelDistance = distance[i];

      if ( i < hali.count() )
        param.mHali = hali[i];

      if ( i < vali.count() )
        param.mVali = vali[i];



      params.append( param );
    }

    return params;
  }

  QList<QgsWmsParametersExternalLayer> QgsWmsParameters::externalLayersParameters() const
  {
    auto notExternalLayer = []( const QString & name ) { return ! QgsWmsParameters::isExternalLayer( name ); };

    QList<QgsWmsParametersExternalLayer> externalLayers;

    QStringList layers = allLayersNickname();
    QStringList::iterator rit = std::remove_if( layers.begin(), layers.end(), notExternalLayer );

    for ( QStringList::iterator it = layers.begin(); it != rit; ++it )
    {
      externalLayers << externalLayerParameter( *it );
    }

    return externalLayers;
  }

  QString QgsWmsParameters::backgroundColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::BGCOLOR ).toString();
  }

  QColor QgsWmsParameters::backgroundColorAsColor() const
  {
    return mWmsParameters.value( QgsWmsParameter::BGCOLOR ).toColor();
  }

  QString QgsWmsParameters::composerTemplate() const
  {
    return mWmsParameters.value( QgsWmsParameter::TEMPLATE ).toString();
  }

  QgsWmsParametersComposerMap QgsWmsParameters::composerMapParameters( const int mapId ) const
  {
    QgsWmsParameter wmsParam;
    QgsWmsParametersComposerMap param;
    param.mId = mapId;

    QString pMapId = QStringLiteral( "MAP" ) + QString::number( mapId );

    wmsParam = idParameter( QgsWmsParameter::EXTENT, mapId );
    QgsRectangle extent;
    if ( wmsParam.isValid() )
    {
      extent = wmsParam.toRectangle();
    }

    param.mHasExtent = !extent.isEmpty();
    param.mExtent = extent;

    // scale
    wmsParam = idParameter( QgsWmsParameter::SCALE, mapId );
    if ( wmsParam.isValid() && !wmsParam.toString().isEmpty() )
    {
      param.mScale = wmsParam.toDouble();
    }

    // rotation
    wmsParam = idParameter( QgsWmsParameter::ROTATION, mapId );
    if ( wmsParam.isValid() && !wmsParam.toString().isEmpty() )
    {
      param.mRotation = wmsParam.toDouble();
    }

    //grid space x / y
    double gridx( -1 ), gridy( -1 );

    wmsParam = idParameter( QgsWmsParameter::GRID_INTERVAL_X, mapId );
    if ( wmsParam.isValid() && !wmsParam.toString().isEmpty() )
    {
      gridx = wmsParam.toDouble();
    }

    wmsParam = idParameter( QgsWmsParameter::GRID_INTERVAL_Y, mapId );
    if ( wmsParam.isValid() && !wmsParam.toString().isEmpty() )
    {
      gridy = wmsParam.toDouble();
    }

    if ( gridx != -1 && gridy != -1 )
    {
      param.mGridX = gridx;
      param.mGridY = gridy;
    }

    //layers
    QStringList allLayers;
    wmsParam = idParameter( QgsWmsParameter::LAYERS, mapId );
    if ( wmsParam.isValid() )
    {
      allLayers = wmsParam.toStringList();
    }

    // external layers
    QStringList layers;

    for ( const auto &layer : std::as_const( allLayers ) )
    {
      if ( isExternalLayer( layer ) )
      {
        const QgsWmsParametersExternalLayer extParam = externalLayerParameter( layer );
        layers << extParam.mName;
      }
      else
      {
        layers << layer;
      }
    }

    QStringList styles;
    wmsParam = idParameter( QgsWmsParameter::STYLES, mapId );
    if ( wmsParam.isValid() )
    {
      styles = wmsParam.toStyleList();
    }

    QList<QgsWmsParametersLayer> lParams;
    for ( int i = 0; i < layers.size(); i++ )
    {
      QString layer = layers[i];
      QgsWmsParametersLayer lParam;
      lParam.mNickname = layer;

      if ( i < styles.count() )
        lParam.mStyle = styles[i];

      lParams.append( lParam );
    }
    param.mLayers = lParams;

    //highlight layers
    QList<QgsWmsParametersHighlightLayer> hParams;

    QList<QgsGeometry> geoms;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_GEOM, mapId );
    if ( wmsParam.isValid() )
    {
      geoms = wmsParam.toGeomList( ';' );
    }

    QStringList slds;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_SYMBOL, mapId );
    if ( wmsParam.isValid() )
    {
      slds = wmsParam.toStringList( ';' );
    }

    QStringList labels;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABELSTRING, mapId );
    if ( wmsParam.isValid() )
    {
      labels = wmsParam.toStringList( ';' );
    }

    QStringList fonts;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABELFONT, mapId );
    if ( wmsParam.isValid() )
    {
      fonts = wmsParam.toStringList( ';' );
    }

    QList<QColor> colors;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABELCOLOR, mapId );
    if ( wmsParam.isValid() )
    {
      colors = wmsParam.toColorList( ';' );
    }

    QList<int> sizes;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABELSIZE, mapId );
    if ( wmsParam.isValid() )
    {
      sizes = wmsParam.toIntList( ';' );
    }

    QList<int> weights;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABELWEIGHT, mapId );
    if ( wmsParam.isValid() )
    {
      weights = wmsParam.toIntList( ';' );
    }

    QList<QColor> bufferColors;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABELBUFFERCOLOR, mapId );
    if ( wmsParam.isValid() )
    {
      bufferColors = wmsParam.toColorList( ';' );
    }

    QList<double> bufferSizes;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABELBUFFERSIZE, mapId );
    if ( wmsParam.isValid() )
    {
      bufferSizes = wmsParam.toDoubleList( ';' );
    }

    QList<double> rotations;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABEL_ROTATION, mapId );
    if ( wmsParam.isValid() )
    {
      rotations = wmsParam.toDoubleList( ';' );
    }

    QList<double> distances;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABEL_DISTANCE, mapId );
    if ( wmsParam.isValid() )
    {
      distances = wmsParam.toDoubleList( ';' );
    }

    QStringList halis;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT, mapId );
    if ( wmsParam.isValid() )
    {
      halis = wmsParam.toStringList();
    }

    QStringList valis;
    wmsParam = idParameter( QgsWmsParameter::HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT, mapId );
    if ( wmsParam.isValid() )
    {
      valis = wmsParam.toStringList();
    }

    int nHLayers = std::min( geoms.size(), slds.size() );
    for ( int i = 0; i < nHLayers; i++ )
    {
      QgsWmsParametersHighlightLayer hParam;
      hParam.mName = pMapId + QStringLiteral( "_highlight_" ) + QString::number( i );
      hParam.mGeom = geoms[i];
      hParam.mSld = slds[i];

      if ( i < labels.count() )
        hParam.mLabel = labels[i];

      if ( i < colors.count() )
        hParam.mColor = colors[i];

      if ( i < sizes.count() )
        hParam.mSize = sizes[i];

      if ( i < weights.count() )
        hParam.mWeight = weights[i];

      if ( i < fonts.count() )
        hParam.mFont = fonts[ i ];

      if ( i < bufferColors.count() )
        hParam.mBufferColor = bufferColors[i];

      if ( i < bufferSizes.count() )
        hParam.mBufferSize = bufferSizes[i];

      if ( i < rotations.count() )
        hParam.mLabelRotation = rotations[i];

      if ( i < distances.count() )
        hParam.mLabelDistance = distances[i];

      if ( i < halis.count() )
        hParam.mHali = halis[i];

      if ( i < valis.count() )
        hParam.mVali = valis[i];

      hParams.append( hParam );
    }
    param.mHighlightLayers = hParams;

    return param;
  }

  QString QgsWmsParameters::externalWMSUri( const QString &layerId ) const
  {

    // Param names may be uppercased.
    QString id { layerId };

    for ( auto it = mExternalWMSParameters.cbegin(); it != mExternalWMSParameters.cend(); ++it )
    {
      if ( it.key().compare( id, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
      {
        id = it.key();
        break;
      }
    }

    if ( !mExternalWMSParameters.contains( id ) )
    {
      return QString();
    }

    QgsDataSourceUri wmsUri;
    const QMap<QString, QString> &paramMap = mExternalWMSParameters[ id ];
    QMap<QString, QString>::const_iterator paramIt = paramMap.constBegin();
    for ( ; paramIt != paramMap.constEnd(); ++paramIt )
    {
      QString paramName = paramIt.key().toLower();
      if ( paramName == QLatin1String( "layers" ) || paramName == QLatin1String( "styles" ) || paramName == QLatin1String( "opacities" ) )
      {
        const QStringList values = paramIt.value().split( ',' );
        for ( const QString &value : values )
          wmsUri.setParam( paramName, value );
      }
      else if ( paramName == QLatin1String( "ignorereportedlayerextents" ) )
      {
        wmsUri.setParam( QStringLiteral( "IgnoreReportedLayerExtents" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "smoothpixmaptransform" ) )
      {
        wmsUri.setParam( QStringLiteral( "SmoothPixmapTransform" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "ignoregetmapurl" ) )
      {
        wmsUri.setParam( QStringLiteral( "IgnoreGetMapUrl" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "ignoregetfeatureinfourl" ) )
      {
        wmsUri.setParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "ignoreaxisorientation" ) )
      {
        wmsUri.setParam( QStringLiteral( "IgnoreAxisOrientation" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "invertaxisorientation" ) )
      {
        wmsUri.setParam( QStringLiteral( "InvertAxisOrientation" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "dpimode" ) )
      {
        wmsUri.setParam( QStringLiteral( "dpiMode" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "stepwidth" ) )
      {
        wmsUri.setParam( QStringLiteral( "stepWidth" ), paramIt.value() );
      }
      else if ( paramName == QLatin1String( "stepheight" ) )
      {
        wmsUri.setParam( QStringLiteral( "stepHeight" ), paramIt.value() );
      }
      else
      {
        wmsUri.setParam( paramName, paramIt.value() );
      }
    }
    return wmsUri.encodedUri();
  }

  bool QgsWmsParameters::withGeometry() const
  {
    return mWmsParameters.value( QgsWmsParameter::WITH_GEOMETRY ).toBool();
  }

  QString QgsWmsParameters::withMapTipAsString() const
  {
    return mWmsParameters.value( QgsWmsParameter::WITH_MAPTIP ).toString();
  }

  bool QgsWmsParameters::withMapTip() const
  {
    const QString mStr = withMapTipAsString();

    if ( mStr.startsWith( QLatin1String( "true" ), Qt::CaseInsensitive ) ||
         mStr.startsWith( QLatin1String( "on" ), Qt::CaseInsensitive ) ||
         mStr.startsWith( QLatin1String( "yes" ), Qt::CaseInsensitive ) ||
         mStr.startsWith( QLatin1Char( '1' ) ) )
      return true;
    else
      return false;
  }

  bool QgsWmsParameters::htmlInfoOnlyMapTip() const
  {
    const QString mStr = withMapTipAsString();

    if ( mStr.startsWith( QLatin1String( "html_fi_only_maptip" ), Qt::CaseInsensitive ) )
      return true;
    else
      return false;
  }

  bool QgsWmsParameters::withDisplayName() const
  {
    return mWmsParameters.value( QgsWmsParameter::WITH_DISPLAY_NAME ).toBool();
  }

  QString QgsWmsParameters::wmtver() const
  {
    return mWmsParameters.value( QgsWmsParameter::WMTVER ).toString();
  }

  void QgsWmsParameters::log( const QString &msg ) const
  {
    QgsMessageLog::logMessage( msg, QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  }

  void QgsWmsParameters::raiseError( const QString &msg ) const
  {
    throw QgsBadRequestException( QgsServiceException::QGIS_InvalidParameterValue, msg );
  }

  QgsWmsParameter QgsWmsParameters::idParameter( const QgsWmsParameter::Name name, const int id ) const
  {
    QgsWmsParameter p;

    for ( const auto &param : mWmsParameters.values( name ) )
    {
      if ( param.mMapId == id )
      {
        p = param;
      }
    }

    return p;
  }

  QgsWmsParametersExternalLayer QgsWmsParameters::externalLayerParameter( const QString &name ) const
  {
    QgsWmsParametersExternalLayer param;

    param.mName = name;
    param.mName.remove( 0, EXTERNAL_LAYER_PREFIX.size() );
    param.mUri = externalWMSUri( param.mName );

    return param;
  }

  bool QgsWmsParameters::isExternalLayer( const QString &name )
  {
    return name.startsWith( EXTERNAL_LAYER_PREFIX );
  }

  QStringList QgsWmsParameters::dxfLayerAttributes() const
  {
    QStringList attributes;
    const QMap<DxfFormatOption, QString> options = formatOptions<QgsWmsParameters::DxfFormatOption>();

    if ( options.contains( DxfFormatOption::LAYERATTRIBUTES ) )
    {
      attributes = options[ DxfFormatOption::LAYERATTRIBUTES ].split( ',' );
    }

    return attributes;
  }

  bool QgsWmsParameters::dxfUseLayerTitleAsName() const
  {
    bool use = false;
    const QMap<DxfFormatOption, QString> options = formatOptions<QgsWmsParameters::DxfFormatOption>();

    if ( options.contains( DxfFormatOption::USE_TITLE_AS_LAYERNAME ) )
    {
      use = QVariant( options[ DxfFormatOption::USE_TITLE_AS_LAYERNAME ] ).toBool();
    }

    return use;
  }

  double QgsWmsParameters::dxfScale() const
  {
    const QMap<DxfFormatOption, QString> options = formatOptions<QgsWmsParameters::DxfFormatOption>();

    double scale = -1;
    if ( options.contains( DxfFormatOption::SCALE ) )
    {
      scale = options[ DxfFormatOption::SCALE ].toDouble();
    }

    return scale;
  }

  Qgis::FeatureSymbologyExport QgsWmsParameters::dxfMode() const
  {
    const QMap<DxfFormatOption, QString> options = formatOptions<QgsWmsParameters::DxfFormatOption>();

    Qgis::FeatureSymbologyExport symbol = Qgis::FeatureSymbologyExport::NoSymbology;

    if ( ! options.contains( DxfFormatOption::MODE ) )
    {
      return symbol;
    }

    const QString mode = options[ DxfFormatOption::MODE ];
    if ( mode.compare( QLatin1String( "SymbolLayerSymbology" ), Qt::CaseInsensitive ) == 0 )
    {
      symbol = Qgis::FeatureSymbologyExport::PerSymbolLayer;
    }
    else if ( mode.compare( QLatin1String( "FeatureSymbology" ), Qt::CaseInsensitive ) == 0 )
    {
      symbol = Qgis::FeatureSymbologyExport::PerFeature;
    }

    return symbol;
  }

  QString QgsWmsParameters::dxfCodec() const
  {
    QString codec = QStringLiteral( "ISO-8859-1" );

    if ( formatOptions<QgsWmsParameters::DxfFormatOption>().contains( DxfFormatOption::CODEC ) )
    {
      codec = formatOptions<QgsWmsParameters::DxfFormatOption>()[ DxfFormatOption::CODEC ];
    }

    return codec;
  }

  bool QgsWmsParameters::writeGeoPdf() const
  {
    bool geoPdf = false;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::WRITE_GEO_PDF ) )
    {
      geoPdf = QVariant( options[PdfFormatOption::WRITE_GEO_PDF] ).toBool();
    }
    return geoPdf;
  }

  bool QgsWmsParameters::pdfForceVectorOutput() const
  {
    bool forceVector = false;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::FORCE_VECTOR_OUTPUT ) )
    {
      forceVector = QVariant( options[PdfFormatOption::FORCE_VECTOR_OUTPUT] ).toBool();
    }
    return forceVector;
  }

  bool QgsWmsParameters::pdfAppendGeoreference() const
  {
    bool appendGeoref = true;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::APPEND_GEOREFERENCE ) )
    {
      appendGeoref = QVariant( options[PdfFormatOption::APPEND_GEOREFERENCE] ).toBool();
    }
    return appendGeoref;
  }

  bool QgsWmsParameters::pdfSimplifyGeometries() const
  {
    bool simplify = true;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::SIMPLIFY_GEOMETRY ) )
    {
      simplify = QVariant( options[PdfFormatOption::SIMPLIFY_GEOMETRY] ).toBool();
    }
    return simplify;
  }

  bool QgsWmsParameters::pdfExportMetadata() const
  {
    bool exportMetadata = false;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::EXPORT_METADATA ) )
    {
      exportMetadata = QVariant( options[PdfFormatOption::EXPORT_METADATA] ).toBool();
    }
    return exportMetadata;
  }

  Qgis::TextRenderFormat QgsWmsParameters::pdfTextRenderFormat() const
  {
    Qgis::TextRenderFormat format = Qgis::TextRenderFormat::AlwaysOutlines;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::TEXT_RENDER_FORMAT ) )
    {
      if ( options[PdfFormatOption::TEXT_RENDER_FORMAT].compare( QStringLiteral( "AlwaysText" ), Qt::CaseInsensitive ) == 0 )
      {
        format = Qgis::TextRenderFormat::AlwaysText;
      }
    }
    return format;
  }

  bool QgsWmsParameters::pdfLosslessImageCompression() const
  {
    bool losslessCompression = false;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::LOSSLESS_IMAGE_COMPRESSION ) )
    {
      losslessCompression = QVariant( options[PdfFormatOption::LOSSLESS_IMAGE_COMPRESSION] ).toBool();
    }
    return losslessCompression;
  }

  bool QgsWmsParameters::pdfDisableTiledRasterRendering() const
  {
    bool disableTiledRaster = false;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::DISABLE_TILED_RASTER_RENDERING ) )
    {
      disableTiledRaster = QVariant( options[PdfFormatOption::DISABLE_TILED_RASTER_RENDERING] ).toBool();
    }
    return disableTiledRaster;
  }

  bool QgsWmsParameters::pdfUseIso32000ExtensionFormatGeoreferencing() const
  {
    bool useIso32000 = true;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::USE_ISO_32000_EXTENSION_FORMAT_GEOREFERENCING ) )
    {
      useIso32000 = QVariant( options[PdfFormatOption::USE_ISO_32000_EXTENSION_FORMAT_GEOREFERENCING] ).toBool();
    }
    return useIso32000;
  }

  bool QgsWmsParameters::pdfUseOgcBestPracticeFormatGeoreferencing() const
  {
    bool useOgcGeoreferencing = false;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::USE_OGC_BEST_PRACTICE_FORMAT_GEOREFERENCING ) )
    {
      useOgcGeoreferencing = QVariant( options[PdfFormatOption::USE_OGC_BEST_PRACTICE_FORMAT_GEOREFERENCING] ).toBool();
    }
    return useOgcGeoreferencing;
  }

  QStringList QgsWmsParameters::pdfExportMapThemes() const
  {
    QStringList themes;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::EXPORT_THEMES ) )
    {
      themes = options[PdfFormatOption::EXPORT_THEMES].split( ',' );
    }
    return themes;
  }

  QVector<qreal> QgsWmsParameters::pdfPredefinedMapScales() const
  {
    QVector<qreal> scales;
    const QMap<QgsWmsParameters::PdfFormatOption, QString> options = formatOptions<QgsWmsParameters::PdfFormatOption>();
    if ( options.contains( PdfFormatOption::PREDEFINED_MAP_SCALES ) )
    {
      const QStringList scaleList = options[PdfFormatOption::PREDEFINED_MAP_SCALES].split( ',' );
      for ( const QString &it : std::as_const( scaleList ) )
      {
        bool ok = false;
        qreal scale = it.toDouble( &ok );
        if ( ok )
        {
          scales.append( scale );
        }
      }
    }
    return scales;
  }

  QMap<QString, QString> QgsWmsParameters::dimensionValues() const
  {
    QMap<QString, QString> dimValues;
    const QMetaEnum pnMetaEnum( QMetaEnum::fromType<QgsMapLayerServerProperties::PredefinedWmsDimensionName>() );
    const QStringList unmanagedNames = mUnmanagedParameters.keys();
    for ( const QString &key : unmanagedNames )
    {
      if ( key.startsWith( QLatin1String( "DIM_" ) ) )
      {
        dimValues[key.mid( 4 )] = mUnmanagedParameters[key];
      }
      else if ( pnMetaEnum.keyToValue( key.toUpper().toStdString().c_str() ) != -1 )
      {
        dimValues[key] = mUnmanagedParameters[key];
      }
    }
    return dimValues;
  }
}
