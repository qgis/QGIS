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
#include <qgsproject.h>
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
    : palLayer( NULL ), ct( NULL ), extentGeom( NULL ), mFeaturesToLabel( 0 ), mFeatsSendingToPal( 0 ), mFeatsRegPal( 0 ), expression( NULL )
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
  blendMode = QgsMapRenderer::BlendNormal;
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
  bufferBlendMode = QgsMapRenderer::BlendNormal;
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
  placement = ( Placement ) layer->customProperty( "labeling/placement" ).toInt();
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
  textFont.setCapitalization(( QFont::Capitalization ) layer->customProperty( "labeling/fontCapitals", QVariant( 0 ) ).toUInt() );
  textFont.setUnderline( layer->customProperty( "labeling/fontUnderline" ).toBool() );
  textFont.setStrikeOut( layer->customProperty( "labeling/fontStrikeout" ).toBool() );
  textFont.setLetterSpacing( QFont::AbsoluteSpacing, layer->customProperty( "labeling/fontLetterSpacing", QVariant( 0.0 ) ).toDouble() );
  textFont.setWordSpacing( layer->customProperty( "labeling/fontWordSpacing", QVariant( 0.0 ) ).toDouble() );
  textColor = _readColor( layer, "labeling/textColor" );
  textTransp = layer->customProperty( "labeling/textTransp" ).toInt();
  blendMode = ( QgsMapRenderer::BlendMode ) layer->customProperty( "labeling/blendMode" ).toInt();
  previewBkgrdColor = QColor( layer->customProperty( "labeling/previewBkgrdColor", QVariant( "#ffffff" ) ).toString() );
  enabled = layer->customProperty( "labeling/enabled" ).toBool();
  priority = layer->customProperty( "labeling/priority" ).toInt();
  obstacle = layer->customProperty( "labeling/obstacle" ).toBool();
  dist = layer->customProperty( "labeling/dist" ).toDouble();
  scaleMin = layer->customProperty( "labeling/scaleMin" ).toInt();
  scaleMax = layer->customProperty( "labeling/scaleMax" ).toInt();
  bufferSize = layer->customProperty( "labeling/bufferSize" ).toDouble();
  bufferColor = _readColor( layer, "labeling/bufferColor" );
  bufferTransp = layer->customProperty( "labeling/bufferTransp" ).toInt();
  bufferBlendMode = ( QgsMapRenderer::BlendMode ) layer->customProperty( "labeling/bufferBlendMode" ).toInt();
  bufferJoinStyle = ( Qt::PenJoinStyle ) layer->customProperty( "labeling/bufferJoinStyle", QVariant( Qt::BevelJoin ) ).toUInt();
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
  placeDirectionSymbol = ( DirectionSymbols ) layer->customProperty( "labeling/placeDirectionSymbol", QVariant( SymbolLeftRight ) ).toUInt();
  upsidedownLabels = ( UpsideDownLabels ) layer->customProperty( "labeling/upsidedownLabels", QVariant( Upright ) ).toUInt();
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
  multilineAlign = ( MultiLineAlign ) layer->customProperty( "labeling/multilineAlign", QVariant( MultiLeft ) ).toUInt();
  preserveRotation = layer->customProperty( "labeling/preserveRotation", QVariant( true ) ).toBool();
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
  layer->setCustomProperty( "labeling/blendMode", ( unsigned int )blendMode );
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
  layer->setCustomProperty( "labeling/bufferBlendMode", ( unsigned int )bufferBlendMode );
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
      text.append( dirSym + wrapchr ); // SymbolAbove or SymbolBelow
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
      if ( sizeDouble <= 0.0 || sizeToPixel( sizeDouble, context ) < 1 )
      {
        return;
      }
      labelFont.setPixelSize( sizeToPixel( sizeDouble, context ) );
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
  double mapUntsPerMM = context.mapToPixel().mapUnitsPerPixel() * context.scaleFactor();
  if ( xOffset != 0 )
  {
    offsetX = xOffset;
    if ( !labelOffsetInMapUnits )
    {
      offsetX = xOffset * mapUntsPerMM; //convert offset from mm to map units
    }
  }
  if ( yOffset != 0 )
  {
    offsetY = yOffset;
    if ( !labelOffsetInMapUnits )
    {
      offsetY = yOffset * mapUntsPerMM; //convert offset from mm to map units
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

int QgsPalLayerSettings::sizeToPixel( double size, const QgsRenderContext& c, bool buffer ) const
{
  double pixelSize;
  if (( !buffer && fontSizeInMapUnits ) || ( buffer && bufferSizeInMapUnits ) )
  {
    pixelSize = size / c.mapToPixel().mapUnitsPerPixel() * c.rasterScaleFactor();
  }
  else //font size in points, or buffer in mm
  {
    double ptsTomm = buffer ? 1 : 0.3527;
    // set font size from points to output size
    pixelSize = ptsTomm * size * c.scaleFactor() * c.rasterScaleFactor();
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
  int pixelFontSize = lyr.sizeToPixel( lyr.textFont.pointSizeF(), ctx );

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
    QFont fontForLabel = lyr.textFont;
    QColor fontColor = lyr.textColor;
    int fontTransp = lyr.textTransp;
    QgsMapRenderer::BlendMode blendMode = lyr.blendMode;
    double bufferSize = lyr.bufferSize;
    QColor bufferColor = lyr.bufferColor;
    int bufferTransp = lyr.bufferTransp;
    QgsMapRenderer::BlendMode bufferBlendMode = lyr.bufferBlendMode;

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
    //font transparency
    QVariant dataDefinedFontTransp = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::FontTransp );
    if ( dataDefinedFontTransp.isValid() )
    {
      bool ftOk = false;
      int ft = dataDefinedFontTransp.toInt( &ftOk );
      if ( ftOk && ft >= 0 && ft <= 100 )
      {
        fontTransp = ft;
      }
    }
    fontColor.setAlphaF(( 100.0 - ( double )( fontTransp ) ) / 100.0 );

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
    //buffer transparency
    QVariant dataDefinedBufTransp = palGeometry->dataDefinedValues().value( QgsPalLayerSettings::BufferTransp );
    if ( dataDefinedBufTransp.isValid() )
    {
      bool btOk = false;
      int bt = dataDefinedBufTransp.toInt( &btOk );
      if ( btOk && bt >= 0 && bt <= 100 )
      {
        bufferTransp = bt;
      }
    }
    bufferColor.setAlphaF(( 100.0 - ( double )( bufferTransp ) ) / 100.0 );

    if ( lyr.bufferSize != 0 )
    {
      // Set the painter composition mode for the buffer
      painter->setCompositionMode( mMapRenderer->getCompositionMode( bufferBlendMode ) );

      int bufferPixelSize = lyr.sizeToPixel( bufferSize, context, true );
      drawLabel( *it, painter, fontForLabel, fontColor, xform, bufferPixelSize, bufferColor, true );
    }

    // Set the painter composition mode before rendering the label
    painter->setCompositionMode( mMapRenderer->getCompositionMode( blendMode ) );

    drawLabel( *it, painter, fontForLabel, fontColor, xform );

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

void QgsPalLabeling::drawLabel( pal::LabelPosition* label, QPainter* painter, const QFont& f, const QColor& c, const QgsMapToPixel* xform, double bufferPixelSize,
                                const QColor& bufferColor, bool drawBuffer )
{
  QgsPoint outPt = xform->transform( label->getX(), label->getY() );

  // TODO: optimize access :)
  const QgsPalLayerSettings& lyr = layer( QString::fromUtf8( label->getLayerName() ) );
  QString text = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->text();
  QString txt = ( label->getPartId() == -1 ? text : QString( text[label->getPartId()] ) );
  QFontMetricsF* labelfm = (( QgsPalGeometry* )label->getFeaturePart()->getUserGeometry() )->getLabelFontMetrics();

  QString wrapchr = !lyr.wrapChar.isEmpty() ? lyr.wrapChar : QString( "\n" );

  //add the direction symbol if needed
  if ( !txt.isEmpty() && lyr.placement == QgsPalLayerSettings::Line &&
       lyr.addDirectionSymbol )
  {
    bool prependSymb = false;
    QString symb = lyr.rightDirectionSymbol;

    if ( label->getReversed() )
    {
      prependSymb = true;
      symb = lyr.leftDirectionSymbol;
    }

    if ( lyr.reverseDirectionSymbol )
    {
      if ( symb == lyr.rightDirectionSymbol )
      {
        prependSymb = true;
        symb = lyr.leftDirectionSymbol;
      }
      else
      {
        prependSymb = false;
        symb = lyr.rightDirectionSymbol;
      }
    }

    if ( lyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolAbove )
    {
      prependSymb = true;
      symb = symb + wrapchr;
    }
    else if ( lyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolBelow )
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

  //QgsDebugMsg( "drawLabel " + QString::number( drawBuffer ) + " " + txt );

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

  // needed to move bottom of text's descender to within bottom edge of label
  double ascentOffset = 0.25 * labelfm->ascent(); // labelfm->descent() is not enough

  for ( int i = 0; i < lines; ++i )
  {
    painter->save();
    painter->translate( QPointF( outPt.x(), outPt.y() ) );
    painter->rotate( -label->getAlpha() * 180 / M_PI );

    // scale down painter: the font size has been multiplied by raster scale factor
    // to workaround a Qt font scaling bug with small font sizes
    painter->scale( 1.0 / lyr.rasterCompressFactor, 1.0 / lyr.rasterCompressFactor );

    // figure x offset for horizontal alignment of multiple lines
    double xMultiLineOffset = 0.0;
    if ( lines > 1 && lyr.multilineAlign != QgsPalLayerSettings::MultiLeft )
    {
      double labelWidth = labelfm->width( multiLineList.at( i ) );
      double labelWidthDiff = labelWidest - labelWidth;
      if ( lyr.multilineAlign == QgsPalLayerSettings::MultiCenter )
      {
        labelWidthDiff /= 2;
      }
      xMultiLineOffset = labelWidthDiff;
      //QgsDebugMsg( QString( "xMultiLineOffset: %1" ).arg( xMultiLineOffset ) );
    }

    double yMultiLineOffset = ( lines - 1 - i ) * labelHeight * lyr.multilineHeight;
    painter->translate( QPointF( xMultiLineOffset, - ascentOffset - yMultiLineOffset ) );

    if ( drawBuffer )
    {
      // we're drawing buffer
      //drawLabelBuffer( painter, multiLineList.at( i ), f, bufferSize * lyr.vectorScaleFactor * lyr.rasterCompressFactor , bufferColor );
      drawLabelBuffer( painter, multiLineList.at( i ), f,  bufferPixelSize , bufferColor, lyr.bufferJoinStyle, lyr.bufferNoFill );
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
      drawLabel( label->getNextPart(), painter, f, c, xform, bufferPixelSize, bufferColor, drawBuffer );
  }
}


void QgsPalLabeling::drawLabelBuffer( QPainter* p, QString text, const QFont& font, double size, QColor color, Qt::PenJoinStyle joinstyle, bool noFill )
{
  QPainterPath path;
  path.addText( 0, 0, font, text );
  QPen pen( color );
  pen.setWidthF( size );
  pen.setJoinStyle( joinstyle );
  p->setPen( pen );
  // honor pref for whether to fill buffer
  if ( noFill )
  {
    color.setAlpha( 0 );
  }
  p->setBrush( color );
  p->drawPath( path );
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
  QgsProject::instance()->removeEntry( "PAL", "/ShowingAllLabels" );
  mSavedWithProject = false;
}

QgsLabelingEngineInterface* QgsPalLabeling::clone()
{
  QgsPalLabeling* lbl = new QgsPalLabeling();
  lbl->mShowingAllLabels = mShowingAllLabels;
  lbl->mShowingCandidates = mShowingCandidates;
  return lbl;
}
