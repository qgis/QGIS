/***************************************************************************
  qgspallabeling.cpp
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspallabeling.h"

#include <iostream>
#include <list>

#include <pal/pal.h>
#include <pal/feature.h>
#include <pal/layer.h>
#include <pal/palgeometry.h>
#include <pal/palexception.h>
#include <pal/problem.h>
#include <pal/labelposition.h>

#include <geos_c.h>

#include <cmath>

#include <QByteArray>
#include <QString>
#include <QFontMetrics>
#include <QTime>
#include <QPainter>

#include "qgsdiagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgslabelsearchtree.h"
#include "qgssearchtreenode.h"
#include "qgssearchstring.h"

#include <qgslogger.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmaprenderer.h>
#include <QMessageBox>

using namespace pal;


class QgsPalGeometry : public PalGeometry
{
  public:
    QgsPalGeometry( QgsFeatureId id, QString text, GEOSGeometry* g )
        : mG( g )
        , mText( text )
        , mId( id )
        , mInfo( NULL )
        , mIsDiagram( false )
    {
      mStrId = FID_TO_STRING( id ).toAscii();
    }

    ~QgsPalGeometry()
    {
      if ( mG )
        GEOSGeom_destroy( mG );
      delete mInfo;
    }

    // getGeosGeometry + releaseGeosGeometry is called twice: once when adding, second time when labeling

    GEOSGeometry* getGeosGeometry()
    {
      return mG;
    }
    void releaseGeosGeometry( GEOSGeometry* /*geom*/ )
    {
      // nothing here - we'll delete the geometry in destructor
    }

    const char* strId() { return mStrId.data(); }
    QString text() { return mText; }

    pal::LabelInfo* info( QFontMetricsF* fm, const QgsMapToPixel* xform, double fontScale )
    {
      if ( mInfo )
        return mInfo;

      // create label info!
      QgsPoint ptZero = xform->toMapCoordinates( 0, 0 );
      QgsPoint ptSize = xform->toMapCoordinatesF( 0.0, -fm->height() / fontScale );

      mInfo = new pal::LabelInfo( mText.count(), ptSize.y() - ptZero.y() );
      for ( int i = 0; i < mText.count(); i++ )
      {
        mInfo->char_info[i].chr = mText[i].unicode();
        ptSize = xform->toMapCoordinatesF( fm->width( mText[i] ) / fontScale , 0.0 );
        mInfo->char_info[i].width = ptSize.x() - ptZero.x();
      }
      return mInfo;
    }

    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& dataDefinedValues() const { return mDataDefinedValues; }
    void addDataDefinedValue( QgsPalLayerSettings::DataDefinedProperties p, QVariant v ) { mDataDefinedValues.insert( p, v ); }

    void setIsDiagram( bool d ) { mIsDiagram = d; }
    bool isDiagram() const { return mIsDiagram; }

    void addDiagramAttribute( int index, QVariant value ) { mDiagramAttributes.insert( index, value ); }
    const QgsAttributeMap& diagramAttributes() { return mDiagramAttributes; }

  protected:
    GEOSGeometry* mG;
    QString mText;
    QByteArray mStrId;
    QgsFeatureId mId;
    LabelInfo* mInfo;
    bool mIsDiagram;
    /**Stores attribute values for data defined properties*/
    QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant > mDataDefinedValues;

    /**Stores attribute values for diagram rendering*/
    QgsAttributeMap mDiagramAttributes;
};

// -------------

QgsPalLayerSettings::QgsPalLayerSettings()
    : palLayer( NULL ), fontMetrics( NULL ), ct( NULL ), extentGeom( NULL )
{
  placement = AroundPoint;
  placementFlags = 0;
  //textFont = QFont();
  textColor = Qt::black;
  enabled = false;
  priority = 5;
  obstacle = true;
  dist = 0;
  scaleMin = 0;
  scaleMax = 0;
  bufferSize = 1;
  bufferColor = Qt::white;
  formatNumbers = false;
  decimals = 3;
  plusSign = false;
  labelPerPart = false;
  mergeLines = false;
  multiLineLabels = false;
  minFeatureSize = 0.0;
  vectorScaleFactor = 1.0;
  rasterCompressFactor = 1.0;
  addDirectionSymbol = false;
  fontSizeInMapUnits = false;
  distInMapUnits = false;
}

QgsPalLayerSettings::QgsPalLayerSettings( const QgsPalLayerSettings& s )
{
  // copy only permanent stuff
  fieldName = s.fieldName;
  isExpression = s.isExpression;
  placement = s.placement;
  placementFlags = s.placementFlags;
  textFont = s.textFont;
  textColor = s.textColor;
  enabled = s.enabled;
  priority = s.priority;
  obstacle = s.obstacle;
  dist = s.dist;
  scaleMin = s.scaleMin;
  scaleMax = s.scaleMax;
  bufferSize = s.bufferSize;
  bufferColor = s.bufferColor;
  formatNumbers = s.formatNumbers;
  decimals = s.decimals;
  plusSign = s.plusSign;
  labelPerPart = s.labelPerPart;
  mergeLines = s.mergeLines;
  multiLineLabels = s.multiLineLabels;
  minFeatureSize = s.minFeatureSize;
  vectorScaleFactor = s.vectorScaleFactor;
  rasterCompressFactor = s.rasterCompressFactor;
  addDirectionSymbol = s.addDirectionSymbol;
  fontSizeInMapUnits = s.fontSizeInMapUnits;
  distInMapUnits = s.distInMapUnits;

  dataDefinedProperties = s.dataDefinedProperties;
  fontMetrics = NULL;
  ct = NULL;
  extentGeom = NULL;
}


QgsPalLayerSettings::~QgsPalLayerSettings()
{
  // pal layer is deleted internally in PAL

  delete fontMetrics;
  delete ct;

  delete extentGeom;
}

static QColor _readColor( QgsVectorLayer* layer, QString property )
{
  int r = layer->customProperty( property + "R" ).toInt();
  int g = layer->customProperty( property + "G" ).toInt();
  int b = layer->customProperty( property + "B" ).toInt();
  return QColor( r, g, b );
}

static void _writeColor( QgsVectorLayer* layer, QString property, QColor color )
{
  layer->setCustomProperty( property + "R", color.red() );
  layer->setCustomProperty( property + "G", color.green() );
  layer->setCustomProperty( property + "B", color.blue() );
}

static void _writeDataDefinedPropertyMap( QgsVectorLayer* layer, const QMap< QgsPalLayerSettings::DataDefinedProperties, int >& propertyMap )
{
  if ( !layer )
  {
    return;
  }

  for ( int i = 0; i < 15; ++i )
  {
    QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator it = propertyMap.find(( QgsPalLayerSettings::DataDefinedProperties )i );
    QVariant propertyValue;
    if ( it == propertyMap.constEnd() )
    {
      propertyValue = QVariant(); //we cannot delete properties, so we just insert an invalid variant
    }
    else
    {
      propertyValue = *it;
    }
    layer->setCustomProperty( "labeling/dataDefinedProperty" + QString::number( i ), propertyValue );
  }
}

static void _readDataDefinedProperty( QgsVectorLayer* layer, QgsPalLayerSettings::DataDefinedProperties p, \
                                      QMap< QgsPalLayerSettings::DataDefinedProperties, int >& propertyMap )
{
  QVariant propertyField = layer->customProperty( "labeling/dataDefinedProperty" + QString::number( p ) );
  bool conversionOk;
  int fieldIndex;

  if ( propertyField.isValid() )
  {
    fieldIndex = propertyField.toInt( &conversionOk );
    if ( conversionOk )
    {
      propertyMap.insert( p, fieldIndex );
    }
  }
}

static void _readDataDefinedPropertyMap( QgsVectorLayer* layer, QMap< QgsPalLayerSettings::DataDefinedProperties, int >& propertyMap )
{
  if ( !layer )
  {
    return;
  }

  _readDataDefinedProperty( layer, QgsPalLayerSettings::Size, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::Color, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::Bold, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::Italic, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::Underline, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::Strikeout, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::Family, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::BufferSize, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::BufferColor, propertyMap );
  _readDataDefinedProperty( layer,  QgsPalLayerSettings::PositionX, propertyMap );
  _readDataDefinedProperty( layer,  QgsPalLayerSettings::PositionY, propertyMap );
  _readDataDefinedProperty( layer,  QgsPalLayerSettings::Hali, propertyMap );
  _readDataDefinedProperty( layer,  QgsPalLayerSettings::Vali, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::LabelDistance, propertyMap );
  _readDataDefinedProperty( layer, QgsPalLayerSettings::Rotation, propertyMap );
}

void QgsPalLayerSettings::readFromLayer( QgsVectorLayer* layer )
{
  if ( layer->customProperty( "labeling" ).toString() != QString( "pal" ) )
    return; // there's no information available

  fieldName = layer->customProperty( "labeling/fieldName" ).toString();
  isExpression = layer->customProperty( "labeling/isExpression").toBool();
  placement = ( Placement ) layer->customProperty( "labeling/placement" ).toInt();
  placementFlags = layer->customProperty( "labeling/placementFlags" ).toUInt();
  QString fontFamily = layer->customProperty( "labeling/fontFamily" ).toString();
  double fontSize = layer->customProperty( "labeling/fontSize" ).toDouble();
  int fontWeight = layer->customProperty( "labeling/fontWeight" ).toInt();
  bool fontItalic = layer->customProperty( "labeling/fontItalic" ).toBool();
  textFont = QFont( fontFamily, fontSize, fontWeight, fontItalic );
  textFont.setUnderline( layer->customProperty( "labeling/fontUnderline" ).toBool() );
  textFont.setStrikeOut( layer->customProperty( "labeling/fontStrikeout" ).toBool() );
  textFont.setPointSizeF( fontSize ); //double precision needed because of map units
  textColor = _readColor( layer, "labeling/textColor" );
  enabled = layer->customProperty( "labeling/enabled" ).toBool();
  priority = layer->customProperty( "labeling/priority" ).toInt();
  obstacle = layer->customProperty( "labeling/obstacle" ).toBool();
  dist = layer->customProperty( "labeling/dist" ).toDouble();
  scaleMin = layer->customProperty( "labeling/scaleMin" ).toInt();
  scaleMax = layer->customProperty( "labeling/scaleMax" ).toInt();
  bufferSize = layer->customProperty( "labeling/bufferSize" ).toDouble();
  bufferColor = _readColor( layer, "labeling/bufferColor" );
  formatNumbers = layer->customProperty( "labeling/formatNumbers" ).toBool();
  decimals = layer->customProperty( "labeling/decimals" ).toInt();
  plusSign = layer->customProperty( "labeling/plussign" ).toInt();
  labelPerPart = layer->customProperty( "labeling/labelPerPart" ).toBool();
  mergeLines = layer->customProperty( "labeling/mergeLines" ).toBool();
  multiLineLabels = layer->customProperty( "labeling/multiLineLabels" ).toBool();
  addDirectionSymbol = layer->customProperty( "labeling/addDirectionSymbol" ).toBool();
  minFeatureSize = layer->customProperty( "labeling/minFeatureSize" ).toDouble();
  fontSizeInMapUnits = layer->customProperty( "labeling/fontSizeInMapUnits" ).toBool();
  distInMapUnits = layer->customProperty( "labeling/distInMapUnits" ).toBool();
  _readDataDefinedPropertyMap( layer, dataDefinedProperties );
}

void QgsPalLayerSettings::writeToLayer( QgsVectorLayer* layer )
{
  // this is a mark that labeling information is present
  layer->setCustomProperty( "labeling", "pal" );

  layer->setCustomProperty( "labeling/fieldName", fieldName );
  layer->setCustomProperty( "labeling/isExpression", isExpression );
  layer->setCustomProperty( "labeling/placement", placement );
  layer->setCustomProperty( "labeling/placementFlags", ( unsigned int )placementFlags );

  layer->setCustomProperty( "labeling/fontFamily", textFont.family() );
  layer->setCustomProperty( "labeling/fontSize", textFont.pointSizeF() );
  layer->setCustomProperty( "labeling/fontWeight", textFont.weight() );
  layer->setCustomProperty( "labeling/fontItalic", textFont.italic() );
  layer->setCustomProperty( "labeling/fontStrikeout", textFont.strikeOut() );
  layer->setCustomProperty( "labeling/fontUnderline", textFont.underline() );

  _writeColor( layer, "labeling/textColor", textColor );
  layer->setCustomProperty( "labeling/enabled", enabled );
  layer->setCustomProperty( "labeling/priority", priority );
  layer->setCustomProperty( "labeling/obstacle", obstacle );
  layer->setCustomProperty( "labeling/dist", dist );
  layer->setCustomProperty( "labeling/scaleMin", scaleMin );
  layer->setCustomProperty( "labeling/scaleMax", scaleMax );
  layer->setCustomProperty( "labeling/bufferSize", bufferSize );
  _writeColor( layer, "labeling/bufferColor", bufferColor );
  layer->setCustomProperty( "labeling/formatNumbers", formatNumbers );
  layer->setCustomProperty( "labeling/decimals", decimals );
  layer->setCustomProperty( "labeling/plussign", plusSign );
  layer->setCustomProperty( "labeling/labelPerPart", labelPerPart );
  layer->setCustomProperty( "labeling/mergeLines", mergeLines );
  layer->setCustomProperty( "labeling/multiLineLabels", multiLineLabels );
  layer->setCustomProperty( "labeling/addDirectionSymbol", addDirectionSymbol );
  layer->setCustomProperty( "labeling/minFeatureSize", minFeatureSize );
  layer->setCustomProperty( "labeling/fontSizeInMapUnits", fontSizeInMapUnits );
  layer->setCustomProperty( "labeling/distInMapUnits", distInMapUnits );
  _writeDataDefinedPropertyMap( layer, dataDefinedProperties );
}

void QgsPalLayerSettings::setDataDefinedProperty( DataDefinedProperties p, int attributeIndex )
{
  dataDefinedProperties.insert( p, attributeIndex );
}

void QgsPalLayerSettings::removeDataDefinedProperty( DataDefinedProperties p )
{
  dataDefinedProperties.remove( p );
}

bool QgsPalLayerSettings::checkMinimumSizeMM( const QgsRenderContext& ct, QgsGeometry* geom, double minSize ) const
{
  if ( minSize <= 0 )
  {
    return true;
  }

  if ( !geom )
  {
    return false;
  }

  QGis::GeometryType featureType = geom->type();
  if ( featureType == QGis::Point ) //minimum size does not apply to point features
  {
    return true;
  }

  double mapUnitsPerMM = ct.mapToPixel().mapUnitsPerPixel() * ct.scaleFactor();
  if ( featureType == QGis::Line )
  {
    double length = geom->length();
    if ( length >= 0.0 )
    {
      return ( length >= ( minSize * mapUnitsPerMM ) );
    }
  }
  else if ( featureType == QGis::Polygon )
  {
    double area = geom->area();
    if ( area >= 0.0 )
    {
      return ( sqrt( area ) >= ( minSize * mapUnitsPerMM ) );
    }
  }
  return true; //should never be reached. Return true in this case to label such geometries anyway.
}

void QgsPalLayerSettings::calculateLabelSize( const QFontMetricsF* fm, QString text, double& labelX, double& labelY )
{
  if ( !fm )
  {
    return;
  }

  if ( addDirectionSymbol && !multiLineLabels && placement == QgsPalLayerSettings::Line ) //consider the space needed for the direction symbol
  {
    text.append( ">" );
  }
  QRectF labelRect = fm->boundingRect( text );
  double w, h;
  if ( !multiLineLabels )
  {
    w = labelRect.width() / rasterCompressFactor;
    h = labelRect.height() / rasterCompressFactor;
  }
  else
  {
    QStringList multiLineSplit = text.split( "\n" );
    h = fm->height() * multiLineSplit.size() / rasterCompressFactor;
    w = 0;
    for ( int i = 0; i < multiLineSplit.size(); ++i )
    {
      double width = fm->width( multiLineSplit.at( i ) );
      if ( width > w )
      {
        w = width;
      }
      w /= rasterCompressFactor;
    }
  }
  QgsPoint ptSize = xform->toMapCoordinatesF( w, h );

  labelX = qAbs( ptSize.x() - ptZero.x() );
  labelY = qAbs( ptSize.y() - ptZero.y() );
}

void QgsPalLayerSettings::registerFeature(QgsVectorLayer* layer,  QgsFeature& f, const QgsRenderContext& context )
{
  QString labelText;
  // Check to see if we are a expression string.
  if (isExpression)
  {
    QgsSearchString searchString;
    // We don't do any validating here as we should only have a vaild expression at this point.
    searchString.setString( fieldName );

    QgsSearchTreeNode* searchTree = searchString.tree();
    if ( !searchTree )
    {
      return;
    }

    QgsSearchTreeValue outValue;
    searchTree->getValue( outValue, searchTree, layer->dataProvider()->fields() , f );

    if (outValue.isError())
    {
        QgsDebugMsg("Expression Label Error = " + outValue.string());
       return;
    }
    labelText  = outValue.string();
  }
  else if ( formatNumbers == true && ( f.attributeMap()[fieldIndex].type() == QVariant::Int ||
                                       f.attributeMap()[fieldIndex].type() == QVariant::Double ) )
  {
     QString numberFormat;
     double d = f.attributeMap()[fieldIndex].toDouble();
     if ( d > 0 && plusSign == true )
     {
       numberFormat.append( "+" );
     }
     numberFormat.append( "%1" );
     labelText = numberFormat.arg( d, 0, 'f', decimals );
  }
  else
  {
      labelText = f.attributeMap()[fieldIndex].toString();
  }

  double labelX, labelY; // will receive label size
  QFont labelFont = textFont;

  //data defined label size?
  QMap< DataDefinedProperties, int >::const_iterator it = dataDefinedProperties.find( QgsPalLayerSettings::Size );
  if ( it != dataDefinedProperties.constEnd() )
  {
    //find out size
    QVariant size = f.attributeMap().value( *it );
    if ( size.isValid() )
    {
      double sizeDouble = size.toDouble();
      if ( sizeDouble <= 0 )
      {
        return;
      }
      labelFont.setPixelSize( sizeToPixel( sizeDouble, context ) );
    }
    QFontMetricsF labelFontMetrics( labelFont );
    calculateLabelSize( &labelFontMetrics, labelText, labelX, labelY );
  }
  else
  {
    calculateLabelSize( fontMetrics, labelText, labelX, labelY );
  }

  QgsGeometry* geom = f.geometry();

  if ( ct ) // reproject the geometry if necessary
    geom->transform( *ct );

  if ( !checkMinimumSizeMM( context, geom, minFeatureSize ) )
  {
    return;
  }

  // CLIP the geometry if it is bigger than the extent
  QgsGeometry* geomClipped = NULL;
  GEOSGeometry* geos_geom;
  bool do_clip = !extentGeom->contains( geom );
  if ( do_clip )
  {
    geomClipped = geom->intersection( extentGeom ); // creates new geometry
    geos_geom = geomClipped->asGeos();
  }
  else
  {
    geos_geom = geom->asGeos();
  }

  if ( geos_geom == NULL )
    return; // invalid geometry
  GEOSGeometry* geos_geom_clone = GEOSGeom_clone( geos_geom );
  if ( do_clip )
    delete geomClipped;

  //data defined position / alignment / rotation?
  bool dataDefinedPosition = false;
  bool dataDefinedRotation = false;
  double xPos = 0.0, yPos = 0.0, angle = 0.0;
  bool ddXPos, ddYPos;

  QMap< DataDefinedProperties, int >::const_iterator dPosXIt = dataDefinedProperties.find( QgsPalLayerSettings::PositionX );
  if ( dPosXIt != dataDefinedProperties.constEnd() )
  {
    QMap< DataDefinedProperties, int >::const_iterator dPosYIt = dataDefinedProperties.find( QgsPalLayerSettings::PositionY );
    if ( dPosYIt != dataDefinedProperties.constEnd() )
    {
      //data defined position. But field values could be NULL -> positions will be generated by PAL
      xPos = f.attributeMap().value( *dPosXIt ).toDouble( &ddXPos );
      yPos = f.attributeMap().value( *dPosYIt ).toDouble( &ddYPos );

      if ( ddXPos && ddYPos )
      {
        dataDefinedPosition = true;
        //x/y shift in case of alignment
        double xdiff = 0;
        double ydiff = 0;

        //horizontal alignment
        QMap< DataDefinedProperties, int >::const_iterator haliIt = dataDefinedProperties.find( QgsPalLayerSettings::Hali );
        if ( haliIt != dataDefinedProperties.end() )
        {
          QString haliString = f.attributeMap().value( *haliIt ).toString();
          if ( haliString.compare( "Center", Qt::CaseInsensitive ) == 0 )
          {
            xdiff -= labelX / 2.0;
          }
          else if ( haliString.compare( "Right", Qt::CaseInsensitive ) == 0 )
          {
            xdiff -= labelX;
          }
        }

        //vertical alignment
        QMap< DataDefinedProperties, int >::const_iterator valiIt = dataDefinedProperties.find( QgsPalLayerSettings::Vali );
        if ( valiIt != dataDefinedProperties.constEnd() )
        {
          QString valiString = f.attributeMap().value( *valiIt ).toString();
          if ( valiString.compare( "Bottom", Qt::CaseInsensitive ) != 0 )
          {
            if ( valiString.compare( "Top", Qt::CaseInsensitive ) == 0 || valiString.compare( "Cap", Qt::CaseInsensitive ) == 0 )
            {
              ydiff -= labelY;
            }
            else
            {
              QFontMetrics labelFontMetrics( labelFont );
              double descentRatio = labelFontMetrics.descent() / labelFontMetrics.height();

              if ( valiString.compare( "Base", Qt::CaseInsensitive ) == 0 )
              {
                ydiff -= labelY * descentRatio;
              }
              else if ( valiString.compare( "Half", Qt::CaseInsensitive ) == 0 )
              {
                ydiff -= labelY * descentRatio;
                ydiff -= labelY * 0.5 * ( 1 - descentRatio );
              }
            }
          }
        }

        //data defined rotation?
        QMap< DataDefinedProperties, int >::const_iterator rotIt = dataDefinedProperties.find( QgsPalLayerSettings::Rotation );
        if ( rotIt != dataDefinedProperties.constEnd() )
        {
          dataDefinedRotation = true;
          angle = f.attributeMap().value( *rotIt ).toDouble() * M_PI / 180;
          //adjust xdiff and ydiff because the hali/vali point needs to be the rotation center
          double xd = xdiff * cos( angle ) - ydiff * sin( angle );
          double yd = xdiff * sin( angle ) + ydiff * cos( angle );
          xdiff = xd;
          ydiff = yd;
        }

        //project xPos and yPos from layer to map CRS
        double z = 0;
        if ( ct )
        {
          ct->transformInPlace( xPos, yPos, z );
        }

        yPos += ydiff;
        xPos += xdiff;

      }
    }
  }

  QgsPalGeometry* lbl = new QgsPalGeometry( f.id(), labelText, geos_geom_clone );

  // record the created geometry - it will be deleted at the end.
  geometries.append( lbl );

  //  feature to the layer
  try
  {
    if ( !palLayer->registerFeature( lbl->strId(), lbl, labelX, labelY, labelText.toUtf8().constData(),
                                     xPos, yPos, dataDefinedPosition, angle, dataDefinedRotation ) )
      return;
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( QString( "Ignoring feature %1 due PAL exception: " ).arg( f.id() ) + QString::fromLatin1( e.what() ) );
    return;
  }

  // TODO: only for placement which needs character info
  pal::Feature* feat = palLayer->getFeature( lbl->strId() );
  feat->setLabelInfo( lbl->info( fontMetrics, xform, rasterCompressFactor ) );

  // TODO: allow layer-wide feature dist in PAL...?

  //data defined label-feature distance?
  double distance = dist;
  QMap< DataDefinedProperties, int >::const_iterator dDistIt = dataDefinedProperties.find( QgsPalLayerSettings::LabelDistance );
  if ( dDistIt != dataDefinedProperties.constEnd() )
  {
    distance = f.attributeMap().value( *dDistIt ).toDouble();
  }

  if ( distance != 0 )
  {
    if ( distInMapUnits ) //convert distance from mm/map units to pixels
    {
      distance /= context.mapToPixel().mapUnitsPerPixel();
    }
    else //mm
    {
      distance *= vectorScaleFactor;
    }
    feat->setDistLabel( qAbs( ptOne.x() - ptZero.x() )* distance );
  }

  //add parameters for data defined labeling to QgsPalGeometry
  QMap< DataDefinedProperties, int >::const_iterator dIt = dataDefinedProperties.constBegin();
  for ( ; dIt != dataDefinedProperties.constEnd(); ++dIt )
  {
    lbl->addDataDefinedValue( dIt.key(), f.attributeMap()[dIt.value()] );
  }
}

int QgsPalLayerSettings::sizeToPixel( double size, const QgsRenderContext& c ) const
{
  double pixelSize;
  if ( fontSizeInMapUnits )
  {
    pixelSize = size / c.mapToPixel().mapUnitsPerPixel() * c.rasterScaleFactor();
  }
  else //font size in points
  {
    // set font size from points to output size
    pixelSize = 0.3527 * size * c.scaleFactor() * c.rasterScaleFactor();
  }
  return ( int )( pixelSize + 0.5 );
}


// -------------

QgsPalLabeling::QgsPalLabeling()
    : mMapRenderer( NULL ), mPal( NULL )
{

  // find out engine defaults
  Pal p;
  mCandPoint = p.getPointP();
  mCandLine = p.getLineP();
  mCandPolygon = p.getPolyP();

  switch ( p.getSearch() )
  {
    case CHAIN: mSearch = Chain; break;
    case POPMUSIC_TABU: mSearch = Popmusic_Tabu; break;
    case POPMUSIC_CHAIN: mSearch = Popmusic_Chain; break;
    case POPMUSIC_TABU_CHAIN: mSearch = Popmusic_Tabu_Chain; break;
    case FALP: mSearch = Falp; break;
  }

  mShowingCandidates = false;
  mShowingAllLabels = false;

  mLabelSearchTree = new QgsLabelSearchTree();
}


QgsPalLabeling::~QgsPalLabeling()
{
  // make sure we've freed everything
  exit();
  delete mLabelSearchTree;
  mLabelSearchTree = NULL;
}


bool QgsPalLabeling::willUseLayer( QgsVectorLayer* layer )
{
  QgsPalLayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );
  return lyrTmp.enabled;
}

int QgsPalLabeling::prepareLayer( QgsVectorLayer* layer, QSet<int>& attrIndices, QgsRenderContext& ctx )
{
  QgsDebugMsg("PREPARE LAYER");
  Q_ASSERT( mMapRenderer != NULL );

  // start with a temporary settings class, find out labeling info
  QgsPalLayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );

  if ( !lyrTmp.enabled )
    return 0;


  int fldIndex ;
  if(lyrTmp.isExpression)
  {
      if (lyrTmp.fieldName.isEmpty())
          return 0;
      QgsSearchString searchString;
      searchString.setString( lyrTmp.fieldName );
      searchString.tree()->referencedColumns();
      foreach(QString name, searchString.tree()->referencedColumns() )
      {
          fldIndex =  layer->fieldNameIndex( name );
          attrIndices.insert(fldIndex);
      }
  }
  else
  {
      // If we aren't an expression, we check to see if we can find the column.
      fldIndex = layer->fieldNameIndex( lyrTmp.fieldName );
      if ( fldIndex == -1)
          return 0;
      attrIndices.insert( fldIndex );
  }

  //add indices of data defined fields
  QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator dIt = lyrTmp.dataDefinedProperties.constBegin();
  for ( ; dIt != lyrTmp.dataDefinedProperties.constEnd(); ++dIt )
  {
    attrIndices.insert( dIt.value() );
  }

  // add layer settings to the pallabeling hashtable: <QgsVectorLayer*, QgsPalLayerSettings>
  mActiveLayers.insert( layer, lyrTmp );
  // start using the reference to the layer in hashtable instead of local instance
  QgsPalLayerSettings& lyr = mActiveLayers[layer];

  // how to place the labels
  Arrangement arrangement;
  switch ( lyr.placement )
  {
    case QgsPalLayerSettings::AroundPoint: arrangement = P_POINT; break;
    case QgsPalLayerSettings::OverPoint:   arrangement = P_POINT_OVER; break;
    case QgsPalLayerSettings::Line:        arrangement = P_LINE; break;
    case QgsPalLayerSettings::Curved:      arrangement = P_CURVED; break;
    case QgsPalLayerSettings::Horizontal:  arrangement = P_HORIZ; break;
    case QgsPalLayerSettings::Free:        arrangement = P_FREE; break;
    default: Q_ASSERT( "unsupported placement" && 0 ); return 0;
  }

  // create the pal layer
  double priority = 1 - lyr.priority / 10.0; // convert 0..10 --> 1..0
  double min_scale = -1, max_scale = -1;
  if ( lyr.scaleMin != 0 && lyr.scaleMax != 0 )
  {
    min_scale = lyr.scaleMin;
    max_scale = lyr.scaleMax;
  }

  Layer* l = mPal->addLayer( layer->id().toUtf8().data(),
                             min_scale, max_scale, arrangement,
                             METER, priority, lyr.obstacle, true, true );

  if ( lyr.placementFlags )
    l->setArrangementFlags( lyr.placementFlags );

  // set label mode (label per feature is the default)
  l->setLabelMode( lyr.labelPerPart ? Layer::LabelPerFeaturePart : Layer::LabelPerFeature );

  // set whether adjacent lines should be merged
  l->setMergeConnectedLines( lyr.mergeLines );

  lyr.textFont.setPixelSize( lyr.sizeToPixel( lyr.textFont.pointSizeF(), ctx ) );

  //raster and vector scale factors
  lyr.vectorScaleFactor = ctx.scaleFactor();
  lyr.rasterCompressFactor = ctx.rasterScaleFactor();

  // save the pal layer to our layer context (with some additional info)
  lyr.palLayer = l;
  lyr.fieldIndex = fldIndex;
  lyr.fontMetrics = new QFontMetricsF( lyr.textFont );

  lyr.xform = mMapRenderer->coordinateTransform();
  if ( mMapRenderer->hasCrsTransformEnabled() )
    lyr.ct = new QgsCoordinateTransform( layer->crs(), mMapRenderer->destinationCrs() );
  else
    lyr.ct = NULL;
  lyr.ptZero = lyr.xform->toMapCoordinates( 0, 0 );
  lyr.ptOne = lyr.xform->toMapCoordinates( 1, 0 );

  // rect for clipping
  lyr.extentGeom = QgsGeometry::fromRect( mMapRenderer->extent() );

  return 1; // init successful
}

int QgsPalLabeling::addDiagramLayer( QgsVectorLayer* layer, QgsDiagramLayerSettings *s )
{
  Layer* l = mPal->addLayer( layer->id().append( "d" ).toUtf8().data(), -1, -1, pal::Arrangement( s->placement ), METER, s->priority, s->obstacle, true, true );
  l->setArrangementFlags( s->placementFlags );

  s->palLayer = l;
  if ( mMapRenderer->hasCrsTransformEnabled() )
    s->ct = new QgsCoordinateTransform( layer->crs(), mMapRenderer->destinationCrs() );
  else
    s->ct = NULL;
  s->xform = mMapRenderer->coordinateTransform();
  mActiveDiagramLayers.insert( layer, *s );
  return 1;
}

void QgsPalLabeling::registerFeature( QgsVectorLayer* layer, QgsFeature& f, const QgsRenderContext& context )
{
  QgsPalLayerSettings& lyr = mActiveLayers[layer];
  lyr.registerFeature(layer, f, context );
}

void QgsPalLabeling::registerDiagramFeature( QgsVectorLayer* layer, QgsFeature& feat, const QgsRenderContext& context )
{
  //get diagram layer settings, diagram renderer
  QHash<QgsVectorLayer*, QgsDiagramLayerSettings>::iterator layerIt = mActiveDiagramLayers.find( layer );
  if ( layerIt == mActiveDiagramLayers.constEnd() )
  {
    return;
  }

  //convert geom to geos
  QgsGeometry* geom = feat.geometry();

  if ( layerIt.value().ct && !willUseLayer( layer ) ) // reproject the geometry if feature not already transformed for labeling
  {
    geom->transform( *( layerIt.value().ct ) );
  }

  GEOSGeometry* geos_geom = geom->asGeos();
  if ( geos_geom == 0 )
  {
    return; // invalid geometry
  }

  //create PALGeometry with diagram = true
  QgsPalGeometry* lbl = new QgsPalGeometry( feat.id(), "", GEOSGeom_clone( geos_geom ) );
  lbl->setIsDiagram( true );

  // record the created geometry - it will be deleted at the end.
  layerIt.value().geometries.append( lbl );

  double diagramWidth = 0;
  double diagramHeight = 0;
  QgsDiagramRendererV2* dr = layerIt.value().renderer;
  if ( dr )
  {
    QSizeF diagSize = dr->sizeMapUnits( feat.attributeMap(), context );
    if ( diagSize.isValid() )
    {
      diagramWidth = diagSize.width();
      diagramHeight = diagSize.height();
    }

    //append the diagram attributes to lbl
    QList<int> diagramAttrib = dr->diagramAttributes();
    QList<int>::const_iterator diagAttIt = diagramAttrib.constBegin();
    for ( ; diagAttIt != diagramAttrib.constEnd(); ++diagAttIt )
    {
      lbl->addDiagramAttribute( *diagAttIt, feat.attributeMap()[*diagAttIt] );
    }
  }

  //  feature to the layer
  int ddColX = layerIt.value().xPosColumn;
  int ddColY = layerIt.value().yPosColumn;
  double ddPosX = 0.0;
  double ddPosY = 0.0;
  bool ddPos = ( ddColX >= 0 && ddColY >= 0 );
  if ( ddPos )
  {
    bool posXOk, posYOk;
    //data defined diagram position is always centered
    ddPosX = feat.attributeMap()[ddColX].toDouble( &posXOk ) - diagramWidth / 2.0;
    ddPosY = feat.attributeMap()[ddColY].toDouble( &posYOk ) - diagramHeight / 2.0;
    if ( !posXOk || !posYOk )
    {
      ddPos = false;
    }
    else
    {
      const QgsCoordinateTransform* ct = layerIt.value().ct;
      if ( ct )
      {
        double z = 0;
        ct->transformInPlace( ddPosX, ddPosY, z );
      }
    }
  }

  try
  {
    if ( !layerIt.value().palLayer->registerFeature( lbl->strId(), lbl, diagramWidth, diagramHeight, "", ddPosX, ddPosY, ddPos ) )
    {
      return;
    }
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( QString( "Ignoring feature %1 due PAL exception: " ).arg( feat.id() ) + QString::fromLatin1( e.what() ) );
    return;
  }

  pal::Feature* palFeat = layerIt.value().palLayer->getFeature( lbl->strId() );
  QgsPoint ptZero = layerIt.value().xform->toMapCoordinates( 0, 0 );
  QgsPoint ptOne = layerIt.value().xform->toMapCoordinates( 1, 0 );
  palFeat->setDistLabel( qAbs( ptOne.x() - ptZero.x() ) * layerIt.value().dist );
}


void QgsPalLabeling::init( QgsMapRenderer* mr )
{
  mMapRenderer = mr;

  // delete if exists already
  if ( mPal )
    delete mPal;

  mPal = new Pal;

  SearchMethod s;
  switch ( mSearch )
  {
    default:
    case Chain: s = CHAIN; break;
    case Popmusic_Tabu: s = POPMUSIC_TABU; break;
    case Popmusic_Chain: s = POPMUSIC_CHAIN; break;
    case Popmusic_Tabu_Chain: s = POPMUSIC_TABU_CHAIN; break;
    case Falp: s = FALP; break;
  }
  mPal->setSearch( s );

  // set number of candidates generated per feature
  mPal->setPointP( mCandPoint );
  mPal->setLineP( mCandLine );
  mPal->setPolyP( mCandPolygon );

  mActiveLayers.clear();
  mActiveDiagramLayers.clear();
}

void QgsPalLabeling::exit()
{
  delete mPal;
  mPal = NULL;
  mMapRenderer = NULL;
}

QgsPalLayerSettings& QgsPalLabeling::layer( const QString& layerName )
{
  QHash<QgsVectorLayer*, QgsPalLayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    if ( lit.key() && lit.key()->id() == layerName )
    {
      return lit.value();
    }
  }
  return mInvalidLayerSettings;
}


void QgsPalLabeling::drawLabeling( QgsRenderContext& context )
{
  Q_ASSERT( mMapRenderer != NULL );
  QPainter* painter = context.painter();
  QgsRectangle extent = context.extent();

  if ( mLabelSearchTree )
  {
    mLabelSearchTree->clear();
  }

  QTime t;
  t.start();

  // do the labeling itself
  double scale = mMapRenderer->scale(); // scale denominator
  QgsRectangle r = extent;
  double bbox[] = { r.xMinimum(), r.yMinimum(), r.xMaximum(), r.yMaximum() };

  std::list<LabelPosition*>* labels;
  pal::Problem* problem;
  try
  {
    problem = mPal->extractProblem( scale, bbox );
  }
  catch ( std::exception& e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "PAL EXCEPTION :-( " + QString::fromLatin1( e.what() ) );
    //mActiveLayers.clear(); // clean up
    return;
  }

  const QgsMapToPixel* xform = mMapRenderer->coordinateTransform();

  // draw rectangles with all candidates
  // this is done before actual solution of the problem
  // before number of candidates gets reduced
  mCandidates.clear();
  if ( mShowingCandidates && problem )
  {
    painter->setPen( QColor( 0, 0, 0, 64 ) );
    painter->setBrush( Qt::NoBrush );
    for ( int i = 0; i < problem->getNumFeatures(); i++ )
    {
      for ( int j = 0; j < problem->getFeatureCandidateCount( i ); j++ )
      {
        pal::LabelPosition* lp = problem->getFeatureCandidate( i, j );

        drawLabelCandidateRect( lp, painter, xform );
      }
    }
  }

  // find the solution
  labels = mPal->solveProblem( problem, mShowingAllLabels );

  QgsDebugMsg( QString( "LABELING work:  %1 ms ... labels# %2" ).arg( t.elapsed() ).arg( labels->size() ) );
  t.restart();

  painter->setRenderHint( QPainter::Antialiasing );

  // draw the labels
  std::list<LabelPosition*>::iterator it = labels->begin();
  for ( ; it != labels->end(); ++it )
  {
    QgsPalGeometry* palGeometry = dynamic_cast< QgsPalGeometry* >(( *it )->getFeaturePart()->getUserGeometry() );
    if ( !palGeometry )
    {
      continue;
    }

    //layer names
    QString layerNameUtf8 = QString::fromUtf8(( *it )->getLayerName() );
    if ( palGeometry->isDiagram() )
    {
      //render diagram
      QHash<QgsVectorLayer*, QgsDiagramLayerSettings>::iterator dit = mActiveDiagramLayers.begin();
      for ( dit = mActiveDiagramLayers.begin(); dit != mActiveDiagramLayers.end(); ++dit )
      {
        if ( dit.key() && dit.key()->id().append( "d" ) == layerNameUtf8 )
        {
          QgsPoint outPt = xform->transform(( *it )->getX(), ( *it )->getY() );
          dit.value().renderer->renderDiagram( palGeometry->diagramAttributes(), context, QPointF( outPt.x(), outPt.y() ) );
        }
      }

      //insert into label search tree to manipulate position interactively
      if ( mLabelSearchTree )
      {
        //for diagrams, remove the additional 'd' at the end of the layer id
        QString layerId = layerNameUtf8;
        layerId.chop( 1 );
        mLabelSearchTree->insertLabel( *it,  QString( palGeometry->strId() ).toInt(), layerId, true );
      }
      continue;
    }

    const QgsPalLayerSettings& lyr = layer( layerNameUtf8 );
    QFont fontForLabel = lyr.textFont;
    QColor fontColor = lyr.textColor;
    double bufferSize = lyr.bufferSize;
    QColor bufferColor = lyr.bufferColor;

    //apply data defined settings for the label
    //font size
    QVariant dataDefinedSize = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Size );
    if ( dataDefinedSize.isValid() )
    {
      fontForLabel.setPixelSize( lyr.sizeToPixel( dataDefinedSize.toDouble(), context ) );
    }
    //font color
    QVariant dataDefinedColor = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Color );
    if ( dataDefinedColor.isValid() )
    {
      fontColor.setNamedColor( dataDefinedColor.toString() );
      if ( !fontColor.isValid() )
      {
        fontColor = lyr.textColor;
      }
    }
    //font bold
    QVariant dataDefinedBold = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Bold );
    if ( dataDefinedBold.isValid() )
    {
      fontForLabel.setBold(( bool )dataDefinedBold.toInt() );
    }
    //font italic
    QVariant dataDefinedItalic = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Italic );
    if ( dataDefinedItalic.isValid() )
    {
      fontForLabel.setItalic(( bool ) dataDefinedItalic.toInt() );
    }
    //font underline
    QVariant dataDefinedUnderline = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Underline );
    if ( dataDefinedUnderline.isValid() )
    {
      fontForLabel.setUnderline(( bool ) dataDefinedUnderline.toInt() );
    }
    //font strikeout
    QVariant dataDefinedStrikeout = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Strikeout );
    if ( dataDefinedStrikeout.isValid() )
    {
      fontForLabel.setStrikeOut(( bool ) dataDefinedStrikeout.toInt() );
    }
    //font family
    QVariant dataDefinedFontFamily = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Family );
    if ( dataDefinedFontFamily.isValid() )
    {
      fontForLabel.setFamily( dataDefinedFontFamily.toString() );
    }
    //buffer size
    QVariant dataDefinedBufferSize = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::BufferSize );
    if ( dataDefinedBufferSize.isValid() )
    {
      bufferSize = dataDefinedBufferSize.toDouble();
    }

    //buffer color
    QVariant dataDefinedBufferColor = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::BufferColor );
    if ( dataDefinedBufferColor.isValid() )
    {
      bufferColor.setNamedColor( dataDefinedBufferColor.toString() );
      if ( !bufferColor.isValid() )
      {
        bufferColor = lyr.bufferColor;
      }
    }

    if ( lyr.bufferSize != 0 )
      drawLabel( *it, painter, fontForLabel, fontColor, xform, bufferSize, bufferColor, true );

    drawLabel( *it, painter, fontForLabel, fontColor, xform );

    if ( mLabelSearchTree )
    {
      mLabelSearchTree->insertLabel( *it,  QString( palGeometry->strId() ).toInt(), ( *it )->getLayerName() );
    }
  }

  QgsDebugMsg( QString( "LABELING draw:  %1 ms" ).arg( t.elapsed() ) );

  delete problem;
  delete labels;

  // delete all allocated geometries for features
  QHash<QgsVectorLayer*, QgsPalLayerSettings>::iterator lit;
  for ( lit = mActiveLayers.begin(); lit != mActiveLayers.end(); ++lit )
  {
    QgsPalLayerSettings& lyr = lit.value();
    for ( QList<QgsPalGeometry*>::iterator git = lyr.geometries.begin(); git != lyr.geometries.end(); ++git )
      delete *git;
    lyr.geometries.clear();
  }

  //delete all allocated geometries for diagrams
  QHash<QgsVectorLayer*, QgsDiagramLayerSettings>::iterator dIt = mActiveDiagramLayers.begin();
  for ( ; dIt != mActiveDiagramLayers.end(); ++dIt )
  {
    QgsDiagramLayerSettings& dls = dIt.value();
    for ( QList<QgsPalGeometry*>::iterator git = dls.geometries.begin(); git != dls.geometries.end(); ++git )
    {
      delete *git;
    }
    dls.geometries.clear();
  }
}

QList<QgsLabelPosition> QgsPalLabeling::labelsAtPosition( const QgsPoint& p )
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition*> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->label( p, positionPointers );
    QList<QgsLabelPosition*>::const_iterator pointerIt = positionPointers.constBegin();
    for ( ; pointerIt != positionPointers.constEnd(); ++pointerIt )
    {
      positions.push_back( QgsLabelPosition( **pointerIt ) );
    }
  }

  return positions;
}

void QgsPalLabeling::numCandidatePositions( int& candPoint, int& candLine, int& candPolygon )
{
  candPoint = mCandPoint;
  candLine = mCandLine;
  candPolygon = mCandPolygon;
}

void QgsPalLabeling::setNumCandidatePositions( int candPoint, int candLine, int candPolygon )
{
  mCandPoint = candPoint;
  mCandLine = candLine;
  mCandPolygon = candPolygon;
}

void QgsPalLabeling::setSearchMethod( QgsPalLabeling::Search s )
{
  mSearch = s;
}

QgsPalLabeling::Search QgsPalLabeling::searchMethod() const
{
  return mSearch;
}

void QgsPalLabeling::drawLabelCandidateRect( pal::LabelPosition* lp, QPainter* painter, const QgsMapToPixel* xform )
{
  QgsPoint outPt = xform->transform( lp->getX(), lp->getY() );
  QgsPoint outPt2 = xform->transform( lp->getX() + lp->getWidth(), lp->getY() + lp->getHeight() );

  painter->save();
  painter->translate( QPointF( outPt.x(), outPt.y() ) );
  painter->rotate( -lp->getAlpha() * 180 / M_PI );
  QRectF rect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );
  painter->drawRect( rect );
  painter->restore();

  // save the rect
  rect.moveTo( outPt.x(), outPt.y() );
  mCandidates.append( QgsLabelCandidate( rect, lp->getCost() * 1000 ) );

  // show all parts of the multipart label
  if ( lp->getNextPart() )
    drawLabelCandidateRect( lp->getNextPart(), painter, xform );
}

void QgsPalLabeling::drawLabel( pal::LabelPosition* label, QPainter* painter, const QFont& f, const QColor& c, const QgsMapToPixel* xform, double bufferSize,
                                const QColor& bufferColor, bool drawBuffer )
{
  QgsPoint outPt = xform->transform( label->getX(), label->getY() );

  // TODO: optimize access :)
  const QgsPalLayerSettings& lyr = layer( QString::fromUtf8( label->getLayerName() ) );
  QString text = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->text();
  QString txt = ( label->getPartId() == -1 ? text : QString( text[label->getPartId()] ) );

  //add the direction symbol if needed
  if ( !txt.isEmpty() && lyr.placement == QgsPalLayerSettings::Line &&
       lyr.addDirectionSymbol && !lyr.multiLineLabels )
  {
    if ( label->getReversed() )
    {
      txt.prepend( "<" );
    }
    else
    {
      txt.append( ">" );
    }
  }

  //QgsDebugMsg( "drawLabel " + QString::number( drawBuffer ) + " " + txt );

  QStringList multiLineList;
  if ( lyr.multiLineLabels )
  {
    multiLineList = txt.split( "\n" );
  }
  else
  {
    multiLineList << txt;
  }

  for ( int i = 0; i < multiLineList.size(); ++i )
  {
    painter->save();
    painter->translate( QPointF( outPt.x(), outPt.y() ) );
    painter->rotate( -label->getAlpha() * 180 / M_PI );

    // scale down painter: the font size has been multiplied by raster scale factor
    // to workaround a Qt font scaling bug with small font sizes
    painter->scale( 1.0 / lyr.rasterCompressFactor, 1.0 / lyr.rasterCompressFactor );

    double yMultiLineOffset = ( multiLineList.size() - 1 - i ) * lyr.fontMetrics->height();
    painter->translate( QPointF( 0, - lyr.fontMetrics->descent() - yMultiLineOffset ) );

    if ( drawBuffer )
    {
      // we're drawing buffer
      drawLabelBuffer( painter, multiLineList.at( i ), f, bufferSize * lyr.vectorScaleFactor * lyr.rasterCompressFactor , bufferColor );
    }
    else
    {
      // we're drawing real label
      QPainterPath path;
      path.addText( 0, 0, f, multiLineList.at( i ) );
      painter->setPen( Qt::NoPen );
      painter->setBrush( c );
      painter->drawPath( path );
    }
    painter->restore();

    if ( label->getNextPart() )
      drawLabel( label->getNextPart(), painter, f, c, xform, bufferSize, bufferColor, drawBuffer );
  }
}


void QgsPalLabeling::drawLabelBuffer( QPainter* p, QString text, const QFont& font, double size, QColor color )
{
  QPainterPath path;
  path.addText( 0, 0, font, text );
  QPen pen( color );
  pen.setWidthF( size );
  p->setPen( pen );
  p->setBrush( color );
  p->drawPath( path );
}

QgsLabelingEngineInterface* QgsPalLabeling::clone()
{
  QgsPalLabeling* lbl = new QgsPalLabeling();
  lbl->mShowingAllLabels = mShowingAllLabels;
  lbl->mShowingCandidates = mShowingCandidates;
  return lbl;
}
