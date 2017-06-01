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
#include "qgsmessagelog.h"
#include <iostream>

namespace QgsWms
{
  QgsWmsParameters::QgsWmsParameters()
  {
    const Parameter pHighlightGeom = { ParameterName::HIGHLIGHT_GEOM,
                                       QVariant::String,
                                       QVariant( "" ),
                                       QVariant()
                                     };
    save( pHighlightGeom );

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
                                        QVariant( "" ),
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
                                              QVariant( "" ),
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

    const Parameter pStyle = { ParameterName::STYLE,
                               QVariant::String,
                               QVariant( "" ),
                               QVariant()
                             };
    save( pLayers );

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

    const Parameter pSelection = { ParameterName::SELECTION,
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
    save( pSelection );
  }

  QgsWmsParameters::QgsWmsParameters( const QgsServerRequest::Parameters &parameters )
  {
    load( parameters );
  }

  void QgsWmsParameters::load( const QgsServerRequest::Parameters &parameters )
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
  }

  void QgsWmsParameters::save( const Parameter &parameter )
  {
    mParameters[ parameter.mName ] = parameter;
  }

  QVariant QgsWmsParameters::value( ParameterName name ) const
  {
    return mParameters[name].mValue;
  }

  QStringList QgsWmsParameters::highlightGeom() const
  {
    return toStringList( ParameterName::HIGHLIGHT_GEOM, ';' );
  }

  QList<QgsGeometry> QgsWmsParameters::highlightGeomAsGeom() const
  {
    QList<QgsGeometry> geometries;

    Q_FOREACH ( QString wkt, highlightGeom() )
    {
      QgsGeometry g( QgsGeometry::fromWkt( wkt ) );

      if ( g.isGeosValid() )
      {
        geometries.append( g );
      }
      else
      {
        QString val = value( ParameterName::HIGHLIGHT_GEOM ).toString();
        QString msg = "HIGHLIGHT_GEOM ('" + val + "') cannot be converted into a list of geometries";
        raiseError( msg );
      }
    }

    return geometries;
  }

  QStringList QgsWmsParameters::highlightSymbol() const
  {
    return toStringList( ParameterName::HIGHLIGHT_SYMBOL, ';' );
  }

  QString QgsWmsParameters::crs() const
  {
    return value( ParameterName::CRS ).toString();
  }

  QString QgsWmsParameters::bbox() const
  {
    return value( ParameterName::BBOX ).toString();
  }

  QgsRectangle QgsWmsParameters::bboxAsRectangle() const
  {
    QgsRectangle extent;

    if ( !bbox().isEmpty() )
    {
      QStringList corners = bbox().split( "," );

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
            raiseError( "BBOX ('" + bbox() + "') cannot be converted into a rectangle" );
          }
        }

        extent = QgsRectangle( d[0], d[1], d[2], d[3] );
      }
      else
      {
        raiseError( "BBOX ('" + bbox() + "') cannot be converted into a rectangle" );
      }
    }

    return extent;
  }

  int QgsWmsParameters::height() const
  {
    bool ok = false;
    int height = value( ParameterName::HEIGHT ).toInt( &ok );

    if ( ! ok )
      raiseError( ParameterName::HEIGHT );

    return height;
  }

  int QgsWmsParameters::width() const
  {
    bool ok = false;
    int width = value( ParameterName::WIDTH ).toInt( &ok );

    if ( ! ok )
      raiseError( ParameterName::WIDTH );

    return width;
  }

  QStringList QgsWmsParameters::toStringList( ParameterName name, char delimiter ) const
  {
    return value( name ).toString().split( delimiter, QString::SkipEmptyParts );
  }

  QList<int> QgsWmsParameters::toIntList( QStringList l, ParameterName p ) const
  {
    QList<int> elements;

    Q_FOREACH ( QString element, l )
    {
      bool ok;
      int e = element.toInt( &ok );

      if ( ok )
      {
        elements.append( e );
      }
      else
      {
        QString val = value( p ).toString();
        QString n = name( p );
        QString msg = n + " ('" + val + "') cannot be converted into a list of int";
        raiseError( msg );
      }
    }

    return elements;
  }

  QList<float> QgsWmsParameters::toFloatList( QStringList l, ParameterName p ) const
  {
    QList<float> elements;

    Q_FOREACH ( QString element, l )
    {
      bool ok;
      float e = element.toFloat( &ok );

      if ( ok )
      {
        elements.append( e );
      }
      else
      {
        QString val = value( p ).toString();
        QString n = name( p );
        QString msg = n + " ('" + val + "') cannot be converted into a list of float";
        raiseError( msg );
      }
    }

    return elements;
  }

  QList<QColor> QgsWmsParameters::toColorList( QStringList l, ParameterName p ) const
  {
    QList<QColor> elements;

    Q_FOREACH ( QString element, l )
    {
      QColor c = QColor( element );

      if ( c.isValid() )
      {
        elements.append( c );
      }
      else
      {
        QString val = value( p ).toString();
        QString n = name( p );
        QString msg = n + " ('" + val + "') cannot be converted into a list of colors";
        raiseError( msg );
      }
    }

    return elements;
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

  QString QgsWmsParameters::sld() const
  {
    return value( ParameterName::SLD ).toString();
  }

  QStringList QgsWmsParameters::filters() const
  {
    return toStringList( ParameterName::FILTER, ';' );
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

  QStringList QgsWmsParameters::allStyles() const
  {
    QStringList style = value( ParameterName::STYLE ).toString().split( ",", QString::SkipEmptyParts );
    QStringList styles = value( ParameterName::STYLES ).toString().split( "," );
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

    int nLayers = qMin( geoms.size(), slds.size() );
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

  void QgsWmsParameters::raiseError( const QString &msg ) const
  {
    throw QgsBadRequestException( QStringLiteral( "Invalid WMS Parameter" ), msg );
  }
}
