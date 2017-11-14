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
#include "qgsmessagelog.h"
#include <iostream>

namespace QgsWms
{
  QgsWmsParameters::QgsWmsParameters()
  {
    // Available version number
    mVersions.append( QgsProjectVersion( 1, 1, 1 ) );
    mVersions.append( QgsProjectVersion( 1, 3, 0 ) );

    // WMS parameters definition
    const Parameter pBoxSpace = { ParameterName::BOXSPACE,
                                  QVariant::Double,
                                  QVariant( 2.0 ),
                                  QVariant()
                                };
    save( pBoxSpace );

    const Parameter pSymbSpace = { ParameterName::SYMBOLSPACE,
                                   QVariant::Double,
                                   QVariant( 2.0 ),
                                   QVariant()
                                 };
    save( pSymbSpace );

    const Parameter pLayerSpace = { ParameterName::LAYERSPACE,
                                    QVariant::Double,
                                    QVariant( 3.0 ),
                                    QVariant()
                                  };
    save( pLayerSpace );

    const Parameter pTitleSpace = { ParameterName::LAYERTITLESPACE,
                                    QVariant::Double,
                                    QVariant( 3.0 ),
                                    QVariant()
                                  };
    save( pTitleSpace );

    const Parameter pSymbHeight = { ParameterName::SYMBOLHEIGHT,
                                    QVariant::Double,
                                    QVariant( 4.0 ),
                                    QVariant()
                                  };
    save( pSymbHeight );

    const Parameter pSymbWidth = { ParameterName::SYMBOLWIDTH,
                                   QVariant::Double,
                                   QVariant( 7.0 ),
                                   QVariant()
                                 };
    save( pSymbWidth );

    const Parameter pIcLabelSpace = { ParameterName::ICONLABELSPACE,
                                      QVariant::Double,
                                      QVariant( 2.0 ),
                                      QVariant()
                                    };
    save( pIcLabelSpace );

    const Parameter pItFontFamily = { ParameterName::ITEMFONTFAMILY,
                                      QVariant::String,
                                      QVariant( "" ),
                                      QVariant()
                                    };
    save( pItFontFamily );

    const Parameter pItFontBold = { ParameterName::ITEMFONTBOLD,
                                    QVariant::Bool,
                                    QVariant( false ),
                                    QVariant()
                                  };
    save( pItFontBold );

    const Parameter pItFontItalic = { ParameterName::ITEMFONTITALIC,
                                      QVariant::Bool,
                                      QVariant( false ),
                                      QVariant()
                                    };
    save( pItFontItalic );

    const Parameter pItFontSize = { ParameterName::ITEMFONTSIZE,
                                    QVariant::Double,
                                    QVariant( -1 ),
                                    QVariant()
                                  };
    save( pItFontSize );

    const Parameter pItFontColor = { ParameterName::ITEMFONTCOLOR,
                                     QVariant::String,
                                     QVariant( "black" ),
                                     QVariant()
                                   };
    save( pItFontColor );

    const Parameter pHighlightGeom = { ParameterName::HIGHLIGHT_GEOM,
                                       QVariant::String,
                                       QVariant( "" ),
                                       QVariant()
                                     };
    save( pHighlightGeom );

    const Parameter pShowFeatureCount = { ParameterName::SHOWFEATURECOUNT,
                                          QVariant::Bool,
                                          QVariant( false ),
                                          QVariant()
                                        };
    save( pShowFeatureCount );

    const Parameter pHighlightSymbol = { ParameterName::HIGHLIGHT_SYMBOL,
                                         QVariant::String,
                                         QVariant( "" ),
                                         QVariant()
                                       };
    save( pHighlightSymbol );

    const Parameter pHighlightLabel = { ParameterName::HIGHLIGHT_LABELSTRING,
                                        QVariant::String,
                                        QVariant( "" ),
                                        QVariant()
                                      };
    save( pHighlightLabel );

    const Parameter pHighlightColor = { ParameterName::HIGHLIGHT_LABELCOLOR,
                                        QVariant::String,
                                        QVariant( "black" ),
                                        QVariant()
                                      };
    save( pHighlightColor );

    const Parameter pHighlightFontSize = { ParameterName::HIGHLIGHT_LABELSIZE,
                                           QVariant::String,
                                           QVariant( "" ),
                                           QVariant()
                                         };
    save( pHighlightFontSize );

    const Parameter pHighlightFontWeight = { ParameterName::HIGHLIGHT_LABELWEIGHT,
                                             QVariant::String,
                                             QVariant( "" ),
                                             QVariant()
                                           };
    save( pHighlightFontWeight );

    const Parameter pHighlightFont = { ParameterName::HIGHLIGHT_LABELFONT,
                                       QVariant::String,
                                       QVariant( "" ),
                                       QVariant()
                                     };
    save( pHighlightFont );

    const Parameter pHighlightBufferColor = { ParameterName::HIGHLIGHT_LABELBUFFERCOLOR,
                                              QVariant::String,
                                              QVariant( "black" ),
                                              QVariant()
                                            };
    save( pHighlightBufferColor );

    const Parameter pHighlightBufferSize = { ParameterName::HIGHLIGHT_LABELBUFFERSIZE,
                                             QVariant::String,
                                             QVariant( "" ),
                                             QVariant()
                                           };
    save( pHighlightBufferSize );

    const Parameter pCRS = { ParameterName::CRS,
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
    save( pCRS );

    const Parameter pSRS = { ParameterName::SRS,
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
    save( pSRS );

    const Parameter pFormat = { ParameterName::FORMAT,
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
    save( pFormat );

    const Parameter pInfoFormat = { ParameterName::INFO_FORMAT,
                                    QVariant::String,
                                    QVariant( "" ),
                                    QVariant()
                                  };
    save( pInfoFormat );

    const Parameter pI = { ParameterName::I,
                           QVariant::Int,
                           QVariant( -1 ),
                           QVariant()
                         };
    save( pI );

    const Parameter pJ = { ParameterName::J,
                           QVariant::Int,
                           QVariant( -1 ),
                           QVariant()
                         };
    save( pJ );

    const Parameter pX = { ParameterName::X,
                           QVariant::Int,
                           QVariant( -1 ),
                           QVariant()
                         };
    save( pX );

    const Parameter pY = { ParameterName::Y,
                           QVariant::Int,
                           QVariant( -1 ),
                           QVariant()
                         };
    save( pY );

    const Parameter pRule = { ParameterName::RULE,
                              QVariant::String,
                              QVariant( "" ),
                              QVariant()
                            };
    save( pRule );

    const Parameter pRuleLabel = { ParameterName::RULELABEL,
                                   QVariant::Bool,
                                   QVariant( true ),
                                   QVariant()
                                 };
    save( pRuleLabel );

    const Parameter pScale = { ParameterName::SCALE,
                               QVariant::Double,
                               QVariant( -1 ),
                               QVariant()
                             };
    save( pScale );

    const Parameter pHeight = { ParameterName::HEIGHT,
                                QVariant::Int,
                                QVariant( 0 ),
                                QVariant()
                              };
    save( pHeight );

    const Parameter pWidth = { ParameterName::WIDTH,
                               QVariant::Int,
                               QVariant( 0 ),
                               QVariant()
                             };
    save( pWidth );

    const Parameter pBbox = { ParameterName::BBOX,
                              QVariant::String,
                              QVariant( "" ),
                              QVariant()
                            };
    save( pBbox );

    const Parameter pSld = { ParameterName::SLD,
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
    save( pSld );

    const Parameter pLayer = { ParameterName::LAYER,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant()
                             };
    save( pLayer );

    const Parameter pLayers = { ParameterName::LAYERS,
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
    save( pLayers );

    const Parameter pQueryLayers = { ParameterName::QUERY_LAYERS,
                                     QVariant::String,
                                     QVariant( "" ),
                                     QVariant()
                                   };
    save( pQueryLayers );

    const Parameter pFeatureCount = { ParameterName::FEATURE_COUNT,
                                      QVariant::Int,
                                      QVariant( 1 ),
                                      QVariant()
                                    };
    save( pFeatureCount );

    const Parameter pLayerTitle = { ParameterName::LAYERTITLE,
                                    QVariant::Bool,
                                    QVariant( true ),
                                    QVariant()
                                  };
    save( pLayerTitle );

    const Parameter pLayerFtFamily = { ParameterName::LAYERFONTFAMILY,
                                       QVariant::String,
                                       QVariant( "" ),
                                       QVariant()
                                     };
    save( pLayerFtFamily );

    const Parameter pLayerFtBold = { ParameterName::LAYERFONTBOLD,
                                     QVariant::Bool,
                                     QVariant( false ),
                                     QVariant()
                                   };
    save( pLayerFtBold );

    const Parameter pLayerFtItalic = { ParameterName::LAYERFONTITALIC,
                                       QVariant::Bool,
                                       QVariant( false ),
                                       QVariant()
                                     };
    save( pLayerFtItalic );

    const Parameter pLayerFtSize = { ParameterName::LAYERFONTSIZE,
                                     QVariant::Double,
                                     QVariant( -1 ),
                                     QVariant()
                                   };
    save( pLayerFtSize );

    const Parameter pLayerFtColor = { ParameterName::LAYERFONTCOLOR,
                                      QVariant::String,
                                      QVariant( "black" ),
                                      QVariant()
                                    };
    save( pLayerFtColor );

    const Parameter pStyle = { ParameterName::STYLE,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant()
                             };
    save( pStyle );

    const Parameter pStyles = { ParameterName::STYLES,
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
    save( pStyles );

    const Parameter pOpacities = { ParameterName::OPACITIES,
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
    save( pOpacities );

    const Parameter pFilter = { ParameterName::FILTER,
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
    save( pFilter );

    const Parameter pFilterGeom = { ParameterName::FILTER_GEOM,
                                    QVariant::String,
                                    QVariant( "" ),
                                    QVariant()
                                  };
    save( pFilterGeom );

    const Parameter pSelection = { ParameterName::SELECTION,
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
    save( pSelection );

    const Parameter pWmsPrecision = { ParameterName::WMS_PRECISION,
                                      QVariant::Int,
                                      QVariant( -1 ),
                                      QVariant()
                                    };
    save( pWmsPrecision );

    const Parameter pTransparent = { ParameterName::TRANSPARENT,
                                     QVariant::Bool,
                                     QVariant( false ),
                                     QVariant()
                                   };
    save( pTransparent );

    const Parameter pBgColor = { ParameterName::BGCOLOR,
                                 QVariant::String,
                                 QVariant( "white" ),
                                 QVariant()
                               };
    save( pBgColor );

    const Parameter pDpi = { ParameterName::DPI,
                             QVariant::Int,
                             QVariant( -1 ),
                             QVariant()
                           };
    save( pDpi );

    const Parameter pTemplate = { ParameterName::TEMPLATE,
                                  QVariant::String,
                                  QVariant(),
                                  QVariant()
                                };
    save( pTemplate );

    const Parameter pExtent = { ParameterName::EXTENT,
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
    save( pExtent );

    const Parameter pRotation = { ParameterName::ROTATION,
                                  QVariant::Double,
                                  QVariant( 0.0 ),
                                  QVariant()
                                };
    save( pRotation );

    const Parameter pGridX = { ParameterName::GRID_INTERVAL_X,
                               QVariant::Double,
                               QVariant( 0.0 ),
                               QVariant()
                             };
    save( pGridX );

    const Parameter pGridY = { ParameterName::GRID_INTERVAL_Y,
                               QVariant::Double,
                               QVariant( 0.0 ),
                               QVariant()
                             };
    save( pGridY );

    const Parameter pWithGeometry = { ParameterName::WITH_GEOMETRY,
                                      QVariant::Bool,
                                      QVariant( false ),
                                      QVariant()
                                    };
    save( pWithGeometry );

    const Parameter pWithMapTip = { ParameterName::WITH_MAPTIP,
                                    QVariant::Bool,
                                    QVariant( false ),
                                    QVariant()
                                  };
    save( pWithMapTip );
  }

  QgsWmsParameters::QgsWmsParameters( const QgsServerRequest::Parameters &parameters )
  {
    load( parameters );
  }

  void QgsWmsParameters::load( const QgsServerRequest::Parameters &parameters )
  {
    mRequestParameters = parameters;

    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
    static QRegExp composerParamRegExp( "^MAP\\d+:" );

    foreach ( QString key, parameters.keys() )
    {
      if ( key.contains( composerParamRegExp ) )
      {
        const int mapId = key.mid( 3, key.indexOf( ':' ) - 3 ).toInt();
        const QString theKey = key.mid( key.indexOf( ':' ) + 1 );
        const ParameterName name = ( ParameterName ) metaEnum.keyToValue( theKey.toStdString().c_str() );
        if ( name >= 0 )
        {
          QVariant value( parameters[key] );
          Parameter param = mParameters[name];
          Parameter nParam =
          {
            param.mName,
            param.mType,
            param.mDefaultValue,
            value
          };
          save( nParam, mapId );
          if ( !value.canConvert( nParam.mType ) )
          {
            raiseError( name, mapId );
          }
        }
      }
      else
      {
        const ParameterName name = ( ParameterName ) metaEnum.keyToValue( key.toStdString().c_str() );
        if ( name >= 0 )
        {
          QVariant value( parameters[key] );
          mParameters[name].mValue = value;
          if ( !value.canConvert( mParameters[name].mType ) )
          {
            raiseError( name );
          }
        }
        else //maybe an external wms parameter?
        {
          int separator = key.indexOf( ":" );
          if ( separator >= 1 )
          {
            QString id = key.left( separator );
            QString param = key.right( key.length() - separator - 1 );
            mExternalWMSParameters[id].insert( param, parameters[key] );
          }
        }
      }
    }
  }

  void QgsWmsParameters::dump() const
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );

    log( "WMS Request parameters:" );
    for ( auto parameter : mParameters.toStdMap() )
    {
      const QString value = parameter.second.mValue.toString();

      if ( ! value.isEmpty() )
      {
        const QString name = metaEnum.valueToKey( parameter.first );
        log( " - " + name + " : " + value );
      }
    }
    for ( auto map : mComposerParameters.toStdMap() )
    {
      const int mapId = map.first;
      log( " - MAP" + QString::number( mapId ) );
      for ( auto param : mComposerParameters[map.first].toStdMap() )
      {
        const QString value = param.second.mValue.toString();

        if ( ! value.isEmpty() )
        {
          const QString name = metaEnum.valueToKey( param.first );
          log( " - MAP" + QString::number( mapId ) + ":" + name + " : " + value );
        }
      }
    }

    if ( !version().isEmpty() )
      log( " - VERSION : " + version() );
  }

  void QgsWmsParameters::save( const Parameter &parameter )
  {
    mParameters[ parameter.mName ] = parameter;
  }

  QVariant QgsWmsParameters::value( ParameterName name ) const
  {
    return mParameters[name].mValue;
  }

  QVariant QgsWmsParameters::defaultValue( ParameterName name ) const
  {
    return mParameters[name].mDefaultValue;
  }

  void QgsWmsParameters::save( const Parameter &parameter, int mapId )
  {
    mComposerParameters[ mapId ][ parameter.mName ] = parameter;
  }

  QVariant QgsWmsParameters::value( ParameterName name, int mapId ) const
  {
    if ( mComposerParameters.contains( mapId ) && mComposerParameters[ mapId ].contains( name ) )
      return mComposerParameters[ mapId ][ name ].mValue;
    else
      return value( name );
  }

  QVariant QgsWmsParameters::defaultValue( ParameterName name, int mapId ) const
  {
    if ( mComposerParameters.contains( mapId ) && mComposerParameters[ mapId ].contains( name ) )
      return mComposerParameters[ mapId ][ name ].mDefaultValue;
    else
      return defaultValue( name );
  }

  QStringList QgsWmsParameters::highlightGeom() const
  {
    return toStringList( ParameterName::HIGHLIGHT_GEOM, ';' );
  }

  QList<QgsGeometry> QgsWmsParameters::highlightGeomAsGeom() const
  {
    return toGeomList( highlightGeom(), ParameterName::HIGHLIGHT_GEOM );
  }

  QStringList QgsWmsParameters::highlightSymbol() const
  {
    return toStringList( ParameterName::HIGHLIGHT_SYMBOL, ';' );
  }

  QString QgsWmsParameters::crs() const
  {
    QString rs;
    QString srs = value( ParameterName::SRS ).toString();
    QString crs = value( ParameterName::CRS ).toString();

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
    return value( ParameterName::BBOX ).toString();
  }

  QgsRectangle QgsWmsParameters::bboxAsRectangle() const
  {
    return toRectangle( ParameterName::BBOX );
  }

  QString QgsWmsParameters::height() const
  {
    return value( ParameterName::HEIGHT ).toString();
  }

  QString QgsWmsParameters::width() const
  {
    return value( ParameterName::WIDTH ).toString();
  }

  int QgsWmsParameters::heightAsInt() const
  {
    return toInt( ParameterName::HEIGHT );
  }

  int QgsWmsParameters::widthAsInt() const
  {
    return toInt( ParameterName::WIDTH );
  }

  QString QgsWmsParameters::dpi() const
  {
    return value( ParameterName::DPI ).toString();
  }

  int QgsWmsParameters::dpiAsInt() const
  {
    return toInt( ParameterName::DPI );
  }

  QString QgsWmsParameters::version() const
  {
    // VERSION parameter is not managed with other parameters because
    // there's a conflict with qgis VERSION defined in qgsconfig.h
    if ( mRequestParameters.contains( "VERSION" ) )
      return mRequestParameters["VERSION"];
    else
      return QString();
  }

  QgsProjectVersion QgsWmsParameters::versionAsNumber() const
  {
    QString vStr = version();
    QgsProjectVersion version;

    if ( vStr.isEmpty() )
      version = QgsProjectVersion( 1, 3, 0 ); // default value
    else if ( mVersions.contains( QgsProjectVersion( vStr ) ) )
      version = QgsProjectVersion( vStr );

    return version;
  }

  double QgsWmsParameters::toDouble( const QVariant &value, const QVariant &defaultValue, bool *error ) const
  {
    double val = defaultValue.toDouble();
    QString valStr = value.toString();
    bool ok = true;

    if ( !valStr.isEmpty() )
    {
      val = value.toDouble( &ok );
    }
    *error = !ok;

    return val;
  }

  double QgsWmsParameters::toDouble( ParameterName p ) const
  {
    bool error;
    double val = toDouble( value( p ), defaultValue( p ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into a double";
      raiseError( msg );
    }

    return val;
  }

  double QgsWmsParameters::toDouble( ParameterName p, int mapId ) const
  {
    bool error;
    double val = toDouble( value( p, mapId ), defaultValue( p, mapId ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p, mapId ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into a double";
      raiseError( msg );
    }

    return val;
  }

  bool QgsWmsParameters::toBool( const QVariant &value, const QVariant &defaultValue ) const
  {
    bool val = defaultValue.toBool();
    QString valStr = value.toString();

    if ( ! valStr.isEmpty() )
      val = value.toBool();

    return val;
  }

  bool QgsWmsParameters::toBool( ParameterName p ) const
  {
    return toBool( value( p ), defaultValue( p ) );
  }

  bool QgsWmsParameters::toBool( ParameterName p, int mapId ) const
  {
    return toBool( value( p, mapId ), defaultValue( p, mapId ) );
  }

  int QgsWmsParameters::toInt( const QVariant &value, const QVariant &defaultValue, bool *error ) const
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

  int QgsWmsParameters::toInt( ParameterName p ) const
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

  int QgsWmsParameters::toInt( ParameterName p, int mapId ) const
  {
    bool error;
    int val = toInt( value( p, mapId ), defaultValue( p, mapId ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p, mapId ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into int";
      raiseError( msg );
    }

    return val;
  }

  QColor QgsWmsParameters::toColor( const QVariant &value, const QVariant &defaultValue, bool *error ) const
  {
    *error = false;
    QColor c = defaultValue.value<QColor>();
    QString cStr = value.toString();

    if ( !cStr.isEmpty() )
    {
      // support hexadecimal notation to define colors
      if ( cStr.startsWith( "0x", Qt::CaseInsensitive ) )
        cStr.replace( 0, 2, "#" );

      c = QColor( cStr );

      *error = !c.isValid();
    }

    return c;
  }

  QColor QgsWmsParameters::toColor( ParameterName p ) const
  {
    bool error;
    QColor c = toColor( value( p ), defaultValue( p ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into a color";
      raiseError( msg );
    }

    return c;
  }

  QColor QgsWmsParameters::toColor( ParameterName p, int mapId ) const
  {
    bool error;
    QColor c = toColor( value( p, mapId ), defaultValue( p, mapId ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p, mapId ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into a color";
      raiseError( msg );
    }

    return c;
  }

  QgsRectangle QgsWmsParameters::toRectangle( const QVariant &value, bool *error ) const
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

        if ( d[0] > d[2] || d[1] > d[3] )
        {
          *error = true;
          return extent;
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

  QgsRectangle QgsWmsParameters::toRectangle( ParameterName p ) const
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

  QgsRectangle QgsWmsParameters::toRectangle( ParameterName p, int mapId ) const
  {
    bool error;
    QgsRectangle extent = toRectangle( value( p, mapId ), &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p, mapId ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into a rectangle";
      raiseError( msg );
    }

    return extent;
  }

  QStringList QgsWmsParameters::toStringList( ParameterName name, char delimiter ) const
  {
    return value( name ).toString().split( delimiter, QString::SkipEmptyParts );
  }

  QStringList QgsWmsParameters::toStringList( ParameterName name, int mapId, char delimiter ) const
  {
    return value( name, mapId ).toString().split( delimiter, QString::SkipEmptyParts );
  }

  QList<int> QgsWmsParameters::toIntList( const QStringList &l, bool *error ) const
  {
    *error = false;
    QList<int> elements;

    for ( const QString &element : l )
    {
      bool ok;
      int e = element.toInt( &ok );

      if ( ok )
      {
        elements.append( e );
      }
      else
      {
        *error = !ok;
        return elements;
      }
    }

    return elements;
  }

  QList<int> QgsWmsParameters::toIntList( const QStringList &l, ParameterName p ) const
  {
    bool error;
    QList<int> elements = toIntList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into a list of int";
      raiseError( msg );
    }

    return elements;
  }

  QList<int> QgsWmsParameters::toIntList( const QStringList &l, ParameterName p, int mapId ) const
  {
    bool error;
    QList<int> elements = toIntList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p, mapId ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into a list of int";
      raiseError( msg );
    }

    return elements;
  }

  QList<float> QgsWmsParameters::toFloatList( const QStringList &l, bool *error ) const
  {
    *error = false;
    QList<float> elements;

    for ( const QString &element : l )
    {
      bool ok;
      float e = element.toFloat( &ok );

      if ( ok )
      {
        elements.append( e );
      }
      else
      {
        *error = !ok;
        return elements;
      }
    }

    return elements;
  }

  QList<float> QgsWmsParameters::toFloatList( const QStringList &l, ParameterName p ) const
  {
    bool error;
    QList<float> elements = toFloatList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into a list of float";
      raiseError( msg );
    }

    return elements;
  }

  QList<float> QgsWmsParameters::toFloatList( const QStringList &l, ParameterName p, int mapId ) const
  {
    bool error;
    QList<float> elements = toFloatList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into a list of float";
      raiseError( msg );
    }

    return elements;
  }

  QList<QColor> QgsWmsParameters::toColorList( const QStringList &l, bool *error ) const
  {
    *error = false;
    QList<QColor> elements;

    for ( const QString &element : l )
    {
      QColor c = QColor( element );

      if ( c.isValid() )
      {
        elements.append( c );
      }
      else
      {
        *error = !c.isValid();
        return elements;
      }
    }

    return elements;
  }

  QList<QColor> QgsWmsParameters::toColorList( const QStringList &l, ParameterName p ) const
  {
    bool error;
    QList<QColor> elements = toColorList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into a list of colors";
      raiseError( msg );
    }

    return elements;
  }

  QList<QColor> QgsWmsParameters::toColorList( const QStringList &l, ParameterName p, int mapId ) const
  {
    bool error;
    QList<QColor> elements = toColorList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into a list of colors";
      raiseError( msg );
    }

    return elements;
  }

  QList<QgsGeometry> QgsWmsParameters::toGeomList( const QStringList &l, bool *error ) const
  {
    *error = false;
    QList<QgsGeometry> geometries;

    for ( const QString &wkt : l )
    {
      QgsGeometry g( QgsGeometry::fromWkt( wkt ) );

      if ( g.isGeosValid() )
      {
        geometries.append( g );
      }
      else
      {
        *error = true;
        return geometries;
      }
    }

    return geometries;
  }

  QList<QgsGeometry> QgsWmsParameters::toGeomList( const QStringList &l, ParameterName p ) const
  {
    bool error;
    QList<QgsGeometry> elements = toGeomList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p ).toString();
      QString msg = n + " ('" + valStr + "') cannot be converted into a list of geometries";
      raiseError( msg );
    }

    return elements;
  }

  QList<QgsGeometry> QgsWmsParameters::toGeomList( const QStringList &l, ParameterName p, int mapId ) const
  {
    bool error;
    QList<QgsGeometry> elements = toGeomList( l, &error );
    if ( error )
    {
      QString n = name( p );
      QString valStr = value( p, mapId ).toString();
      QString msg = "MAP" + QString::number( mapId ) + ":" + n + " ('" + valStr + "') cannot be converted into a list of geometries";
      raiseError( msg );
    }

    return elements;
  }

  QString QgsWmsParameters::formatAsString() const
  {
    return value( ParameterName::FORMAT ).toString();
  }

  QgsWmsParameters::Format QgsWmsParameters::format() const
  {
    QString fStr = formatAsString();

    if ( fStr.isEmpty() )
      return Format::NONE;

    Format f = Format::PNG;
    if ( fStr.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0
         || fStr.compare( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) == 0
         || fStr.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0 )
      f = Format::JPG;

    return f;
  }

  QString QgsWmsParameters::infoFormatAsString() const
  {
    return value( ParameterName::INFO_FORMAT ).toString();
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
    return value( ParameterName::I ).toString();
  }

  QString QgsWmsParameters::j() const
  {
    return value( ParameterName::J ).toString();
  }

  int QgsWmsParameters::iAsInt() const
  {
    return toInt( ParameterName::I );
  }

  int QgsWmsParameters::jAsInt() const
  {
    return toInt( ParameterName::J );
  }

  QString QgsWmsParameters::x() const
  {
    return value( ParameterName::X ).toString();
  }

  QString QgsWmsParameters::y() const
  {
    return value( ParameterName::Y ).toString();
  }

  int QgsWmsParameters::xAsInt() const
  {
    return toInt( ParameterName::X );
  }

  int QgsWmsParameters::yAsInt() const
  {
    return toInt( ParameterName::Y );
  }

  QString QgsWmsParameters::rule() const
  {
    return value( ParameterName::RULE ).toString();
  }

  QString QgsWmsParameters::ruleLabel() const
  {
    return value( ParameterName::RULELABEL ).toString();
  }

  bool QgsWmsParameters::ruleLabelAsBool() const
  {
    return toBool( ParameterName::RULELABEL );
  }

  QString QgsWmsParameters::transparent() const
  {
    return value( ParameterName::TRANSPARENT ).toString();
  }

  bool QgsWmsParameters::transparentAsBool() const
  {
    return toBool( ParameterName::TRANSPARENT );
  }

  QString QgsWmsParameters::scale() const
  {
    return value( ParameterName::SCALE ).toString();
  }

  double QgsWmsParameters::scaleAsDouble() const
  {
    return toDouble( ParameterName::SCALE );
  }

  QString QgsWmsParameters::showFeatureCount() const
  {
    return value( ParameterName::SHOWFEATURECOUNT ).toString();
  }

  bool QgsWmsParameters::showFeatureCountAsBool() const
  {
    return toBool( ParameterName::SHOWFEATURECOUNT );
  }

  QString QgsWmsParameters::featureCount() const
  {
    return value( ParameterName::FEATURE_COUNT ).toString();
  }

  int QgsWmsParameters::featureCountAsInt() const
  {
    return toInt( ParameterName::FEATURE_COUNT );
  }

  QString QgsWmsParameters::boxSpace() const
  {
    return value( ParameterName::BOXSPACE ).toString();
  }

  double QgsWmsParameters::boxSpaceAsDouble() const
  {
    return toDouble( ParameterName::BOXSPACE );
  }

  QString QgsWmsParameters::layerSpace() const
  {
    return value( ParameterName::LAYERSPACE ).toString();
  }

  double QgsWmsParameters::layerSpaceAsDouble() const
  {
    return toDouble( ParameterName::LAYERSPACE );
  }

  QString QgsWmsParameters::layerTitleSpace() const
  {
    return value( ParameterName::LAYERTITLESPACE ).toString();
  }

  double QgsWmsParameters::layerTitleSpaceAsDouble() const
  {
    return toDouble( ParameterName::LAYERTITLESPACE );
  }

  QString QgsWmsParameters::symbolSpace() const
  {
    return value( ParameterName::SYMBOLSPACE ).toString();
  }

  double QgsWmsParameters::symbolSpaceAsDouble() const
  {
    return toDouble( ParameterName::SYMBOLSPACE );
  }

  QString QgsWmsParameters::symbolHeight() const
  {
    return value( ParameterName::SYMBOLHEIGHT ).toString();
  }

  double QgsWmsParameters::symbolHeightAsDouble() const
  {
    return toDouble( SYMBOLHEIGHT );
  }

  QString QgsWmsParameters::symbolWidth() const
  {
    return value( ParameterName::SYMBOLWIDTH ).toString();
  }

  double QgsWmsParameters::symbolWidthAsDouble() const
  {
    return toDouble( SYMBOLWIDTH );
  }

  QString QgsWmsParameters::iconLabelSpace() const
  {
    return value( ParameterName::ICONLABELSPACE ).toString();
  }

  double QgsWmsParameters::iconLabelSpaceAsDouble() const
  {
    return toDouble( ICONLABELSPACE );
  }

  QString QgsWmsParameters::layerFontFamily() const
  {
    return value( ParameterName::LAYERFONTFAMILY ).toString();
  }

  QString QgsWmsParameters::itemFontFamily() const
  {
    return value( ParameterName::ITEMFONTFAMILY ).toString();
  }

  QString QgsWmsParameters::layerFontBold() const
  {
    return value( ParameterName::LAYERFONTBOLD ).toString();
  }

  bool QgsWmsParameters::layerFontBoldAsBool() const
  {
    return toBool( ParameterName::LAYERFONTBOLD );
  }

  QString QgsWmsParameters::itemFontBold() const
  {
    return value( ParameterName::ITEMFONTBOLD ).toString();
  }

  bool QgsWmsParameters::itemFontBoldAsBool() const
  {
    return toBool( ParameterName::ITEMFONTBOLD );
  }

  QString QgsWmsParameters::layerFontItalic() const
  {
    return value( ParameterName::LAYERFONTITALIC ).toString();
  }

  bool QgsWmsParameters::layerFontItalicAsBool() const
  {
    return toBool( ParameterName::LAYERFONTITALIC );
  }

  QString QgsWmsParameters::itemFontItalic() const
  {
    return value( ParameterName::ITEMFONTITALIC ).toString();
  }

  bool QgsWmsParameters::itemFontItalicAsBool() const
  {
    return toBool( ParameterName::ITEMFONTITALIC );
  }

  QString QgsWmsParameters::layerFontSize() const
  {
    return value( ParameterName::LAYERFONTSIZE ).toString();
  }

  double QgsWmsParameters::layerFontSizeAsDouble() const
  {
    return toDouble( LAYERFONTSIZE );
  }

  QString QgsWmsParameters::layerFontColor() const
  {
    return value( ParameterName::LAYERFONTCOLOR ).toString();
  }

  QColor QgsWmsParameters::layerFontColorAsColor() const
  {
    return toColor( ParameterName::LAYERFONTCOLOR );
  }

  QString QgsWmsParameters::itemFontSize() const
  {
    return value( ParameterName::ITEMFONTSIZE ).toString();
  }

  double QgsWmsParameters::itemFontSizeAsDouble() const
  {
    return toDouble( ITEMFONTSIZE );
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
      font.setFamily( layerFontFamily() );

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
      font.setFamily( itemFontFamily() );

    return font;
  }

  QString QgsWmsParameters::layerTitle() const
  {
    return value( ParameterName::LAYERTITLE ).toString();
  }

  bool QgsWmsParameters::layerTitleAsBool() const
  {
    return toBool( ParameterName::LAYERTITLE );
  }

  QgsLegendSettings QgsWmsParameters::legendSettings() const
  {
    QgsLegendSettings settings;
    settings.setTitle( QString() );
    settings.setBoxSpace( boxSpaceAsDouble() );
    settings.setSymbolSize( QSizeF( symbolWidthAsDouble(), symbolHeightAsDouble() ) );

    settings.rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, layerSpaceAsDouble() );
    settings.rstyle( QgsLegendStyle::Subgroup ).setFont( layerFont() );

    settings.rstyle( QgsLegendStyle::SymbolLabel ).setFont( itemFont() );
    settings.rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, symbolSpaceAsDouble() );
    settings.rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Left, iconLabelSpaceAsDouble() );

    return settings;
  }

  QStringList QgsWmsParameters::highlightLabelString() const
  {
    return toStringList( ParameterName::HIGHLIGHT_LABELSTRING, ';' );
  }

  QStringList QgsWmsParameters::highlightLabelSize() const
  {
    return toStringList( ParameterName::HIGHLIGHT_LABELSIZE, ';' );
  }

  QList<int> QgsWmsParameters::highlightLabelSizeAsInt() const
  {
    return toIntList( highlightLabelSize(), ParameterName::HIGHLIGHT_LABELSIZE );
  }

  QStringList QgsWmsParameters::highlightLabelColor() const
  {
    return toStringList( ParameterName::HIGHLIGHT_LABELCOLOR, ';' );
  }

  QList<QColor> QgsWmsParameters::highlightLabelColorAsColor() const
  {
    return toColorList( highlightLabelColor(), ParameterName::HIGHLIGHT_LABELCOLOR );
  }

  QStringList QgsWmsParameters::highlightLabelWeight() const
  {
    return toStringList( ParameterName::HIGHLIGHT_LABELWEIGHT, ';' );
  }

  QList<int> QgsWmsParameters::highlightLabelWeightAsInt() const
  {
    return toIntList( highlightLabelWeight(), ParameterName::HIGHLIGHT_LABELWEIGHT );
  }

  QStringList QgsWmsParameters::highlightLabelFont() const
  {
    return toStringList( ParameterName::HIGHLIGHT_LABELFONT, ';' );
  }

  QStringList QgsWmsParameters::highlightLabelBufferColor() const
  {
    return toStringList( ParameterName::HIGHLIGHT_LABELBUFFERCOLOR, ';' );
  }

  QList<QColor> QgsWmsParameters::highlightLabelBufferColorAsColor() const
  {
    return toColorList( highlightLabelBufferColor(), ParameterName::HIGHLIGHT_LABELBUFFERCOLOR );
  }

  QStringList QgsWmsParameters::highlightLabelBufferSize() const
  {
    return toStringList( ParameterName::HIGHLIGHT_LABELBUFFERSIZE, ';' );
  }

  QList<float> QgsWmsParameters::highlightLabelBufferSizeAsFloat() const
  {
    return toFloatList( highlightLabelBufferSize(), ParameterName::HIGHLIGHT_LABELBUFFERSIZE );
  }

  QString QgsWmsParameters::wmsPrecision() const
  {
    return value( ParameterName::WMS_PRECISION ).toString();
  }

  int QgsWmsParameters::wmsPrecisionAsInt() const
  {
    return toInt( ParameterName::WMS_PRECISION );
  }

  QString QgsWmsParameters::sld() const
  {
    return value( ParameterName::SLD ).toString();
  }

  QStringList QgsWmsParameters::filters() const
  {
    return toStringList( ParameterName::FILTER, ';' );
  }

  QString QgsWmsParameters::filterGeom() const
  {
    return value( ParameterName::FILTER_GEOM ).toString();
  }

  QStringList QgsWmsParameters::selections() const
  {
    return toStringList( ParameterName::SELECTION );
  }

  QStringList QgsWmsParameters::opacities() const
  {
    return toStringList( ParameterName::OPACITIES );
  }

  QList<int> QgsWmsParameters::opacitiesAsInt() const
  {
    return toIntList( opacities(), ParameterName::OPACITIES );
  }

  QStringList QgsWmsParameters::allLayersNickname() const
  {
    QStringList layer = toStringList( ParameterName::LAYER );
    QStringList layers = toStringList( ParameterName::LAYERS );
    return layer << layers;
  }

  QStringList QgsWmsParameters::queryLayersNickname() const
  {
    return toStringList( ParameterName::QUERY_LAYERS );
  }

  QStringList QgsWmsParameters::allStyles() const
  {
    QStringList style = toStringList( ParameterName::STYLE );
    QStringList styles = toStringList( ParameterName::STYLES );
    return style << styles;
  }

  QList<QgsWmsParametersLayer> QgsWmsParameters::layersParameters() const
  {
    QList<QgsWmsParametersLayer> parameters;
    QStringList layers = allLayersNickname();
    QStringList styles = allStyles();
    QStringList filter = filters();
    QStringList selection = selections();
    QList<int> opacities = opacitiesAsInt();

    // filter format: "LayerName:filterString;LayerName2:filterString2;..."
    // several filters can be defined for one layer
    QMultiMap<QString, QString> layerFilters;
    Q_FOREACH ( QString f, filter )
    {
      QStringList splits = f.split( ":" );
      if ( splits.size() == 2 )
      {
        layerFilters.insert( splits[0], splits[1] );
      }
      else
      {
        QString filterStr = value( ParameterName::FILTER ).toString();
        raiseError( "FILTER ('" + filterStr + "') is not properly formatted" );
      }
    }

    // selection format: "LayerName:id0,id1;LayerName2:id0,id1;..."
    // several filters can be defined for one layer
    QMultiMap<QString, QString> layerSelections;
    Q_FOREACH ( QString s, selection )
    {
      QStringList splits = s.split( ":" );
      if ( splits.size() == 2 )
      {
        layerSelections.insert( splits[0], splits[1] );
      }
      else
      {
        QString selStr = value( ParameterName::SELECTION ).toString();
        raiseError( "SELECTION ('" + selStr + "') is not properly formatted" );
      }
    }

    for ( int i = 0; i < layers.size(); i++ )
    {
      QString layer = layers[i];
      QgsWmsParametersLayer param;
      param.mNickname = layer;

      if ( i < styles.count() )
        param.mStyle = styles[i];

      if ( i < opacities.count() )
        param.mOpacity = opacities[i];

      if ( layerFilters.contains( layer ) )
      {
        QMultiMap<QString, QString>::const_iterator it;
        it = layerFilters.find( layer );
        while ( it != layerFilters.end() && it.key() == layer )
        {
          param.mFilter.append( it.value() );
          ++it;
        }
      }

      if ( layerSelections.contains( layer ) )
      {
        QMultiMap<QString, QString>::const_iterator it;
        it = layerSelections.find( layer );
        while ( it != layerSelections.end() && it.key() == layer )
        {
          param.mSelection << it.value().split( "," );
          ++it;
        }
      }

      parameters.append( param );
    }

    return parameters;
  }

  QList<QgsWmsParametersHighlightLayer> QgsWmsParameters::highlightLayersParameters() const
  {
    QList<QgsWmsParametersHighlightLayer> params;
    QList<QgsGeometry> geoms = highlightGeomAsGeom();
    QStringList slds = highlightSymbol();
    QStringList labels = highlightLabelString();
    QList<QColor> colors = highlightLabelColorAsColor();
    QList<int> sizes = highlightLabelSizeAsInt();
    QList<int> weights = highlightLabelWeightAsInt();
    QStringList fonts = highlightLabelFont();
    QList<QColor> bufferColors = highlightLabelBufferColorAsColor();
    QList<float> bufferSizes = highlightLabelBufferSizeAsFloat();

    int nLayers = std::min( geoms.size(), slds.size() );
    for ( int i = 0; i < nLayers; i++ )
    {
      QgsWmsParametersHighlightLayer param;
      param.mName = "highlight_" + QString::number( i );
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

      params.append( param );
    }

    return params;
  }

  QString QgsWmsParameters::backgroundColor() const
  {
    return value( ParameterName::BGCOLOR ).toString();
  }

  QColor QgsWmsParameters::backgroundColorAsColor() const
  {
    return toColor( ParameterName::BGCOLOR );
  }

  QString QgsWmsParameters::composerTemplate() const
  {
    return value( ParameterName::TEMPLATE ).toString();
  }

  QgsWmsParametersComposerMap QgsWmsParameters::composerMapParameters( int mapId ) const
  {
    QgsWmsParametersComposerMap param;
    param.mId = mapId;

    //map extent is mandatory
    QString extentStr = value( ParameterName::EXTENT, mapId ).toString();
    if ( extentStr.isEmpty() )
      return param;

    QString pMapId = "MAP" + QString::number( mapId );

    QgsRectangle extent = toRectangle( ParameterName::EXTENT, mapId );
    if ( extent.isEmpty() )
      return param;

    param.mHasExtent = !extent.isEmpty();
    param.mExtent = extent;

    // scale
    if ( !value( ParameterName::SCALE, mapId ).toString().isEmpty() )
    {
      param.mScale = toDouble( ParameterName::SCALE, mapId );
    }

    // rotation
    if ( !value( ParameterName::ROTATION, mapId ).toString().isEmpty() )
    {
      param.mRotation = toDouble( ParameterName::ROTATION, mapId );
    }

    //grid space x / y
    if ( !value( ParameterName::GRID_INTERVAL_X, mapId ).toString().isEmpty() && !value( ParameterName::GRID_INTERVAL_Y, mapId ).toString().isEmpty() )
    {
      param.mGridX = toDouble( ParameterName::GRID_INTERVAL_X, mapId );
      param.mGridY = toDouble( ParameterName::GRID_INTERVAL_Y, mapId );
    }

    //layers
    QList<QgsWmsParametersLayer> lParams;
    QStringList layers = toStringList( ParameterName::LAYERS, mapId, ',' );
    QStringList styles = toStringList( ParameterName::STYLES, mapId, ',' );
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
    QList<QgsGeometry> geoms = toGeomList( toStringList( ParameterName::HIGHLIGHT_GEOM, mapId, ';' ), ParameterName::HIGHLIGHT_GEOM, mapId );
    QStringList slds = toStringList( ParameterName::HIGHLIGHT_SYMBOL, mapId, ';' );
    QStringList labels = toStringList( ParameterName::HIGHLIGHT_LABELSTRING, mapId, ';' );
    QList<QColor> colors = toColorList( toStringList( ParameterName::HIGHLIGHT_LABELCOLOR, mapId, ';' ), ParameterName::HIGHLIGHT_LABELCOLOR, mapId );
    QList<int> sizes = toIntList( toStringList( ParameterName::HIGHLIGHT_LABELSIZE, mapId, ';' ), ParameterName::HIGHLIGHT_LABELSIZE, mapId );
    QList<int> weights = toIntList( toStringList( ParameterName::HIGHLIGHT_LABELWEIGHT, mapId, ';' ), ParameterName::HIGHLIGHT_LABELWEIGHT, mapId );
    QStringList fonts = toStringList( ParameterName::HIGHLIGHT_LABELFONT, mapId, ';' );
    QList<QColor> bufferColors = toColorList( toStringList( ParameterName::HIGHLIGHT_LABELBUFFERCOLOR, mapId, ';' ), ParameterName::HIGHLIGHT_LABELBUFFERCOLOR, mapId );
    QList<float> bufferSizes = toFloatList( toStringList( ParameterName::HIGHLIGHT_LABELBUFFERSIZE, mapId, ';' ), ParameterName::HIGHLIGHT_LABELBUFFERSIZE, mapId );

    int nHLayers = std::min( geoms.size(), slds.size() );
    for ( int i = 0; i < nHLayers; i++ )
    {
      QgsWmsParametersHighlightLayer hParam;
      hParam.mName = pMapId + "_highlight_" + QString::number( i );
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

      hParams.append( hParam );
    }
    param.mHighlightLayers = hParams;

    return param;
  }

  QString QgsWmsParameters::externalWMSUri( const QString &id ) const
  {
    if ( !mExternalWMSParameters.contains( id ) )
    {
      return QString();
    }

    QgsDataSourceUri wmsUri;
    const QMap<QString, QString> &paramMap = mExternalWMSParameters[ id ];
    QMap<QString, QString>::const_iterator paramIt = paramMap.constBegin();
    for ( ; paramIt != paramMap.constEnd(); ++paramIt )
    {
      wmsUri.setParam( paramIt.key().toLower(), paramIt.value() );
    }
    return wmsUri.encodedUri();
  }

  bool QgsWmsParameters::withGeometry() const
  {
    return toBool( ParameterName::WITH_GEOMETRY );
  }

  bool QgsWmsParameters::withMapTip() const
  {
    return toBool( ParameterName::WITH_MAPTIP );
  }

  QString QgsWmsParameters::name( ParameterName name ) const
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<ParameterName>() );
    return metaEnum.valueToKey( name );
  }

  void QgsWmsParameters::log( const QString &msg ) const
  {
    QgsMessageLog::logMessage( msg, "Server", QgsMessageLog::INFO );
  }

  void QgsWmsParameters::raiseError( ParameterName paramName ) const
  {
    const QString value = mParameters[paramName].mValue.toString();
    const QString param = name( paramName );
    const QString type = QVariant::typeToName( mParameters[paramName].mType );
    raiseError( param + " ('" + value + "') cannot be converted into " + type );
  }

  void QgsWmsParameters::raiseError( ParameterName paramName, int mapId ) const
  {
    const QString value = mComposerParameters[mapId][paramName].mValue.toString();
    const QString param = name( paramName );
    const QString type = QVariant::typeToName( mComposerParameters[mapId][paramName].mType );
    raiseError( "MAP" + QString::number( mapId ) + ":" + param + " ('" + value + "') cannot be converted into " + type );
  }

  void QgsWmsParameters::raiseError( const QString &msg ) const
  {
    throw QgsBadRequestException( QStringLiteral( "Invalid WMS Parameter" ), msg );
  }
}
