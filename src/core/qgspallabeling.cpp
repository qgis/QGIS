/***************************************************************************
  qgspallabeling.cpp
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder dot sk at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspallabeling.h"

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

#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QFontMetrics>
#include <QTime>
#include <QPainter>

#include "diagram/qgsdiagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgslabelsearchtree.h"
#include "qgsexpression.h"

#include <qgslogger.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmaprenderer.h>
#include <qgsmarkersymbollayerv2.h>
#include <qgsproject.h>
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include <QMessageBox>

using namespace pal;


class QgsPalGeometry : public PalGeometry
{
  public:
    QgsPalGeometry( QgsFeatureId id, QString text, GEOSGeometry* g,
                    qreal ltrSpacing = 0.0, qreal wordSpacing = 0.0, bool curvedLabeling = false )
        : mG( g )
        , mText( text )
        , mId( id )
        , mInfo( NULL )
        , mIsDiagram( false )
        , mIsPinned( false )
        , mFontMetrics( NULL )
        , mLetterSpacing( ltrSpacing )
        , mWordSpacing( wordSpacing )
        , mCurvedLabeling( curvedLabeling )
    {
      mStrId = FID_TO_STRING( id ).toAscii();
    }

    ~QgsPalGeometry()
    {
      if ( mG )
        GEOSGeom_destroy( mG );
      delete mInfo;
      delete mFontMetrics;
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

    pal::LabelInfo* info( QFontMetricsF* fm, const QgsMapToPixel* xform, double fontScale, double maxinangle, double maxoutangle )
    {
      if ( mInfo )
        return mInfo;

      mFontMetrics = new QFontMetricsF( *fm ); // duplicate metrics for when drawing label

      // max angle between curved label characters (20.0/-20.0 was default in QGIS <= 1.8)
      if ( maxinangle < 20.0 )
        maxinangle = 20.0;
      if ( 60.0 < maxinangle )
        maxinangle = 60.0;
      if ( maxoutangle > -20.0 )
        maxoutangle = -20.0;
      if ( -95.0 > maxoutangle )
        maxoutangle = -95.0;

      // create label info!
      QgsPoint ptZero = xform->toMapCoordinates( 0, 0 );
      QgsPoint ptSize = xform->toMapCoordinatesF( 0.0, -fm->height() / fontScale );

      // mLetterSpacing/mWordSpacing = 0.0 is default for non-curved labels
      // (non-curved spacings handled by Qt in QgsPalLayerSettings/QgsPalLabeling)
      qreal charWidth;
      qreal wordSpaceFix;
      mInfo = new pal::LabelInfo( mText.count(), ptSize.y() - ptZero.y(), maxinangle, maxoutangle );
      for ( int i = 0; i < mText.count(); i++ )
      {
        mInfo->char_info[i].chr = mText[i].unicode();

        // reconstruct how Qt creates word spacing, then adjust per individual stored character
        // this will allow PAL to create each candidate width = character width + correct spacing
        charWidth = fm->width( mText[i] );
        if ( mCurvedLabeling )
        {
          wordSpaceFix = qreal( 0.0 );
          if ( mText[i] == QString( " " )[0] )
          {
            // word spacing only gets added once at end of consecutive run of spaces, see QTextEngine::shapeText()
            int nxt = i + 1;
            wordSpaceFix = ( nxt < mText.count() && mText[nxt] != QString( " " )[0] ) ? mWordSpacing : qreal( 0.0 );
          }
          if ( fm->width( QString( mText[i] ) ) - fm->width( mText[i] ) - mLetterSpacing != qreal( 0.0 ) )
          {
            // word spacing applied when it shouldn't be
            wordSpaceFix -= mWordSpacing;
          }
          charWidth = fm->width( QString( mText[i] ) ) + wordSpaceFix;
        }

        ptSize = xform->toMapCoordinatesF((( double ) charWidth ) / fontScale , 0.0 );
        mInfo->char_info[i].width = ptSize.x() - ptZero.x();
      }
      return mInfo;
    }

    const QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant >& dataDefinedValues() const { return mDataDefinedValues; }
    void addDataDefinedValue( QgsPalLayerSettings::DataDefinedProperties p, QVariant v ) { mDataDefinedValues.insert( p, v ); }

    void setIsDiagram( bool d ) { mIsDiagram = d; }
    bool isDiagram() const { return mIsDiagram; }

    void setIsPinned( bool f ) { mIsPinned = f; }
    bool isPinned() const { return mIsPinned; }

    QFontMetricsF* getLabelFontMetrics() { return mFontMetrics; }

    void setDiagramAttributes( const QgsAttributes& attrs ) { mDiagramAttributes = attrs; }
    const QgsAttributes& diagramAttributes() { return mDiagramAttributes; }

  protected:
    GEOSGeometry* mG;
    QString mText;
    QByteArray mStrId;
    QgsFeatureId mId;
    LabelInfo* mInfo;
    bool mIsDiagram;
    bool mIsPinned;
    QFontMetricsF* mFontMetrics;
    qreal mLetterSpacing; // for use with curved labels
    qreal mWordSpacing; // for use with curved labels
    bool mCurvedLabeling; // whether the geometry is to be used for curved labeling placement
    /**Stores attribute values for data defined properties*/
    QMap< QgsPalLayerSettings::DataDefinedProperties, QVariant > mDataDefinedValues;

    /**Stores attribute values for diagram rendering*/
    QgsAttributes mDiagramAttributes;
};

// -------------

QgsPalLayerSettings::QgsPalLayerSettings()
    : palLayer( NULL )
    , ct( NULL )
    , extentGeom( NULL )
    , mFeaturesToLabel( 0 )
    , mFeatsSendingToPal( 0 )
    , mFeatsRegPal( 0 )
    , expression( NULL )
{
  placement = AroundPoint;
  placementFlags = 0;
  xQuadOffset = 0;
  yQuadOffset = 0;
  xOffset = 0;
  yOffset = 0;
  angleOffset = 0;
  centroidWhole = false;
  textFont = QApplication::font();
  textNamedStyle = QString( "" );
  textColor = Qt::black;
  textTransp = 0;
  blendMode = QPainter::CompositionMode_SourceOver;
  previewBkgrdColor = Qt::white;
  enabled = false;
  priority = 5;
  obstacle = true;
  dist = 0;
  scaleMin = 0;
  scaleMax = 0;
  bufferSize = 1;
  bufferColor = Qt::white;
  bufferTransp = 0;
  bufferBlendMode = QPainter::CompositionMode_SourceOver;
  bufferNoFill = false;
  bufferJoinStyle = Qt::BevelJoin;
  formatNumbers = false;
  decimals = 3;
  plusSign = false;
  labelPerPart = false;
  displayAll = false;
  mergeLines = false;
  minFeatureSize = 0.0;
  limitNumLabels = false;
  maxNumLabels = 2000;
  vectorScaleFactor = 1.0;
  rasterCompressFactor = 1.0;
  addDirectionSymbol = false;
  leftDirectionSymbol = QString( "<" );
  rightDirectionSymbol = QString( ">" );
  reverseDirectionSymbol = false;
  placeDirectionSymbol = SymbolLeftRight;
  upsidedownLabels = Upright;
  maxCurvedCharAngleIn = 20.0;
  maxCurvedCharAngleOut = -20.0;
  fontSizeInMapUnits = false;
  fontLimitPixelSize = false;
  fontMinPixelSize = 0; //trigger to turn it on by default for map unit labels
  fontMaxPixelSize = 10000;
  bufferSizeInMapUnits = false;
  labelOffsetInMapUnits = true;
  distInMapUnits = false;
  wrapChar = "";
  multilineHeight = 1.0;
  multilineAlign = MultiLeft;
  preserveRotation = true;

  // shape background
  shapeDraw = false;
  shapeType = ShapeRectangle;
  shapeSVGFile = QString();
  shapeSizeType = SizeBuffer;
  shapeSize = QPointF( 0.0, 0.0 );
  shapeSizeUnits = MM;
  shapeRotationType = RotationSync;
  shapeRotation = 0.0;
  shapeOffset = QPointF( 0.0, 0.0 );
  shapeOffsetUnits = MM;
  shapeRadii = QPointF( 0.0, 0.0 );
  shapeRadiiUnits = MM;
  shapeFillColor = Qt::white;
  shapeBorderColor = Qt::darkGray;
  shapeBorderWidth = 0.0;
  shapeBorderWidthUnits = MM;
  shapeJoinStyle = Qt::BevelJoin;
  shapeTransparency = 0;
  shapeBlendMode = QPainter::CompositionMode_SourceOver;

  // drop shadow
  shadowDraw = false;
  shadowUnder = ShadowLowest;
  shadowOffsetAngle = 135;
  shadowOffsetDist = 0.0;
  shadowOffsetUnits = MM;
  shadowOffsetGlobal = true;
  shadowRadius = 0.0;
  shadowRadiusUnits = MM;
  shadowRadiusAlphaOnly = false;
  shadowTransparency = 0;
  shadowScale = 100;
  shadowColor = Qt::black;
  shadowBlendMode = QPainter::CompositionMode_Multiply;

  // data defined string values
  mDataDefinedNames << "Size";
  mDataDefinedNames << "Bold";
  mDataDefinedNames << "Italic";
  mDataDefinedNames << "Underline";
  mDataDefinedNames << "Color";
  mDataDefinedNames << "Strikeout";
  mDataDefinedNames << "Family";
  mDataDefinedNames << "BufferSize";
  mDataDefinedNames << "BufferColor";
  mDataDefinedNames << "PositionX";
  mDataDefinedNames << "PositionY";
  mDataDefinedNames << "Hali";
  mDataDefinedNames << "Vali";
  mDataDefinedNames << "LabelDistance";
  mDataDefinedNames << "Rotation";
  mDataDefinedNames << "Show";
  mDataDefinedNames << "MinScale";
  mDataDefinedNames << "MaxScale";
  mDataDefinedNames << "FontTransp";
  mDataDefinedNames << "BufferTransp";
  mDataDefinedNames << "AlwaysShow";

  // temp stuff for when drawing label components (don't copy)
  showingShadowRects = false;
}

QgsPalLayerSettings::QgsPalLayerSettings( const QgsPalLayerSettings& s )
{
  // copy only permanent stuff
  fieldName = s.fieldName;
  isExpression = s.isExpression;
  placement = s.placement;
  placementFlags = s.placementFlags;
  xQuadOffset = s.xQuadOffset;
  yQuadOffset = s.yQuadOffset;
  xOffset = s.xOffset;
  yOffset = s.yOffset;
  angleOffset = s.angleOffset;
  centroidWhole = s.centroidWhole;
  textFont = s.textFont;
  textNamedStyle = s.textNamedStyle;
  textColor = s.textColor;
  textTransp = s.textTransp;
  blendMode = s.blendMode;
  previewBkgrdColor = s.previewBkgrdColor;
  enabled = s.enabled;
  priority = s.priority;
  obstacle = s.obstacle;
  dist = s.dist;
  scaleMin = s.scaleMin;
  scaleMax = s.scaleMax;
  bufferSize = s.bufferSize;
  bufferColor = s.bufferColor;
  bufferTransp = s.bufferTransp;
  bufferBlendMode = s.bufferBlendMode;
  bufferJoinStyle = s.bufferJoinStyle;
  bufferNoFill = s.bufferNoFill;
  formatNumbers = s.formatNumbers;
  decimals = s.decimals;
  plusSign = s.plusSign;
  labelPerPart = s.labelPerPart;
  displayAll = s.displayAll;
  mergeLines = s.mergeLines;
  minFeatureSize = s.minFeatureSize;
  limitNumLabels = s.limitNumLabels;
  maxNumLabels = s.maxNumLabels;
  vectorScaleFactor = s.vectorScaleFactor;
  rasterCompressFactor = s.rasterCompressFactor;
  addDirectionSymbol = s.addDirectionSymbol;
  leftDirectionSymbol = s.leftDirectionSymbol;
  rightDirectionSymbol = s.rightDirectionSymbol;
  reverseDirectionSymbol = s.reverseDirectionSymbol;
  placeDirectionSymbol = s.placeDirectionSymbol;
  upsidedownLabels = s.upsidedownLabels;
  maxCurvedCharAngleIn = s.maxCurvedCharAngleIn;
  maxCurvedCharAngleOut = s.maxCurvedCharAngleOut;
  fontSizeInMapUnits = s.fontSizeInMapUnits;
  fontLimitPixelSize = s.fontLimitPixelSize;
  fontMinPixelSize = s.fontMinPixelSize;
  fontMaxPixelSize = s.fontMaxPixelSize;
  bufferSizeInMapUnits = s.bufferSizeInMapUnits;
  distInMapUnits = s.distInMapUnits;
  labelOffsetInMapUnits = s.labelOffsetInMapUnits;
  wrapChar = s.wrapChar;
  multilineHeight = s.multilineHeight;
  multilineAlign = s.multilineAlign;
  preserveRotation = s.preserveRotation;

  // shape background
  shapeDraw = s.shapeDraw;
  shapeType = s.shapeType;
  shapeSVGFile = s.shapeSVGFile;
  shapeSizeType = s.shapeSizeType;
  shapeSize = s.shapeSize;
  shapeSizeUnits = s.shapeSizeUnits;
  shapeRotationType = s.shapeRotationType;
  shapeRotation = s.shapeRotation;
  shapeOffset = s.shapeOffset;
  shapeOffsetUnits = s.shapeOffsetUnits;
  shapeRadii = s.shapeRadii;
  shapeRadiiUnits = s.shapeRadiiUnits;
  shapeFillColor = s.shapeFillColor;
  shapeBorderColor = s.shapeBorderColor;
  shapeBorderWidth = s.shapeBorderWidth;
  shapeBorderWidthUnits = s.shapeBorderWidthUnits;
  shapeJoinStyle = s.shapeJoinStyle;
  shapeTransparency = s.shapeTransparency;
  shapeBlendMode = s.shapeBlendMode;

  // drop shadow
  shadowDraw = s.shadowDraw;
  shadowUnder = s.shadowUnder;
  shadowOffsetAngle = s.shadowOffsetAngle;
  shadowOffsetDist = s.shadowOffsetDist;
  shadowOffsetUnits = s.shadowOffsetUnits;
  shadowOffsetGlobal = s.shadowOffsetGlobal;
  shadowRadius = s.shadowRadius;
  shadowRadiusUnits = s.shadowRadiusUnits;
  shadowRadiusAlphaOnly = s.shadowRadiusAlphaOnly;
  shadowTransparency = s.shadowTransparency;
  shadowScale = s.shadowScale;
  shadowColor = s.shadowColor;
  shadowBlendMode = s.shadowBlendMode;

  dataDefinedProperties = s.dataDefinedProperties;
  mDataDefinedNames = s.mDataDefinedNames;

  ct = NULL;
  extentGeom = NULL;
  expression = NULL;
}


QgsPalLayerSettings::~QgsPalLayerSettings()
{
  // pal layer is deleted internally in PAL

  delete ct;
  delete expression;
  delete extentGeom;
}

QgsExpression* QgsPalLayerSettings::getLabelExpression()
{
  if ( expression == NULL )
  {
    expression = new QgsExpression( fieldName );
  }
  return expression;
}

static QColor _readColor( QgsVectorLayer* layer, QString property, QColor defaultColor = Qt::black, bool withAlpha = true )
{
  int r = layer->customProperty( property + "R", QVariant( defaultColor.red() ) ).toInt();
  int g = layer->customProperty( property + "G", QVariant( defaultColor.green() ) ).toInt();
  int b = layer->customProperty( property + "B", QVariant( defaultColor.blue() ) ).toInt();
  int a = withAlpha ? layer->customProperty( property + "A", QVariant( defaultColor.alpha() ) ).toInt() : 255;
  return QColor( r, g, b, a );
}

static void _writeColor( QgsVectorLayer* layer, QString property, QColor color, bool withAlpha = true )
{
  layer->setCustomProperty( property + "R", color.red() );
  layer->setCustomProperty( property + "G", color.green() );
  layer->setCustomProperty( property + "B", color.blue() );
  if ( withAlpha )
    layer->setCustomProperty( property + "A", color.alpha() );
}

void QgsPalLayerSettings::readDataDefinedPropertyMap( QgsVectorLayer* layer,
    QMap < QgsPalLayerSettings::DataDefinedProperties, QString > & propertyMap )
{
  if ( !layer )
  {
    return;
  }

  for ( int i = 0; i < mDataDefinedNames.size() ; ++i )
  {
    readDataDefinedProperty( layer, ( QgsPalLayerSettings::DataDefinedProperties )i, propertyMap );
  }
}

void QgsPalLayerSettings::writeDataDefinedPropertyMap( QgsVectorLayer* layer,
    const QMap < QgsPalLayerSettings::DataDefinedProperties, QString > & propertyMap )
{
  if ( !layer )
  {
    return;
  }
  for ( int i = 0; i < mDataDefinedNames.size() ; ++i )
  {
    QString newPropertyName = "labeling/dataDefined/" + mDataDefinedNames.at( i );
    QString oldPropertyName = "labeling/dataDefinedProperty" + QString::number( i );

    QMap< QgsPalLayerSettings::DataDefinedProperties, QString >::const_iterator it = propertyMap.find(( QgsPalLayerSettings::DataDefinedProperties )i );
    QVariant propertyValue;
    if ( it == propertyMap.constEnd() )
    {
      propertyValue = QVariant(); //we cannot delete properties, so we just insert an invalid variant
    }
    else
    {
      propertyValue = *it;
    }
    layer->setCustomProperty( newPropertyName, propertyValue );

    if ( layer->customProperty( newPropertyName ).isValid() )
    {
      // remove old-style field index-based property, if still present
      layer->removeCustomProperty( oldPropertyName );
    }
  }
}

void QgsPalLayerSettings::readDataDefinedProperty( QgsVectorLayer* layer,
    QgsPalLayerSettings::DataDefinedProperties p,
    QMap < QgsPalLayerSettings::DataDefinedProperties, QString > & propertyMap )
{
  QString newPropertyName = "labeling/dataDefined/" + mDataDefinedNames.at( p );
  QString oldPropertyName = "labeling/dataDefinedProperty" + QString::number( p );

  QVariant newPropertyField = layer->customProperty( newPropertyName );
  QVariant oldPropertyField = layer->customProperty( oldPropertyName );

  // Fix to migrate from old-style vector api, where returned QMap keys possibly
  //   had 'holes' in sequence of field indices, e.g. 0,2,3
  // QgsAttrPalIndexNameHash provides a means of access field name in sequences from
  //   providers that procuded holes (e.g. PostGIS skipped geom column), otherwise it is empty
  QgsAttrPalIndexNameHash oldIndicesToNames = layer->dataProvider()->palAttributeIndexNames();

  QString name;
  if ( newPropertyField.isValid() )
  {
    name = newPropertyField.toString();
    if ( !name.isEmpty() )
    {
      propertyMap.insert( p, name );
    }
  }
  else if ( oldPropertyField.isValid() )
  {
    // switch from old-style field index- to name-based properties
    bool conversionOk;
    int indx = oldPropertyField.toInt( &conversionOk );

    if ( conversionOk )
    {
      if ( !oldIndicesToNames.isEmpty() )
      {
        name = oldIndicesToNames.value( indx );
      }
      else
      {
        QgsFields fields = layer->dataProvider()->fields();
        if ( indx < fields.size() ) // in case field count has changed
        {
          name = fields.at( indx ).name();
        }
      }
    }

    QVariant upgradeValue = QVariant();
    if ( !name.isEmpty() )
    {
      propertyMap.insert( p, name );
      upgradeValue = QVariant( name );
    }

    //upgrade property to field name-based
    layer->setCustomProperty( newPropertyName, upgradeValue );

    // remove old-style field index-based property
    layer->removeCustomProperty( oldPropertyName );
  }
}

void QgsPalLayerSettings::updateFontViaStyle( const QString & fontstyle )
{
  if ( !fontstyle.isEmpty() )
  {
    QFont styledfont = mFontDB.font( textFont.family(), fontstyle, 12 );
    styledfont.setPointSizeF( textFont.pointSizeF() );
    if ( QApplication::font().toString() != styledfont.toString() )
    {
      textFont = styledfont;
    }
  }
}

void QgsPalLayerSettings::readFromLayer( QgsVectorLayer* layer )
{
  if ( layer->customProperty( "labeling" ).toString() != QString( "pal" ) )
    return; // there's no information available

  // NOTE: set defaults for newly added properties, for backwards compatibility

  fieldName = layer->customProperty( "labeling/fieldName" ).toString();
  isExpression = layer->customProperty( "labeling/isExpression" ).toBool();
  placement = ( Placement )layer->customProperty( "labeling/placement" ).toInt();
  placementFlags = layer->customProperty( "labeling/placementFlags" ).toUInt();
  xQuadOffset = layer->customProperty( "labeling/xQuadOffset", QVariant( 0 ) ).toInt();
  yQuadOffset = layer->customProperty( "labeling/yQuadOffset", QVariant( 0 ) ).toInt();
  xOffset = layer->customProperty( "labeling/xOffset", QVariant( 0.0 ) ).toDouble();
  yOffset = layer->customProperty( "labeling/yOffset", QVariant( 0.0 ) ).toDouble();
  angleOffset = layer->customProperty( "labeling/angleOffset", QVariant( 0.0 ) ).toDouble();
  centroidWhole = layer->customProperty( "labeling/centroidWhole", QVariant( false ) ).toBool();
  QString fontFamily = layer->customProperty( "labeling/fontFamily", QVariant( QApplication::font().family() ) ).toString();
  double fontSize = layer->customProperty( "labeling/fontSize" ).toDouble();
  int fontWeight = layer->customProperty( "labeling/fontWeight" ).toInt();
  bool fontItalic = layer->customProperty( "labeling/fontItalic" ).toBool();
  textFont = QFont( fontFamily, fontSize, fontWeight, fontItalic );
  textFont.setPointSizeF( fontSize ); //double precision needed because of map units
  textNamedStyle = layer->customProperty( "labeling/namedStyle", QVariant( "" ) ).toString();
  updateFontViaStyle( textNamedStyle ); // must come after textFont.setPointSizeF()
  textFont.setCapitalization(( QFont::Capitalization )layer->customProperty( "labeling/fontCapitals", QVariant( 0 ) ).toUInt() );
  textFont.setUnderline( layer->customProperty( "labeling/fontUnderline" ).toBool() );
  textFont.setStrikeOut( layer->customProperty( "labeling/fontStrikeout" ).toBool() );
  textFont.setLetterSpacing( QFont::AbsoluteSpacing, layer->customProperty( "labeling/fontLetterSpacing", QVariant( 0.0 ) ).toDouble() );
  textFont.setWordSpacing( layer->customProperty( "labeling/fontWordSpacing", QVariant( 0.0 ) ).toDouble() );
  textColor = _readColor( layer, "labeling/textColor", Qt::black, false );
  textTransp = layer->customProperty( "labeling/textTransp" ).toInt();
  blendMode = QgsMapRenderer::getCompositionMode(
                ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/blendMode", QVariant( QgsMapRenderer::BlendNormal ) ).toUInt() );
  previewBkgrdColor = QColor( layer->customProperty( "labeling/previewBkgrdColor", QVariant( "#ffffff" ) ).toString() );
  enabled = layer->customProperty( "labeling/enabled" ).toBool();
  priority = layer->customProperty( "labeling/priority" ).toInt();
  obstacle = layer->customProperty( "labeling/obstacle" ).toBool();
  dist = layer->customProperty( "labeling/dist" ).toDouble();
  scaleMin = layer->customProperty( "labeling/scaleMin" ).toInt();
  scaleMax = layer->customProperty( "labeling/scaleMax" ).toInt();
  bufferSize = layer->customProperty( "labeling/bufferSize" ).toDouble();
  bufferColor = _readColor( layer, "labeling/bufferColor", Qt::white, false );
  bufferTransp = layer->customProperty( "labeling/bufferTransp" ).toInt();
  bufferBlendMode = QgsMapRenderer::getCompositionMode(
                      ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/bufferBlendMode", QVariant( QgsMapRenderer::BlendNormal ) ).toUInt() );
  bufferJoinStyle = ( Qt::PenJoinStyle )layer->customProperty( "labeling/bufferJoinStyle", QVariant( Qt::BevelJoin ) ).toUInt();
  bufferNoFill = layer->customProperty( "labeling/bufferNoFill", QVariant( false ) ).toBool();
  formatNumbers = layer->customProperty( "labeling/formatNumbers" ).toBool();
  decimals = layer->customProperty( "labeling/decimals" ).toInt();
  plusSign = layer->customProperty( "labeling/plussign" ).toInt();
  labelPerPart = layer->customProperty( "labeling/labelPerPart" ).toBool();
  displayAll = layer->customProperty( "labeling/displayAll", QVariant( false ) ).toBool();
  mergeLines = layer->customProperty( "labeling/mergeLines" ).toBool();
  addDirectionSymbol = layer->customProperty( "labeling/addDirectionSymbol" ).toBool();
  leftDirectionSymbol = layer->customProperty( "labeling/leftDirectionSymbol", QVariant( "<" ) ).toString();
  rightDirectionSymbol = layer->customProperty( "labeling/rightDirectionSymbol", QVariant( ">" ) ).toString();
  reverseDirectionSymbol = layer->customProperty( "labeling/reverseDirectionSymbol" ).toBool();
  placeDirectionSymbol = ( DirectionSymbols )layer->customProperty( "labeling/placeDirectionSymbol", QVariant( SymbolLeftRight ) ).toUInt();
  upsidedownLabels = ( UpsideDownLabels )layer->customProperty( "labeling/upsidedownLabels", QVariant( Upright ) ).toUInt();
  maxCurvedCharAngleIn = layer->customProperty( "labeling/maxCurvedCharAngleIn", QVariant( 20.0 ) ).toDouble();
  maxCurvedCharAngleOut = layer->customProperty( "labeling/maxCurvedCharAngleOut", QVariant( -20.0 ) ).toDouble();
  minFeatureSize = layer->customProperty( "labeling/minFeatureSize" ).toDouble();
  limitNumLabels = layer->customProperty( "labeling/limitNumLabels", QVariant( false ) ).toBool();
  maxNumLabels = layer->customProperty( "labeling/maxNumLabels", QVariant( 2000 ) ).toInt();
  fontSizeInMapUnits = layer->customProperty( "labeling/fontSizeInMapUnits" ).toBool();
  fontLimitPixelSize = layer->customProperty( "labeling/fontLimitPixelSize", QVariant( false ) ).toBool();
  fontMinPixelSize = layer->customProperty( "labeling/fontMinPixelSize", QVariant( 0 ) ).toInt();
  fontMaxPixelSize = layer->customProperty( "labeling/fontMaxPixelSize", QVariant( 10000 ) ).toInt();
  bufferSizeInMapUnits = layer->customProperty( "labeling/bufferSizeInMapUnits" ).toBool();
  distInMapUnits = layer->customProperty( "labeling/distInMapUnits" ).toBool();
  labelOffsetInMapUnits = layer->customProperty( "labeling/labelOffsetInMapUnits", QVariant( true ) ).toBool();
  wrapChar = layer->customProperty( "labeling/wrapChar" ).toString();
  multilineHeight = layer->customProperty( "labeling/multilineHeight", QVariant( 1.0 ) ).toDouble();
  multilineAlign = ( MultiLineAlign )layer->customProperty( "labeling/multilineAlign", QVariant( MultiLeft ) ).toUInt();
  preserveRotation = layer->customProperty( "labeling/preserveRotation", QVariant( true ) ).toBool();

  // shape background
  shapeDraw = layer->customProperty( "labeling/shapeDraw", QVariant( false ) ).toBool();
  shapeType = ( ShapeType )layer->customProperty( "labeling/shapeType", QVariant( ShapeRectangle ) ).toUInt();
  shapeSVGFile = layer->customProperty( "labeling/shapeSVGFile", QVariant( "" ) ).toString();
  shapeSizeType = ( SizeType )layer->customProperty( "labeling/shapeSizeType", QVariant( SizeBuffer ) ).toUInt();
  shapeSize = QPointF( layer->customProperty( "labeling/shapeSizeX", QVariant( 0.0 ) ).toDouble(),
                       layer->customProperty( "labeling/shapeSizeY", QVariant( 0.0 ) ).toDouble() );
  shapeSizeUnits = ( SizeUnit )layer->customProperty( "labeling/shapeSizeUnits", QVariant( MM ) ).toUInt();
  shapeRotationType = ( RotationType )layer->customProperty( "labeling/shapeRotationType", QVariant( RotationSync ) ).toUInt();
  shapeRotation = layer->customProperty( "labeling/shapeRotation", QVariant( 0.0 ) ).toDouble();
  shapeOffset = QPointF( layer->customProperty( "labeling/shapeOffsetX", QVariant( 0.0 ) ).toDouble(),
                         layer->customProperty( "labeling/shapeOffsetY", QVariant( 0.0 ) ).toDouble() );
  shapeOffsetUnits = ( SizeUnit )layer->customProperty( "labeling/shapeOffsetUnits", QVariant( MM ) ).toUInt();
  shapeRadii = QPointF( layer->customProperty( "labeling/shapeRadiiX", QVariant( 0.0 ) ).toDouble(),
                        layer->customProperty( "labeling/shapeRadiiY", QVariant( 0.0 ) ).toDouble() );
  shapeRadiiUnits = ( SizeUnit )layer->customProperty( "labeling/shapeRadiiUnits", QVariant( MM ) ).toUInt();
  shapeFillColor = _readColor( layer, "labeling/shapeFillColor", Qt::white, true );
  shapeBorderColor = _readColor( layer, "labeling/shapeBorderColor", Qt::darkGray, true );
  shapeBorderWidth = layer->customProperty( "labeling/shapeBorderWidth", QVariant( .0 ) ).toDouble();
  shapeBorderWidthUnits = ( SizeUnit )layer->customProperty( "labeling/shapeBorderWidthUnits", QVariant( MM ) ).toUInt();
  shapeJoinStyle = ( Qt::PenJoinStyle )layer->customProperty( "labeling/shapeJoinStyle", QVariant( Qt::BevelJoin ) ).toUInt();
  shapeTransparency = layer->customProperty( "labeling/shapeTransparency", QVariant( 0 ) ).toInt();
  shapeBlendMode = QgsMapRenderer::getCompositionMode(
                     ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/shapeBlendMode", QVariant( QgsMapRenderer::BlendNormal ) ).toUInt() );

  // drop shadow
  shadowDraw = layer->customProperty( "labeling/shadowDraw", QVariant( false ) ).toBool();
  shadowUnder = ( ShadowType )layer->customProperty( "labeling/shadowUnder", QVariant( ShadowLowest ) ).toUInt();//ShadowLowest;
  shadowOffsetAngle = layer->customProperty( "labeling/shadowOffsetAngle", QVariant( 135 ) ).toInt();
  shadowOffsetDist = layer->customProperty( "labeling/shadowOffsetDist", QVariant( 0.0 ) ).toDouble();
  shadowOffsetUnits = ( SizeUnit )layer->customProperty( "labeling/shadowOffsetUnits", QVariant( MM ) ).toUInt();
  shadowOffsetGlobal = layer->customProperty( "labeling/shadowOffsetGlobal", QVariant( true ) ).toBool();
  shadowRadius = layer->customProperty( "labeling/shadowRadius", QVariant( 0.0 ) ).toDouble();
  shadowRadiusUnits = ( SizeUnit )layer->customProperty( "labeling/shadowRadiusUnits", QVariant( MM ) ).toUInt();
  shadowRadiusAlphaOnly = layer->customProperty( "labeling/shadowRadiusAlphaOnly", QVariant( false ) ).toBool();
  shadowTransparency = layer->customProperty( "labeling/shadowTransparency", QVariant( 0 ) ).toInt();
  shadowScale = layer->customProperty( "labeling/shadowScale", QVariant( 100 ) ).toInt();
  shadowColor = _readColor( layer, "labeling/shadowColor", Qt::black, false );
  shadowBlendMode = QgsMapRenderer::getCompositionMode(
                      ( QgsMapRenderer::BlendMode )layer->customProperty( "labeling/shadowBlendMode", QVariant( QgsMapRenderer::BlendMultiply ) ).toUInt() );

  readDataDefinedPropertyMap( layer, dataDefinedProperties );
}

void QgsPalLayerSettings::writeToLayer( QgsVectorLayer* layer )
{
  // this is a mark that labeling information is present
  layer->setCustomProperty( "labeling", "pal" );

  layer->setCustomProperty( "labeling/fieldName", fieldName );
  layer->setCustomProperty( "labeling/isExpression", isExpression );
  layer->setCustomProperty( "labeling/placement", placement );
  layer->setCustomProperty( "labeling/placementFlags", ( unsigned int )placementFlags );
  layer->setCustomProperty( "labeling/xQuadOffset", xQuadOffset );
  layer->setCustomProperty( "labeling/yQuadOffset", yQuadOffset );
  layer->setCustomProperty( "labeling/xOffset", xOffset );
  layer->setCustomProperty( "labeling/yOffset", yOffset );
  layer->setCustomProperty( "labeling/angleOffset", angleOffset );
  layer->setCustomProperty( "labeling/centroidWhole", centroidWhole );

  layer->setCustomProperty( "labeling/fontFamily", textFont.family() );
  layer->setCustomProperty( "labeling/namedStyle", textNamedStyle );
  layer->setCustomProperty( "labeling/fontCapitals", ( unsigned int )textFont.capitalization() );
  layer->setCustomProperty( "labeling/fontSize", textFont.pointSizeF() );
  layer->setCustomProperty( "labeling/fontWeight", textFont.weight() );
  layer->setCustomProperty( "labeling/fontItalic", textFont.italic() );
  layer->setCustomProperty( "labeling/fontStrikeout", textFont.strikeOut() );
  layer->setCustomProperty( "labeling/fontUnderline", textFont.underline() );
  layer->setCustomProperty( "labeling/fontLetterSpacing", textFont.letterSpacing() );
  layer->setCustomProperty( "labeling/fontWordSpacing", textFont.wordSpacing() );

  _writeColor( layer, "labeling/textColor", textColor );
  layer->setCustomProperty( "labeling/textTransp", textTransp );
  layer->setCustomProperty( "labeling/blendMode", QgsMapRenderer::getBlendModeEnum( blendMode ) );
  layer->setCustomProperty( "labeling/previewBkgrdColor", previewBkgrdColor.name() );
  layer->setCustomProperty( "labeling/enabled", enabled );
  layer->setCustomProperty( "labeling/priority", priority );
  layer->setCustomProperty( "labeling/obstacle", obstacle );
  layer->setCustomProperty( "labeling/dist", dist );
  layer->setCustomProperty( "labeling/scaleMin", scaleMin );
  layer->setCustomProperty( "labeling/scaleMax", scaleMax );
  layer->setCustomProperty( "labeling/bufferSize", bufferSize );
  _writeColor( layer, "labeling/bufferColor", bufferColor );
  layer->setCustomProperty( "labeling/bufferTransp", bufferTransp );
  layer->setCustomProperty( "labeling/bufferBlendMode", QgsMapRenderer::getBlendModeEnum( bufferBlendMode ) );
  layer->setCustomProperty( "labeling/bufferJoinStyle", ( unsigned int )bufferJoinStyle );
  layer->setCustomProperty( "labeling/bufferNoFill", bufferNoFill );
  layer->setCustomProperty( "labeling/formatNumbers", formatNumbers );
  layer->setCustomProperty( "labeling/decimals", decimals );
  layer->setCustomProperty( "labeling/plussign", plusSign );
  layer->setCustomProperty( "labeling/labelPerPart", labelPerPart );
  layer->setCustomProperty( "labeling/displayAll", displayAll );
  layer->setCustomProperty( "labeling/mergeLines", mergeLines );
  layer->setCustomProperty( "labeling/addDirectionSymbol", addDirectionSymbol );
  layer->setCustomProperty( "labeling/leftDirectionSymbol", leftDirectionSymbol );
  layer->setCustomProperty( "labeling/rightDirectionSymbol", rightDirectionSymbol );
  layer->setCustomProperty( "labeling/reverseDirectionSymbol", reverseDirectionSymbol );
  layer->setCustomProperty( "labeling/placeDirectionSymbol", ( unsigned int )placeDirectionSymbol );
  layer->setCustomProperty( "labeling/upsidedownLabels", ( unsigned int )upsidedownLabels );
  layer->setCustomProperty( "labeling/maxCurvedCharAngleIn", maxCurvedCharAngleIn );
  layer->setCustomProperty( "labeling/maxCurvedCharAngleOut", maxCurvedCharAngleOut );
  layer->setCustomProperty( "labeling/minFeatureSize", minFeatureSize );
  layer->setCustomProperty( "labeling/limitNumLabels", limitNumLabels );
  layer->setCustomProperty( "labeling/maxNumLabels", maxNumLabels );
  layer->setCustomProperty( "labeling/fontSizeInMapUnits", fontSizeInMapUnits );
  layer->setCustomProperty( "labeling/fontLimitPixelSize", fontLimitPixelSize );
  layer->setCustomProperty( "labeling/fontMinPixelSize", fontMinPixelSize );
  layer->setCustomProperty( "labeling/fontMaxPixelSize", fontMaxPixelSize );
  layer->setCustomProperty( "labeling/bufferSizeInMapUnits", bufferSizeInMapUnits );
  layer->setCustomProperty( "labeling/distInMapUnits", distInMapUnits );
  layer->setCustomProperty( "labeling/labelOffsetInMapUnits", labelOffsetInMapUnits );
  layer->setCustomProperty( "labeling/wrapChar", wrapChar );
  layer->setCustomProperty( "labeling/multilineHeight", multilineHeight );
  layer->setCustomProperty( "labeling/multilineAlign", ( unsigned int )multilineAlign );
  layer->setCustomProperty( "labeling/preserveRotation", preserveRotation );

  // shape background
  layer->setCustomProperty( "labeling/shapeDraw", shapeDraw );
  layer->setCustomProperty( "labeling/shapeType", ( unsigned int )shapeType );
  layer->setCustomProperty( "labeling/shapeSVGFile", shapeSVGFile );
  layer->setCustomProperty( "labeling/shapeSizeType", ( unsigned int )shapeSizeType );
  layer->setCustomProperty( "labeling/shapeSizeX", shapeSize.x() );
  layer->setCustomProperty( "labeling/shapeSizeY", shapeSize.y() );
  layer->setCustomProperty( "labeling/shapeSizeUnits", ( unsigned int )shapeSizeUnits );
  layer->setCustomProperty( "labeling/shapeRotationType", ( unsigned int )shapeRotationType );
  layer->setCustomProperty( "labeling/shapeRotation", shapeRotation );
  layer->setCustomProperty( "labeling/shapeOffsetX", shapeOffset.x() );
  layer->setCustomProperty( "labeling/shapeOffsetY", shapeOffset.y() );
  layer->setCustomProperty( "labeling/shapeOffsetUnits", ( unsigned int )shapeOffsetUnits );
  layer->setCustomProperty( "labeling/shapeRadiiX", shapeRadii.x() );
  layer->setCustomProperty( "labeling/shapeRadiiY", shapeRadii.y() );
  layer->setCustomProperty( "labeling/shapeRadiiUnits", ( unsigned int )shapeRadiiUnits );
  _writeColor( layer, "labeling/shapeFillColor", shapeFillColor, true );
  _writeColor( layer, "labeling/shapeBorderColor", shapeBorderColor, true );
  layer->setCustomProperty( "labeling/shapeBorderWidth", shapeBorderWidth );
  layer->setCustomProperty( "labeling/shapeBorderWidthUnits", ( unsigned int )shapeBorderWidthUnits );
  layer->setCustomProperty( "labeling/shapeJoinStyle", ( unsigned int )shapeJoinStyle );
  layer->setCustomProperty( "labeling/shapeTransparency", shapeTransparency );
  layer->setCustomProperty( "labeling/shapeBlendMode", QgsMapRenderer::getBlendModeEnum( shapeBlendMode ) );

  // drop shadow
  layer->setCustomProperty( "labeling/shadowDraw", shadowDraw );
  layer->setCustomProperty( "labeling/shadowUnder", ( unsigned int )shadowUnder );
  layer->setCustomProperty( "labeling/shadowOffsetAngle", shadowOffsetAngle );
  layer->setCustomProperty( "labeling/shadowOffsetDist", shadowOffsetDist );
  layer->setCustomProperty( "labeling/shadowOffsetUnits", ( unsigned int )shadowOffsetUnits );
  layer->setCustomProperty( "labeling/shadowOffsetGlobal", shadowOffsetGlobal );
  layer->setCustomProperty( "labeling/shadowRadius", shadowRadius );
  layer->setCustomProperty( "labeling/shadowRadiusUnits", ( unsigned int )shadowRadiusUnits );
  layer->setCustomProperty( "labeling/shadowRadiusAlphaOnly", shadowRadiusAlphaOnly );
  layer->setCustomProperty( "labeling/shadowTransparency", shadowTransparency );
  layer->setCustomProperty( "labeling/shadowScale", shadowScale );
  _writeColor( layer, "labeling/shadowColor", shadowColor, false );
  layer->setCustomProperty( "labeling/shadowBlendMode", QgsMapRenderer::getBlendModeEnum( shadowBlendMode ) );

  writeDataDefinedPropertyMap( layer, dataDefinedProperties );
}

void QgsPalLayerSettings::setDataDefinedProperty( DataDefinedProperties p, QString attributeName )
{
  dataDefinedProperties.insert( p, attributeName );
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

  QString wrapchr = !wrapChar.isEmpty() ? wrapChar : QString( "\n" );

  //consider the space needed for the direction symbol
  if ( addDirectionSymbol && placement == QgsPalLayerSettings::Line
       && ( !leftDirectionSymbol.isEmpty() || !rightDirectionSymbol.isEmpty() ) )
  {
    QString dirSym = leftDirectionSymbol;

    if ( fm->width( rightDirectionSymbol ) > fm->width( dirSym ) )
      dirSym = rightDirectionSymbol;

    if ( placeDirectionSymbol == QgsPalLayerSettings::SymbolLeftRight )
    {
      text.append( dirSym );
    }
    else
    {
      text.prepend( dirSym + wrapchr ); // SymbolAbove or SymbolBelow
    }
  }

  double w = 0.0, h = 0.0;
  QStringList multiLineSplit = text.split( wrapchr );
  int lines = multiLineSplit.size();

  double labelHeight = fm->ascent() + fm->descent(); // ignore +1 for baseline

  h += fm->height() + ( double )(( lines - 1 ) * labelHeight * multilineHeight );
  h /= rasterCompressFactor;

  for ( int i = 0; i < lines; ++i )
  {
    double width = fm->width( multiLineSplit.at( i ) );
    if ( width > w )
    {
      w = width;
    }
  }
  w /= rasterCompressFactor;
  QgsPoint ptSize = xform->toMapCoordinatesF( w, h );

  labelX = qAbs( ptSize.x() - ptZero.x() );
  labelY = qAbs( ptSize.y() - ptZero.y() );
}

void QgsPalLayerSettings::registerFeature( QgsVectorLayer* layer,  QgsFeature& f, const QgsRenderContext& context )
{
  // data defined show label? defaults to show label if not 0
  QMap< DataDefinedProperties, QString >::const_iterator showIt = dataDefinedProperties.find( QgsPalLayerSettings::Show );
  if ( showIt != dataDefinedProperties.constEnd() )
  {
    QVariant showValue = f.attribute( *showIt );
    if ( showValue.isValid() )
    {
      bool conversionOk;
      int showLabel = showValue.toInt( &conversionOk );
      if ( conversionOk && showLabel == 0 )
      {
        return;
      }
    }
  }

  // data defined min scale?
  QMap< DataDefinedProperties, QString >::const_iterator minScaleIt = dataDefinedProperties.find( QgsPalLayerSettings::MinScale );
  if ( minScaleIt != dataDefinedProperties.constEnd() )
  {
    QVariant minScaleValue = f.attribute( *minScaleIt );
    if ( minScaleValue.isValid() )
    {
      bool conversionOk;
      double minScale = minScaleValue.toDouble( &conversionOk );
      if ( conversionOk && context.rendererScale() < minScale )
      {
        return;
      }
    }
  }

  // data defined max scale?
  QMap< DataDefinedProperties, QString >::const_iterator maxScaleIt = dataDefinedProperties.find( QgsPalLayerSettings::MaxScale );
  if ( maxScaleIt != dataDefinedProperties.constEnd() )
  {
    QVariant maxScaleValue = f.attribute( *maxScaleIt );
    if ( maxScaleValue.isValid() )
    {
      bool conversionOk;
      double maxScale = maxScaleValue.toDouble( &conversionOk );
      if ( conversionOk && context.rendererScale() > maxScale )
      {
        return;
      }
    }
  }

  QFont labelFont = textFont;

  //data defined label size?
  QMap< DataDefinedProperties, QString >::const_iterator it = dataDefinedProperties.find( QgsPalLayerSettings::Size );
  if ( it != dataDefinedProperties.constEnd() )
  {
    //find out size
    QVariant size = f.attribute( *it );
    if ( size.isValid() )
    {
      double sizeDouble = size.toDouble();
      if ( sizeDouble <= 0.0 || sizeToPixel( sizeDouble, context, fontSizeInMapUnits ? MapUnits : Points, true ) < 1 )
      {
        return;
      }
      labelFont.setPixelSize( sizeToPixel( sizeDouble, context, fontSizeInMapUnits ? MapUnits : Points, true ) );
    }
  }

  // defined 'minimum/maximum pixel font size' option
  // TODO: add any data defined setting to override fontMinPixelSize/fontMaxPixelSize
  if ( fontLimitPixelSize && fontSizeInMapUnits &&
       ( fontMinPixelSize > labelFont.pixelSize()  || labelFont.pixelSize() > fontMaxPixelSize ) )
  {
    return;
  }

  QString labelText;

  // Check to see if we are a expression string.
  if ( isExpression )
  {
    QgsExpression* exp = getLabelExpression();
    if ( exp->hasParserError() )
    {
      QgsDebugMsg( "Expression parser error:" + exp->parserErrorString() );
      return;
    }
    exp->setScale( context.rendererScale() );
    QVariant result = exp->evaluate( &f, layer->pendingFields() );
    if ( exp->hasEvalError() )
    {
      QgsDebugMsg( "Expression parser eval error:" + exp->evalErrorString() );
      return;
    }
    labelText  = result.toString();
  }
  else if ( formatNumbers == true && ( f.attribute( fieldIndex ).type() == QVariant::Int ||
                                       f.attribute( fieldIndex ).type() == QVariant::Double ) )
  {
    QString numberFormat;
    double d = f.attribute( fieldIndex ).toDouble();
    if ( d > 0 && plusSign == true )
    {
      numberFormat.append( "+" );
    }
    numberFormat.append( "%1" );
    labelText = numberFormat.arg( d, 0, 'f', decimals );
  }
  else
  {
    labelText = f.attribute( fieldIndex ).toString();
  }

  // this should come AFTER any data defined option that affects font metrics
  QFontMetricsF* labelFontMetrics = new QFontMetricsF( labelFont );
  double labelX, labelY; // will receive label size
  calculateLabelSize( labelFontMetrics, labelText, labelX, labelY );

  // maximum angle between curved label characters
  double maxcharanglein = 20.0;
  double maxcharangleout = -20.0;
  if ( placement == QgsPalLayerSettings::Curved )
  {
    maxcharanglein = maxCurvedCharAngleIn;
    maxcharangleout = maxCurvedCharAngleOut > 0 ? -maxCurvedCharAngleOut : maxCurvedCharAngleOut;
  }
  // TODO: add data defined override for maximum angle between curved label characters

  QgsGeometry* geom = f.geometry();
  if ( !geom )
  {
    return;
  }

  if ( ct ) // reproject the geometry if necessary
    geom->transform( *ct );

  if ( !checkMinimumSizeMM( context, geom, minFeatureSize ) )
  {
    return;
  }

  // whether we're going to create a centroid for polygon
  bool centroidPoly = (( placement == QgsPalLayerSettings::AroundPoint
                         || placement == QgsPalLayerSettings::OverPoint )
                       && geom->type() == QGis::Polygon );

  // CLIP the geometry if it is bigger than the extent
  // don't clip if centroid is requested for whole feature
  bool do_clip = false;
  if ( !centroidPoly || ( centroidPoly && !centroidWhole ) )
  {
    do_clip = !extentGeom->contains( geom );
    if ( do_clip )
    {
      geom = geom->intersection( extentGeom ); // creates new geometry
      if ( !geom )
      {
        return;
      }
    }
  }

  GEOSGeometry* geos_geom = geom->asGeos();

  if ( geos_geom == NULL )
    return; // invalid geometry

  // likelihood exists label will be registered with PAL and may be drawn
  // check if max number of features to label (already registered with PAL) has been reached
  // Debug output at end of QgsPalLabeling::drawLabeling(), when deleting temp geometries
  if ( limitNumLabels )
  {
    if ( !maxNumLabels )
    {
      return;
    }
    mFeatsRegPal = palLayer->getNbFeatures();
    if ( mFeatsRegPal >= maxNumLabels )
    {
      return;
    }

    int divNum = ( int )(( mFeaturesToLabel / maxNumLabels ) + 0.5 );
    if ( divNum && ( mFeatsRegPal == ( int )( mFeatsSendingToPal / divNum ) ) )
    {
      mFeatsSendingToPal += 1;
      if ( divNum &&  mFeatsSendingToPal % divNum )
      {
        return;
      }
    }
  }

  GEOSGeometry* geos_geom_clone = GEOSGeom_clone( geos_geom );

  //data defined position / alignment / rotation?
  bool dataDefinedPosition = false;
  bool labelIsPinned = false;
  bool layerDefinedRotation = false;
  bool dataDefinedRotation = false;
  double xPos = 0.0, yPos = 0.0, angle = 0.0;
  bool ddXPos = false, ddYPos = false;
  double quadOffsetX = 0.0, quadOffsetY = 0.0;
  double offsetX = 0.0, offsetY = 0.0;

  // adjust quadrant offset of labels
  if ( xQuadOffset != 0 )
  {
    quadOffsetX = xQuadOffset;
  }
  if ( yQuadOffset != 0 )
  {
    quadOffsetY = yQuadOffset;
  }

  // adjust offset of labels to match chosen unit and map scale
  // offsets match those of symbology: -x = left, -y = up
  double mapUntsPerMM = context.mapToPixel().mapUnitsPerPixel() * context.scaleFactor();
  if ( xOffset != 0 )
  {
    offsetX = xOffset;  // must be positive to match symbology offset direction
    if ( !labelOffsetInMapUnits )
    {
      offsetX *= mapUntsPerMM; //convert offset from mm to map units
    }
  }
  if ( yOffset != 0 )
  {
    offsetY = -yOffset; // must be negative to match symbology offset direction
    if ( !labelOffsetInMapUnits )
    {
      offsetY *= mapUntsPerMM; //convert offset from mm to map units
    }
  }

  // layer defined rotation?
  // only rotate non-pinned OverPoint placements until other placements are supported in pal::Feature
  if ( placement == QgsPalLayerSettings::OverPoint && angleOffset != 0 )
  {
    layerDefinedRotation = true;
    angle = angleOffset * M_PI / 180; // convert to radians
  }

  //data defined rotation?
  QMap< DataDefinedProperties, QString >::const_iterator rotIt = dataDefinedProperties.find( QgsPalLayerSettings::Rotation );
  if ( rotIt != dataDefinedProperties.constEnd() )
  {
    dataDefinedRotation = true;
    angle = f.attribute( *rotIt ).toDouble() * M_PI / 180.0;
  }

  QMap< DataDefinedProperties, QString >::const_iterator dPosXIt = dataDefinedProperties.find( QgsPalLayerSettings::PositionX );
  if ( dPosXIt != dataDefinedProperties.constEnd() )
  {
    QMap< DataDefinedProperties, QString >::const_iterator dPosYIt = dataDefinedProperties.find( QgsPalLayerSettings::PositionY );
    if ( dPosYIt != dataDefinedProperties.constEnd() )
    {
      //data defined position. But field values could be NULL -> positions will be generated by PAL
      xPos = f.attribute( *dPosXIt ).toDouble( &ddXPos );
      yPos = f.attribute( *dPosYIt ).toDouble( &ddYPos );

      if ( ddXPos && ddYPos )
      {
        dataDefinedPosition = true;
        labelIsPinned = true;
        // layer rotation set, but don't rotate pinned labels unless data defined
        if ( layerDefinedRotation && !dataDefinedRotation )
        {
          angle = 0.0;
        }

        //x/y shift in case of alignment
        double xdiff = 0.0;
        double ydiff = 0.0;

        //horizontal alignment
        QMap< DataDefinedProperties, QString >::const_iterator haliIt = dataDefinedProperties.find( QgsPalLayerSettings::Hali );
        if ( haliIt != dataDefinedProperties.constEnd() )
        {
          QString haliString = f.attribute( *haliIt ).toString();
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
        QMap< DataDefinedProperties, QString >::const_iterator valiIt = dataDefinedProperties.find( QgsPalLayerSettings::Vali );
        if ( valiIt != dataDefinedProperties.constEnd() )
        {
          QString valiString = f.attribute( *valiIt ).toString();
          if ( valiString.compare( "Bottom", Qt::CaseInsensitive ) != 0 )
          {
            if ( valiString.compare( "Top", Qt::CaseInsensitive ) == 0 || valiString.compare( "Cap", Qt::CaseInsensitive ) == 0 )
            {
              ydiff -= labelY;
            }
            else
            {
              double descentRatio = labelFontMetrics->descent() / labelFontMetrics->height();

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

        if ( dataDefinedRotation )
        {
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

        xPos += xdiff;
        yPos += ydiff;
      }
      else
      {
        // only rotate non-pinned OverPoint placements until other placements are supported in pal::Feature
        if ( dataDefinedRotation && placement != QgsPalLayerSettings::OverPoint )
        {
          angle = 0.0;
        }
      }
    }
  }

  // data defined always show?
  bool alwaysShow = false;
  QMap< DataDefinedProperties, QString >::const_iterator dAlwShowIt = dataDefinedProperties.find( QgsPalLayerSettings::AlwaysShow );
  if ( dAlwShowIt != dataDefinedProperties.constEnd() )
  {
    QVariant alwShow = f.attribute( *dAlwShowIt );
    if ( alwShow.isValid() )
    {
      alwaysShow = alwShow.toBool();
    }
  }

  QgsPalGeometry* lbl = new QgsPalGeometry(
    f.id(),
    labelText,
    geos_geom_clone,
    labelFont.letterSpacing(),
    labelFont.wordSpacing(),
    placement == QgsPalLayerSettings::Curved );

  // record the created geometry - it will be deleted at the end.
  geometries.append( lbl );

  //  feature to the layer
  try
  {
    if ( !palLayer->registerFeature( lbl->strId(), lbl, labelX, labelY, labelText.toUtf8().constData(),
                                     xPos, yPos, dataDefinedPosition, angle, dataDefinedRotation,
                                     quadOffsetX, quadOffsetY, offsetX, offsetY, alwaysShow ) )
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
  // account for any data defined font metrics adjustments
  feat->setLabelInfo( lbl->info( labelFontMetrics, xform, rasterCompressFactor, maxcharanglein, maxcharangleout ) );
  delete labelFontMetrics;

  // TODO: allow layer-wide feature dist in PAL...?

  //data defined label-feature distance?
  double distance = dist;
  QMap< DataDefinedProperties, QString >::const_iterator dDistIt = dataDefinedProperties.find( QgsPalLayerSettings::LabelDistance );
  if ( dDistIt != dataDefinedProperties.constEnd() )
  {
    distance = f.attribute( *dDistIt ).toDouble();
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
  QMap< DataDefinedProperties, QString >::const_iterator dIt = dataDefinedProperties.constBegin();
  for ( ; dIt != dataDefinedProperties.constEnd(); ++dIt )
  {
    lbl->addDataDefinedValue( dIt.key(), f.attribute( dIt.value() ) );
  }

  // set geometry's pinned property
  lbl->setIsPinned( labelIsPinned );
}

int QgsPalLayerSettings::sizeToPixel( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor ) const
{
  return ( int )( scaleToPixelContext( size, c, unit, rasterfactor ) + 0.5 );
}

double QgsPalLayerSettings::scaleToPixelContext( double size, const QgsRenderContext& c, SizeUnit unit, bool rasterfactor ) const
{
  // if render context is that of device (i.e. not a scaled map), just return size
  double mapUnitsPerPixel = c.mapToPixel().mapUnitsPerPixel();

  if ( unit == MapUnits && mapUnitsPerPixel > 0.0 )
  {
    size = size / mapUnitsPerPixel * ( rasterfactor ? c.rasterScaleFactor() : 1 );
  }
  else // e.g. in points or mm
  {
    double ptsTomm = ( unit == Points ? 0.352778 : 1 );
    size *= ptsTomm * c.scaleFactor() * ( rasterfactor ? c.rasterScaleFactor() : 1 );
  }
  return size;
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
  QgsDebugMsg( "PREPARE LAYER " + layer->id() );
  Q_ASSERT( mMapRenderer != NULL );

  // start with a temporary settings class, find out labeling info
  QgsPalLayerSettings lyrTmp;
  lyrTmp.readFromLayer( layer );

  if ( !lyrTmp.enabled )
    return 0;


  int fldIndex = -1;
  if ( lyrTmp.isExpression )
  {
    if ( lyrTmp.fieldName.isEmpty() )
      return 0;
    QgsExpression exp( lyrTmp.fieldName );
    foreach ( QString name, exp.referencedColumns() )
    {
      QgsDebugMsg( "REFERENCED COLUMN = " + name );
      fldIndex =  layer->fieldNameIndex( name );
      attrIndices.insert( fldIndex );
    }

  }
  else
  {
    // If we aren't an expression, we check to see if we can find the column.
    fldIndex = layer->fieldNameIndex( lyrTmp.fieldName );
    if ( fldIndex == -1 )
      return 0;
    attrIndices.insert( fldIndex );
  }

  //add indices of data defined fields
  QMap< QgsPalLayerSettings::DataDefinedProperties, QString >::const_iterator dIt = lyrTmp.dataDefinedProperties.constBegin();
  for ( ; dIt != lyrTmp.dataDefinedProperties.constEnd(); ++dIt )
  {
    attrIndices.insert( layer->fieldNameIndex( dIt.value() ) );
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
                             METER, priority, lyr.obstacle, true, true,
                             lyr.displayAll );

  if ( lyr.placementFlags )
    l->setArrangementFlags( lyr.placementFlags );

  // set label mode (label per feature is the default)
  l->setLabelMode( lyr.labelPerPart ? Layer::LabelPerFeaturePart : Layer::LabelPerFeature );

  // set whether adjacent lines should be merged
  l->setMergeConnectedLines( lyr.mergeLines );

  // set how to show upside-down labels
  Layer::UpsideDownLabels upsdnlabels;
  switch ( lyr.upsidedownLabels )
  {
    case QgsPalLayerSettings::Upright:     upsdnlabels = Layer::Upright; break;
    case QgsPalLayerSettings::ShowDefined: upsdnlabels = Layer::ShowDefined; break;
    case QgsPalLayerSettings::ShowAll:     upsdnlabels = Layer::ShowAll; break;
    default: Q_ASSERT( "unsupported upside-down label setting" && 0 ); return 0;
  }
  l->setUpsidedownLabels( upsdnlabels );

  // fix for font size in map units causing font to show pointsize at small map scales
  int pixelFontSize = lyr.sizeToPixel( lyr.textFont.pointSizeF(), ctx,
                                       lyr.fontSizeInMapUnits ? QgsPalLayerSettings::MapUnits : QgsPalLayerSettings::Points,
                                       true );

  if ( pixelFontSize < 1 )
  {
    lyr.textFont.setPointSize( 1 );
    lyr.textFont.setPixelSize( 1 );
  }
  else
  {
    lyr.textFont.setPixelSize( pixelFontSize );
  }

  // scale spacing sizes if using map units
  if ( lyr.fontSizeInMapUnits )
  {
    double spacingPixelSize;
    if ( lyr.textFont.wordSpacing() != 0 )
    {
      spacingPixelSize = lyr.textFont.wordSpacing() / ctx.mapToPixel().mapUnitsPerPixel() * ctx.rasterScaleFactor();
      lyr.textFont.setWordSpacing( spacingPixelSize );
    }

    if ( lyr.textFont.letterSpacing() != 0 )
    {
      spacingPixelSize = lyr.textFont.letterSpacing() / ctx.mapToPixel().mapUnitsPerPixel() * ctx.rasterScaleFactor();
      lyr.textFont.setLetterSpacing( QFont::AbsoluteSpacing, spacingPixelSize );
    }
  }

  //raster and vector scale factors
  lyr.vectorScaleFactor = ctx.scaleFactor();
  lyr.rasterCompressFactor = ctx.rasterScaleFactor();

  // save the pal layer to our layer context (with some additional info)
  lyr.palLayer = l;
  lyr.fieldIndex = fldIndex;

  lyr.xform = mMapRenderer->coordinateTransform();
  if ( mMapRenderer->hasCrsTransformEnabled() )
    lyr.ct = new QgsCoordinateTransform( layer->crs(), mMapRenderer->destinationCrs() );
  else
    lyr.ct = NULL;
  lyr.ptZero = lyr.xform->toMapCoordinates( 0, 0 );
  lyr.ptOne = lyr.xform->toMapCoordinates( 1, 0 );

  // rect for clipping
  lyr.extentGeom = QgsGeometry::fromRect( mMapRenderer->extent() );

  lyr.mFeatsSendingToPal = 0;

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
  lyr.registerFeature( layer, f, context );
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
    QSizeF diagSize = dr->sizeMapUnits( feat.attributes(), context );
    if ( diagSize.isValid() )
    {
      diagramWidth = diagSize.width();
      diagramHeight = diagSize.height();
    }

    //append the diagram attributes to lbl
    lbl->setDiagramAttributes( feat.attributes() );
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
    ddPosX = feat.attribute( ddColX ).toDouble( &posXOk ) - diagramWidth / 2.0;
    ddPosY = feat.attribute( ddColY ).toDouble( &posYOk ) - diagramHeight / 2.0;
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
        mLabelSearchTree->insertLabel( *it,  QString( palGeometry->strId() ).toInt(), QString( "" ) , layerId, true, false );
      }
      continue;
    }

    const QgsPalLayerSettings& lyr = layer( layerNameUtf8 );

    // Copy to temp, editable layer settings
    // these settings will be changed by any data defined values, then used for rendering label components
    // settings may be adjusted during rendering of components
    QgsPalLayerSettings tmpLyr( lyr );

    //apply data defined settings for the label
    //font size
    QVariant dataDefinedSize = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Size );
    if ( dataDefinedSize.isValid() )
    {
      tmpLyr.textFont.setPixelSize( lyr.sizeToPixel( dataDefinedSize.toDouble(), context,
                                    ( lyr.fontSizeInMapUnits ? QgsPalLayerSettings::MapUnits : QgsPalLayerSettings::Points ),
                                    true ) );
    }
    //font color
    QVariant dataDefinedColor = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Color );
    if ( dataDefinedColor.isValid() )
    {
      tmpLyr.textColor.setNamedColor( dataDefinedColor.toString() );
      if ( !tmpLyr.textColor.isValid() )
      {
        tmpLyr.textColor = lyr.textColor;
      }
    }
    //font transparency
    QVariant dataDefinedFontTransp = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::FontTransp );
    if ( dataDefinedFontTransp.isValid() )
    {
      bool ftOk = false;
      int ft = dataDefinedFontTransp.toInt( &ftOk );
      if ( ftOk && ft >= 0 && ft <= 100 )
      {
        tmpLyr.textTransp = ft;
      }
    }
    tmpLyr.textColor.setAlphaF(( 100.0 - ( double )( tmpLyr.textTransp ) ) / 100.0 );

    //font bold
    QVariant dataDefinedBold = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Bold );
    if ( dataDefinedBold.isValid() )
    {
      tmpLyr.textFont.setBold(( bool ) dataDefinedBold.toInt() );
    }
    //font italic
    QVariant dataDefinedItalic = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Italic );
    if ( dataDefinedItalic.isValid() )
    {
      tmpLyr.textFont.setItalic(( bool ) dataDefinedItalic.toInt() );
    }
    //font underline
    QVariant dataDefinedUnderline = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Underline );
    if ( dataDefinedUnderline.isValid() )
    {
      tmpLyr.textFont.setUnderline(( bool ) dataDefinedUnderline.toInt() );
    }
    //font strikeout
    QVariant dataDefinedStrikeout = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Strikeout );
    if ( dataDefinedStrikeout.isValid() )
    {
      tmpLyr.textFont.setStrikeOut(( bool ) dataDefinedStrikeout.toInt() );
    }
    //font family
    QVariant dataDefinedFontFamily = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::Family );
    if ( dataDefinedFontFamily.isValid() )
    {
      tmpLyr.textFont.setFamily( dataDefinedFontFamily.toString() );
    }
    //buffer size
    QVariant dataDefinedBufferSize = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::BufferSize );
    if ( dataDefinedBufferSize.isValid() )
    {
      tmpLyr.bufferSize = dataDefinedBufferSize.toDouble();
    }

    //buffer color
    QVariant dataDefinedBufferColor = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::BufferColor );
    if ( dataDefinedBufferColor.isValid() )
    {
      tmpLyr.bufferColor.setNamedColor( dataDefinedBufferColor.toString() );
      if ( !tmpLyr.bufferColor.isValid() )
      {
        tmpLyr.bufferColor = lyr.bufferColor;
      }
    }
    //buffer transparency
    QVariant dataDefinedBufTransp = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::BufferTransp );
    if ( dataDefinedBufTransp.isValid() )
    {
      bool btOk = false;
      int bt = dataDefinedBufTransp.toInt( &btOk );
      if ( btOk && bt >= 0 && bt <= 100 )
      {
        tmpLyr.bufferTransp = bt;
      }
    }
    tmpLyr.bufferColor.setAlphaF(( 100.0 - ( double )( tmpLyr.bufferTransp ) ) / 100.0 );

    tmpLyr.showingShadowRects = mShowingShadowRects;

    tmpLyr.shadowDraw = ( tmpLyr.shadowDraw
                          && tmpLyr.shadowTransparency < 100
                          && !( tmpLyr.shadowOffsetDist == 0.0 && tmpLyr.shadowRadius == 0.0 ) );

    // Render the components of a label in reverse order
    //   (backgrounds -> text)

    if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowLowest )
    {
      if ( tmpLyr.shapeDraw )
      {
        tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowShape;
      }
      else if ( tmpLyr.bufferSize > 0 )
      {
        tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowBuffer;
      }
      else
      {
        tmpLyr.shadowUnder = QgsPalLayerSettings::ShadowText;
      }
    }

    if ( tmpLyr.shapeDraw )
    {
      drawLabel( *it, context, tmpLyr, LabelShape );
    }

    if ( tmpLyr.bufferSize > 0 )
    {
      drawLabel( *it, context, tmpLyr, LabelBuffer );
    }

    drawLabel( *it, context, tmpLyr, LabelText );

    if ( mLabelSearchTree )
    {
      QString labeltext = (( QgsPalGeometry* )( *it )->getFeaturePart()->getUserGeometry() )->text();
      mLabelSearchTree->insertLabel( *it,  QString( palGeometry->strId() ).toInt(), ( *it )->getLayerName(), labeltext, false, palGeometry->isPinned() );
    }
  }

  // Reset composition mode for further drawing operations
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

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
    if ( lyr.limitNumLabels )
    {
      QgsDebugMsg( QString( "mFeaturesToLabel: %1" ).arg( lyr.mFeaturesToLabel ) );
      QgsDebugMsg( QString( "maxNumLabels: %1" ).arg( lyr.maxNumLabels ) );
      QgsDebugMsg( QString( "mFeatsSendingToPal: %1" ).arg( lyr.mFeatsSendingToPal ) );
      QgsDebugMsg( QString( "mFeatsRegPal: %1" ).arg( lyr.geometries.count() ) );
    }
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

QList<QgsLabelPosition> QgsPalLabeling::labelsWithinRect( const QgsRectangle& r )
{
  QList<QgsLabelPosition> positions;

  QList<QgsLabelPosition*> positionPointers;
  if ( mLabelSearchTree )
  {
    mLabelSearchTree->labelsInRect( r, positionPointers );
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

void QgsPalLabeling::drawLabel( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, DrawLabelType drawType )
{
  // NOTE: this is repeatedly called for multi-part labels
  QPainter* painter = context.painter();
  const QgsMapToPixel* xform = &context.mapToPixel();

  QgsLabelComponent component;

  // account for print output or image saving @ specific dpi
  if ( !qgsDoubleNear( context.rasterScaleFactor(), 1.0, 0.1 ) )
  {
    // find relative dpi scaling for local painter
    QPicture localPict;
    QPainter localp;
    localp.begin( &localPict );

    double localdpi = ( localp.device()->logicalDpiX() + localp.device()->logicalDpiY() ) / 2;
    double contextdpi = ( painter->device()->logicalDpiX() + painter->device()->logicalDpiY() ) / 2;
    component.setDpiRatio( localdpi / contextdpi );

    localp.end();
  }

  QgsPoint outPt = xform->transform( label->getX(), label->getY() );
//  QgsPoint outPt2 = xform->transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
//  QRectF labelRect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );

  component.setOrigin( outPt );
  component.setRotation( label->getAlpha() );

  if ( drawType == QgsPalLabeling::LabelShape )
  {
    // get rotated label's center point
    QgsPoint centerPt( outPt );
    QgsPoint outPt2 = xform->transform( label->getX() + label->getWidth() / 2,
                                        label->getY() + label->getHeight() / 2 );

    double xc = outPt2.x() - outPt.x();
    double yc = outPt2.y() - outPt.y();

    double angle = -label->getAlpha();
    double xd = xc * cos( angle ) - yc * sin( angle );
    double yd = xc * sin( angle ) + yc * cos( angle );

    centerPt.setX( centerPt.x() + xd );
    centerPt.setY( centerPt.y() + yd );

    component.setCenter( centerPt );
    component.setSize( QgsPoint( label->getWidth(), label->getHeight() ) );

    drawLabelBackground( context, component, tmpLyr );
  }

  if ( drawType == QgsPalLabeling::LabelBuffer
       || drawType == QgsPalLabeling::LabelText )
  {

    // TODO: optimize access :)
    QString text = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->text();
    QString txt = ( label->getPartId() == -1 ? text : QString( text[label->getPartId()] ) );
    QFontMetricsF* labelfm = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->getLabelFontMetrics();

    QString wrapchr = !tmpLyr.wrapChar.isEmpty() ? tmpLyr.wrapChar : QString( "\n" );

    //add the direction symbol if needed
    if ( !txt.isEmpty() && tmpLyr.placement == QgsPalLayerSettings::Line &&
         tmpLyr.addDirectionSymbol )
    {
      bool prependSymb = false;
      QString symb = tmpLyr.rightDirectionSymbol;

      if ( label->getReversed() )
      {
        prependSymb = true;
        symb = tmpLyr.leftDirectionSymbol;
      }

      if ( tmpLyr.reverseDirectionSymbol )
      {
        if ( symb == tmpLyr.rightDirectionSymbol )
        {
          prependSymb = true;
          symb = tmpLyr.leftDirectionSymbol;
        }
        else
        {
          prependSymb = false;
          symb = tmpLyr.rightDirectionSymbol;
        }
      }

      if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolAbove )
      {
        prependSymb = true;
        symb = symb + wrapchr;
      }
      else if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolBelow )
      {
        prependSymb = false;
        symb = wrapchr + symb;
      }

      if ( prependSymb )
      {
        txt.prepend( symb );
      }
      else
      {
        txt.append( symb );
      }
    }

    //QgsDebugMsg( "drawLabel " + txt );

    QStringList multiLineList = txt.split( wrapchr );
    int lines = multiLineList.size();

    double labelWidest = 0.0;
    for ( int i = 0; i < lines; ++i )
    {
      double labelWidth = labelfm->width( multiLineList.at( i ) );
      if ( labelWidth > labelWidest )
      {
        labelWidest = labelWidth;
      }
    }

    double labelHeight = labelfm->ascent() + labelfm->descent(); // ignore +1 for baseline
    //  double labelHighest = labelfm->height() + ( double )(( lines - 1 ) * labelHeight * tmpLyr.multilineHeight );

    // needed to move bottom of text's descender to within bottom edge of label
    double ascentOffset = 0.25 * labelfm->ascent(); // labelfm->descent() is not enough

    for ( int i = 0; i < lines; ++i )
    {
      painter->save();
      painter->translate( QPointF( outPt.x(), outPt.y() ) );
      painter->rotate( -label->getAlpha() * 180 / M_PI );

      // scale down painter: the font size has been multiplied by raster scale factor
      // to workaround a Qt font scaling bug with small font sizes
      painter->scale( 1.0 / tmpLyr.rasterCompressFactor, 1.0 / tmpLyr.rasterCompressFactor );

      // figure x offset for horizontal alignment of multiple lines
      double xMultiLineOffset = 0.0;
      double labelWidth = labelfm->width( multiLineList.at( i ) );
      if ( lines > 1 && tmpLyr.multilineAlign != QgsPalLayerSettings::MultiLeft )
      {
        double labelWidthDiff = labelWidest - labelWidth;
        if ( tmpLyr.multilineAlign == QgsPalLayerSettings::MultiCenter )
        {
          labelWidthDiff /= 2;
        }
        xMultiLineOffset = labelWidthDiff;
        //QgsDebugMsg( QString( "xMultiLineOffset: %1" ).arg( xMultiLineOffset ) );
      }

      double yMultiLineOffset = ( lines - 1 - i ) * labelHeight * tmpLyr.multilineHeight;
      painter->translate( QPointF( xMultiLineOffset, - ascentOffset - yMultiLineOffset ) );

      component.setText( multiLineList.at( i ) );
      component.setSize( QgsPoint( labelWidth, labelHeight ) );
      component.setOffset( QgsPoint( 0.0, -ascentOffset ) );
      component.setRotation( -component.rotation() * 180 / M_PI );
      component.setRotationOffset( 0.0 );

      if ( drawType == QgsPalLabeling::LabelBuffer )
      {
        // draw label's buffer
        drawLabelBuffer( context, component, tmpLyr );
      }
      else
      {
        // draw label's text, QPainterPath method
        QPainterPath path;
        path.addText( 0, 0, tmpLyr.textFont, component.text() );

        // store text's drawing in QPicture for drop shadow call
        QPicture textPict;
        QPainter textp;
        textp.begin( &textPict );
        textp.setPen( Qt::NoPen );
        textp.setBrush( tmpLyr.textColor );
        textp.drawPath( path );
        textp.end();

        if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowText )
        {
          component.setPicture( &textPict );
          component.setPictureBuffer( 0.0 ); // no pen width to deal with
          component.setOrigin( QgsPoint( 0.0, 0.0 ) );

          drawLabelShadow( context, component, tmpLyr );
        }

        // paint the text
        painter->setCompositionMode( tmpLyr.blendMode );
//        painter->setPen( Qt::NoPen );
//        painter->setBrush( tmpLyr.textColor );
//        painter->drawPath( path );

        // scale for any print output or image saving @ specific dpi
        painter->scale( component.dpiRatio(), component.dpiRatio() );
        painter->drawPicture( 0, 0, textPict );

        // regular text draw, for testing optimization
//        painter->setFont( tmpLyr.textFont );
//        painter->setPen( tmpLyr.textColor );
//        painter->drawText( 0, 0, multiLineList.at( i ) );

      }
      painter->restore();
    }
  }

  // NOTE: this used to be within above multi-line loop block, at end. (a mistake since 2010? [LS])
  if ( label->getNextPart() )
    drawLabel( label->getNextPart(), context, tmpLyr, drawType );
}

void QgsPalLabeling::drawLabelBuffer( QgsRenderContext& context,
                                      QgsLabelComponent component,
                                      const QgsPalLayerSettings& tmpLyr )
{
  QPainter* p = context.painter();

  double penSize = tmpLyr.scaleToPixelContext( tmpLyr.bufferSize, context,
                   ( tmpLyr.bufferSizeInMapUnits ? QgsPalLayerSettings::MapUnits : QgsPalLayerSettings::MM ), true );

  QPainterPath path;
  path.addText( 0, 0, tmpLyr.textFont, component.text() );
  QPen pen( tmpLyr.bufferColor );
  pen.setWidthF( penSize );
  pen.setJoinStyle( tmpLyr.bufferJoinStyle );
  QColor tmpColor( tmpLyr.bufferColor );
  // honor pref for whether to fill buffer interior
  if ( tmpLyr.bufferNoFill )
  {
    tmpColor.setAlpha( 0 );
  }

  // store buffer's drawing in QPicture for drop shadow call
  QPicture buffPict;
  QPainter buffp;
  buffp.begin( &buffPict );
  buffp.setPen( pen );
  buffp.setBrush( tmpColor );
  buffp.drawPath( path );
  buffp.end();

  if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowBuffer )
  {
    component.setOrigin( QgsPoint( 0.0, 0.0 ) );
    component.setPicture( &buffPict );
    component.setPictureBuffer( penSize / 2.0 );

    drawLabelShadow( context, component, tmpLyr );
  }

  p->save();
  p->setCompositionMode( tmpLyr.bufferBlendMode );
//  p->setPen( pen );
//  p->setBrush( tmpColor );
//  p->drawPath( path );

  // scale for any print output or image saving @ specific dpi
  p->scale( component.dpiRatio(), component.dpiRatio() );
  p->drawPicture( 0, 0, buffPict );
  p->restore();
}

void QgsPalLabeling::drawLabelBackground( QgsRenderContext& context,
    QgsLabelComponent component,
    const QgsPalLayerSettings& tmpLyr )
{
  QPainter* p = context.painter();
  double labelWidth = component.size().x(), labelHeight = component.size().y();
  //QgsDebugMsg( QString( "Background label rotation: %1" ).arg( component.rotation() ) );

  // shared calculations between shapes and SVG

  // configure angles, set component rotation and rotationOffset
  if ( tmpLyr.shapeRotationType != QgsPalLayerSettings::RotationFixed )
  {
    component.setRotation( -( component.rotation() * 180 / M_PI ) ); // RotationSync
    component.setRotationOffset(
      tmpLyr.shapeRotationType == QgsPalLayerSettings::RotationOffset ? tmpLyr.shapeRotation : 0.0 );
  }
  else // RotationFixed
  {
    component.setRotation( 0.0 ); // don't use label's rotation
    component.setRotationOffset( tmpLyr.shapeRotation );
  }

  // mm to map units conversion factor
  double mmToMapUnits = context.mapToPixel().mapUnitsPerPixel() * context.scaleFactor();

  // TODO: the following label-buffered generated shapes and SVG symbols should be moved into marker symbology classes

  if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeSVG )
  {
    // all calculations done in shapeSizeUnits, which are then passed to symbology class for painting

    if ( tmpLyr.shapeSVGFile.isEmpty() )
      return;

    double sizeOut = 0.0;
    // only one size used for SVG sizing/scaling (no use of shapeSize.y() or Y field in gui)
    if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeFixed )
    {
      sizeOut = tmpLyr.shapeSize.x();
    }
    else if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeBuffer )
    {
      // add buffer to greatest dimension of label
      if ( labelWidth >= labelHeight )
        sizeOut = labelWidth;
      else if ( labelHeight > labelWidth )
        sizeOut = labelHeight;

      // label size in map units, convert to shapeSizeUnits, if different
      if ( tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MM )
      {
        sizeOut /= mmToMapUnits;
      }

      // add buffer
      sizeOut += tmpLyr.shapeSize.x() * 2;
    }

    // don't bother rendering symbols smaller than 1x1 pixels in size
    // TODO: add option to not show any svgs under/over a certian size
    if ( tmpLyr.scaleToPixelContext( sizeOut, context, tmpLyr.shapeSizeUnits ) < 1.0 )
      return;

    QgsStringMap map; // for SVG symbology marker
    map["name"] = tmpLyr.shapeSVGFile;
    map["size"] = QString::number( sizeOut );
    map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
                         tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );
    map["angle"] = QString::number( 0.0 ); // angle is handled by this local painter

    // offset is handled by this local painter
    // TODO: see why the marker renderer doesn't seem to translate offset *after* applying rotation
    //map["offset"] = QgsSymbolLayerV2Utils::encodePoint( tmpLyr.shapeOffset );
    //map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
    //                       tmpLyr.shapeOffsetUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );

    map["fill"] = tmpLyr.shapeFillColor.name();
    map["outline"] = tmpLyr.shapeBorderColor.name();
    map["outline-width"] = QString::number( tmpLyr.shapeBorderWidth );

    // TODO: fix overriding SVG symbol's border width/units in QgsSvgCache
    // currently broken, fall back to symbol's
    //map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
    //                              tmpLyr.shapeBorderWidthUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );

    if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowShape )
    {
      // configure SVG shadow specs
      QgsStringMap shdwmap( map );
      shdwmap["fill"] = tmpLyr.shadowColor.name();
      shdwmap["outline"] = tmpLyr.shadowColor.name();
      shdwmap["size"] = QString::number( sizeOut * tmpLyr.rasterCompressFactor );

      // store SVG's drawing in QPicture for drop shadow call
      QPicture svgPict;
      QPainter svgp;
      svgp.begin( &svgPict );

      // draw shadow symbol

      // clone current render context map unit/mm conversion factors, but not
      // other map canvas parameters, then substitute this painter for use in symbology painting
      // NOTE: this is because the shadow needs to be scaled correctly for output to map canvas,
      //       but will be created relative to the SVG's computed size, not the current map canvas
      QgsRenderContext shdwContext;
      shdwContext.setMapToPixel( context.mapToPixel() );
      shdwContext.setScaleFactor( context.scaleFactor() );
      shdwContext.setPainter( &svgp );

      QgsSymbolLayerV2* symShdwL = QgsSvgMarkerSymbolLayerV2::create( shdwmap );
      QgsSvgMarkerSymbolLayerV2* svgShdwM = static_cast<QgsSvgMarkerSymbolLayerV2*>( symShdwL );
      QgsSymbolV2RenderContext svgShdwContext( shdwContext, QgsSymbolV2::Mixed,
          ( 100.0 - ( double )( tmpLyr.shapeTransparency ) ) / 100.0 );

      double svgSize = tmpLyr.scaleToPixelContext( sizeOut, context, tmpLyr.shapeSizeUnits, true );
      svgShdwM->renderPoint( QPointF( svgSize / 2, -svgSize / 2 ), svgShdwContext );
      svgp.end();

      component.setPicture( &svgPict );
      // TODO: when SVG symbol's border width/units is fixed in QgsSvgCache, adjust for it here
      component.setPictureBuffer( 0.0 );

      component.setSize( QgsPoint( svgSize, svgSize ) );
      component.setOffset( QgsPoint( 0.0, 0.0 ) );

      // rotate about origin center of SVG
      p->save();
      p->translate( component.center().x(), component.center().y() );
      p->rotate( component.rotation() );
      p->scale( 1.0 / tmpLyr.rasterCompressFactor, 1.0 / tmpLyr.rasterCompressFactor );
      double xoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.x(), context, tmpLyr.shapeOffsetUnits, true );
      double yoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.y(), context, tmpLyr.shapeOffsetUnits, true );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset() );
      p->translate( -svgSize / 2, svgSize / 2 );

      drawLabelShadow( context, component, tmpLyr );
      p->restore();

      delete svgShdwM;
      svgShdwM = 0;
    }

    // draw the actual symbol
    QgsSymbolLayerV2* symL = QgsSvgMarkerSymbolLayerV2::create( map );
    QgsSvgMarkerSymbolLayerV2* svgM = static_cast<QgsSvgMarkerSymbolLayerV2*>( symL );
    QgsSymbolV2RenderContext svgContext( context, QgsSymbolV2::Mixed,
                                         ( 100.0 - ( double )( tmpLyr.shapeTransparency ) ) / 100.0 );

    p->save();
    p->setCompositionMode( tmpLyr.shapeBlendMode );
    p->translate( component.center().x(), component.center().y() );
    p->rotate( component.rotation() );
    double xoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.x(), context, tmpLyr.shapeOffsetUnits );
    double yoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.y(), context, tmpLyr.shapeOffsetUnits );
    p->translate( QPointF( xoff, yoff ) );
    p->rotate( component.rotationOffset() );
    svgM->renderPoint( QPointF( 0, 0 ), svgContext );
    p->setCompositionMode( QPainter::CompositionMode_SourceOver ); // just to be sure
    p->restore();

    delete svgM;
    svgM = 0;

  }
  else  // Generated Shapes
  {
    // all calculations done in shapeSizeUnits

    double w = labelWidth / ( tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MM ? mmToMapUnits : 1 );
    double h = labelHeight / ( tmpLyr.shapeSizeUnits == QgsPalLayerSettings::MM ? mmToMapUnits : 1 );

    double xsize = tmpLyr.shapeSize.x();
    double ysize = tmpLyr.shapeSize.y();

    if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeFixed )
    {
      w = xsize;
      h = ysize;
    }
    else if ( tmpLyr.shapeSizeType == QgsPalLayerSettings::SizeBuffer )
    {
      if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeSquare )
      {
        if ( w > h )
          h = w;
        else if ( h > w )
          w = h;
      }
      else if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeCircle )
      {
        // start with label bound by circle
        h = sqrt( pow( w, 2 ) + pow( h, 2 ) );
        w = h;
      }
      else if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeEllipse )
      {
        // start with label bound by ellipse
        h = h / sqrt( 2.0 ) * 2;
        w = w / sqrt( 2.0 ) * 2;
      }

      w += xsize * 2;
      h += ysize * 2;
    }

    // convert everything over to map pixels from here on
    w = tmpLyr.scaleToPixelContext( w, context, tmpLyr.shapeSizeUnits, true );
    h = tmpLyr.scaleToPixelContext( h, context, tmpLyr.shapeSizeUnits, true );

    // offsets match those of symbology: -x = left, -y = up
    QRectF rect( -w / 2.0, - h / 2.0, w, h );

    if ( rect.isNull() )
      return;

    p->save();
    p->translate( QPointF( component.center().x(), component.center().y() ) );
    p->rotate( component.rotation() );
    double xoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.x(), context, tmpLyr.shapeOffsetUnits, true );
    double yoff = tmpLyr.scaleToPixelContext( tmpLyr.shapeOffset.y(), context, tmpLyr.shapeOffsetUnits, true );
    p->translate( QPointF( xoff, yoff ) );
    p->rotate( component.rotationOffset() );

    double penSize = tmpLyr.scaleToPixelContext( tmpLyr.shapeBorderWidth, context, tmpLyr.shapeBorderWidthUnits, true );

    QPen pen;
    if ( tmpLyr.shapeBorderWidth > 0 )
    {
      pen.setColor( tmpLyr.shapeBorderColor );
      pen.setWidthF( penSize );
      if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeRectangle )
        pen.setJoinStyle( tmpLyr.shapeJoinStyle );
    }
    else
    {
      pen = Qt::NoPen;
    }

    // store painting in QPicture for shadow drawing
    QPicture shapePict;
    QPainter shapep;
    shapep.begin( &shapePict );
    shapep.setPen( pen );
    shapep.setBrush( tmpLyr.shapeFillColor );

    if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeRectangle
         || tmpLyr.shapeType == QgsPalLayerSettings::ShapeSquare )
    {
      if ( tmpLyr.shapeRadiiUnits == QgsPalLayerSettings::Percent )
      {
        shapep.drawRoundedRect( rect, tmpLyr.shapeRadii.x(), tmpLyr.shapeRadii.y(), Qt::RelativeSize );
      }
      else
      {
        double xRadius = tmpLyr.scaleToPixelContext( tmpLyr.shapeRadii.x(), context, tmpLyr.shapeRadiiUnits, true );
        double yRadius = tmpLyr.scaleToPixelContext( tmpLyr.shapeRadii.y(), context, tmpLyr.shapeRadiiUnits, true );
        shapep.drawRoundedRect( rect, xRadius, yRadius );
      }
    }
    else if ( tmpLyr.shapeType == QgsPalLayerSettings::ShapeEllipse
              || tmpLyr.shapeType == QgsPalLayerSettings::ShapeCircle )
    {
      shapep.drawEllipse( rect );
    }
    shapep.end();

    p->scale( 1.0 / tmpLyr.rasterCompressFactor, 1.0 / tmpLyr.rasterCompressFactor );

    if ( tmpLyr.shadowDraw && tmpLyr.shadowUnder == QgsPalLayerSettings::ShadowShape )
    {
      component.setPicture( &shapePict );
      component.setPictureBuffer( penSize / 2.0 );

      component.setSize( QgsPoint( rect.width(), rect.height() ) );
      component.setOffset( QgsPoint( rect.width() / 2, -rect.height() / 2 ) );
      drawLabelShadow( context, component, tmpLyr );
    }

    p->setOpacity(( 100.0 - ( double )( tmpLyr.shapeTransparency ) ) / 100.0 );
    p->setCompositionMode( tmpLyr.shapeBlendMode );

    // scale for any print output or image saving @ specific dpi
    p->scale( component.dpiRatio(), component.dpiRatio() );
    p->drawPicture( 0, 0, shapePict );
    p->restore();
  }
}

void QgsPalLabeling::drawLabelShadow( QgsRenderContext& context,
                                      QgsLabelComponent component,
                                      const QgsPalLayerSettings& tmpLyr )
{
  // incoming component sizes should be multiplied by rasterCompressFactor, as
  // this allows shadows to be created at paint device dpi (e.g. high resolution),
  // then scale device painter by 1.0 / rasterCompressFactor for output

  QPainter* p = context.painter();
  double componentWidth = component.size().x(), componentHeight = component.size().y();
  double xOffset = component.offset().x(), yOffset = component.offset().y();
  double pictbuffer = component.pictureBuffer();

  // generate pixmap representation of label component drawing
  bool mapUnits = ( tmpLyr.shadowRadiusUnits == QgsPalLayerSettings::MapUnits );
  double radius = tmpLyr.scaleToPixelContext( tmpLyr.shadowRadius , context, tmpLyr.shadowRadiusUnits, !mapUnits );
  radius /= ( mapUnits ? tmpLyr.vectorScaleFactor / component.dpiRatio() : 1 );
  radius = ( int )( radius + 0.5 );

  // TODO: add labeling gui option to adjust blurBufferClippingScale to minimize pixels, or
  //       to ensure shadow isn't clipped too tight. (Or, find a better method of buffering)
  double blurBufferClippingScale = 3.75;
  int blurbuffer = ( radius > 17 ? 16 : radius ) * blurBufferClippingScale;

  QImage blurImg( componentWidth + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  componentHeight + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  QImage::Format_ARGB32_Premultiplied );

  // TODO: add labeling gui option to not show any shadows under/over a certian size
  // keep very small QImages from causing paint device issues, i.e. must be at least > 1
  int minBlurImgSize = 1;
  // max limitation on QgsSvgCache is 10,000 for screen, which will probably be reasonable for future caching here, too
  // 4 x QgsSvgCache limit for output to print/image at higher dpi
  // TODO: should it be higher, scale with dpi, or have no limit? Needs testing with very large labels rendered at high dpi output
  int maxBlurImgSize = 40000;
  if ( blurImg.isNull()
       || ( blurImg.width() < minBlurImgSize || blurImg.height() < minBlurImgSize )
       || ( blurImg.width() > maxBlurImgSize || blurImg.height() > maxBlurImgSize ) )
    return;

  blurImg.fill( QColor( Qt::transparent ).rgba() );
  QPainter pictp;
  if ( !pictp.begin( &blurImg ) )
    return;
  pictp.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  QPointF imgOffset( blurbuffer + pictbuffer + xOffset,
                     blurbuffer + pictbuffer + componentHeight + yOffset );

  pictp.drawPicture( imgOffset,
                     *component.picture() );

  // overlay shadow color
  pictp.setCompositionMode( QPainter::CompositionMode_SourceIn );
  pictp.fillRect( blurImg.rect(), tmpLyr.shadowColor );
  pictp.end();

  // blur the QImage in-place
  if ( tmpLyr.shadowRadius > 0.0 && radius > 0 )
  {
    QgsSymbolLayerV2Utils::blurImageInPlace( blurImg, blurImg.rect(), radius, tmpLyr.shadowRadiusAlphaOnly );
  }

  if ( tmpLyr.showingShadowRects ) // engine setting, not per layer
  {
    // debug rect for QImage shadow registration and clipping visualization
    QPainter picti;
    picti.begin( &blurImg );
    picti.setBrush( Qt::Dense7Pattern );
    QPen imgPen( QColor( 0, 0, 255, 255 ) );
    imgPen.setWidth( 1 );
    picti.setPen( imgPen );
    picti.setOpacity( 0.1 );
    picti.drawRect( 0, 0, blurImg.width(), blurImg.height() );
    picti.end();
  }

  double offsetDist = tmpLyr.scaleToPixelContext( tmpLyr.shadowOffsetDist, context, tmpLyr.shadowOffsetUnits, true );
  double angleRad = tmpLyr.shadowOffsetAngle * M_PI / 180; // to radians
  if ( tmpLyr.shadowOffsetGlobal )
  {
    // TODO: check for differences in rotation origin and cw/ccw direction,
    //       when this shadow function is used for something other than labels

    // it's 0-->cw-->360 for labels
    //QgsDebugMsg( QString( "Shadow aggregated label rotation (degrees): %1" ).arg( component.rotation() + component.rotationOffset() ) );
    angleRad -= ( component.rotation() * M_PI / 180 + component.rotationOffset() * M_PI / 180 );
  }

  QPointF transPt( -offsetDist * cos( angleRad + M_PI / 2 ),
                   -offsetDist * sin( angleRad + M_PI / 2 ) );

  p->save();
  p->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  p->setCompositionMode( tmpLyr.shadowBlendMode );
  p->setOpacity(( 100.0 - ( double )( tmpLyr.shadowTransparency ) ) / 100.0 );

  double scale = ( double )tmpLyr.shadowScale / 100.0;
  // TODO: scale from center/center, left/center or left/top, instead of default left/bottom?
  p->scale( scale, scale );
  if ( component.useOrigin() )
  {
    p->translate( component.origin().x(), component.origin().y() );
  }
  p->translate( transPt );
  p->translate( -imgOffset.x(),
                -imgOffset.y() );
  p->drawImage( 0, 0, blurImg );
  p->restore();

  // debug rects
  if ( tmpLyr.showingShadowRects ) // engine setting, not per layer
  {
    // draw debug rect for QImage painting registration
    p->save();
    p->setBrush( Qt::NoBrush );
    QPen imgPen( QColor( 255, 0, 0, 10 ) );
    imgPen.setWidth( 2 );
    imgPen.setStyle( Qt::DashLine );
    p->setPen( imgPen );
    p->scale( scale, scale );
    if ( component.useOrigin() )
    {
      p->translate( component.origin().x(), component.origin().y() );
    }
    p->translate( transPt );
    p->translate( -imgOffset.x(),
                  -imgOffset.y() );
    p->drawRect( 0, 0, blurImg.width(), blurImg.height() );
    p->restore();

    // draw debug rect for passed in component dimensions
    p->save();
    p->setBrush( Qt::NoBrush );
    QPen componentRectPen( QColor( 0, 255, 0, 70 ) );
    componentRectPen.setWidth( 1 );
    if ( component.useOrigin() )
    {
      p->translate( component.origin().x(), component.origin().y() );
    }
    p->setPen( componentRectPen );
    p->drawRect( QRect( -xOffset, -componentHeight - yOffset, componentWidth, componentHeight ) );
    p->restore();
  }
}

void QgsPalLabeling::loadEngineSettings()
{
  // start with engine defaults for new project, or project that has no saved settings
  Pal p;
  bool saved = false;
  mSearch = ( QgsPalLabeling::Search )( QgsProject::instance()->readNumEntry(
                                          "PAL", "/SearchMethod", ( int )p.getSearch(), &saved ) );
  mCandPoint = QgsProject::instance()->readNumEntry(
                 "PAL", "/CandidatesPoint", p.getPointP(), &saved );
  mCandLine = QgsProject::instance()->readNumEntry(
                "PAL", "/CandidatesLine", p.getLineP(), &saved );
  mCandPolygon = QgsProject::instance()->readNumEntry(
                   "PAL", "/CandidatesPolygon", p.getPolyP(), &saved );
  mShowingCandidates = QgsProject::instance()->readBoolEntry(
                         "PAL", "/ShowingCandidates", false, &saved );
  mShowingShadowRects = QgsProject::instance()->readBoolEntry(
                          "PAL", "/ShowingShadowRects", false, &saved );
  mShowingAllLabels = QgsProject::instance()->readBoolEntry(
                        "PAL", "/ShowingAllLabels", false, &saved );
  mSavedWithProject = saved;
}

void QgsPalLabeling::saveEngineSettings()
{
  QgsProject::instance()->writeEntry( "PAL", "/SearchMethod", ( int )mSearch );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPoint", mCandPoint );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesLine", mCandLine );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPolygon", mCandPolygon );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingCandidates", mShowingCandidates );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingShadowRects", mShowingShadowRects );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingAllLabels", mShowingAllLabels );
  mSavedWithProject = true;
}

void QgsPalLabeling::clearEngineSettings()
{
  QgsProject::instance()->removeEntry( "PAL", "/SearchMethod" );
  QgsProject::instance()->removeEntry( "PAL", "/CandidatesPoint" );
  QgsProject::instance()->removeEntry( "PAL", "/CandidatesLine" );
  QgsProject::instance()->removeEntry( "PAL", "/CandidatesPolygon" );
  QgsProject::instance()->removeEntry( "PAL", "/ShowingCandidates" );
  QgsProject::instance()->removeEntry( "PAL", "/ShowingShadowRects" );
  QgsProject::instance()->removeEntry( "PAL", "/ShowingAllLabels" );
  mSavedWithProject = false;
}

QgsLabelingEngineInterface* QgsPalLabeling::clone()
{
  QgsPalLabeling* lbl = new QgsPalLabeling();
  lbl->mShowingAllLabels = mShowingAllLabels;
  lbl->mShowingCandidates = mShowingCandidates;
  lbl->mShowingShadowRects = mShowingShadowRects;
  return lbl;
}
