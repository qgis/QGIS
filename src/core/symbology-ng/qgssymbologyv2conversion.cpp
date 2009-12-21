#include "qgssymbologyv2conversion.h"

#include "qgssinglesymbolrenderer.h"
#include "qgsgraduatedsymbolrenderer.h"
#include "qgsuniquevaluerenderer.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"

#include "qgslogger.h"

#include "qgsmarkersymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2.h"

// some ad-hoc conversions
#define MM2PIXELS(x) ((x)/0.26)
#define PIXELS2MM(x) ((x)*0.26)


QgsSymbolV2* QgsSymbologyV2Conversion::symbolV1toV2( const QgsSymbol* s )
{
  switch ( s->type() )
  {
    case QGis::Point:
    {
      QgsMarkerSymbolLayerV2* sl = NULL;
      double size = MM2PIXELS( s->pointSize() );
      double angle = 0; // rotation only from classification field
      QString symbolName = s->pointSymbolName();
      if ( symbolName.startsWith( "hard:" ) )
      {
        // simple symbol marker
        QColor color = s->fillColor();
        QColor borderColor = s->color();
        QString name = symbolName.mid( 5 );
        sl = new QgsSimpleMarkerSymbolLayerV2( name, color, borderColor, size, angle );
      }
      else
      {
        // svg symbol marker
        QString name = symbolName.mid( 4 );
        sl = new QgsSvgMarkerSymbolLayerV2( name, size, angle );
      }
      QgsSymbolLayerV2List layers;
      layers.append( sl );
      return new QgsMarkerSymbolV2( layers );
    }

    case QGis::Line:
    {
      QColor color = s->color();
      double width = MM2PIXELS( s->lineWidth() );
      Qt::PenStyle penStyle = s->pen().style();
      QgsLineSymbolLayerV2* sl = new QgsSimpleLineSymbolLayerV2( color, width, penStyle );

      QgsSymbolLayerV2List layers;
      layers.append( sl );
      return new QgsLineSymbolV2( layers );
    }

    case QGis::Polygon:
    {
      QColor color = s->fillColor();
      QColor borderColor = s->color();
      Qt::BrushStyle brushStyle = s->brush().style();
      Qt::PenStyle borderStyle = s->pen().style();
      double borderWidth = MM2PIXELS( s->lineWidth() );
      QgsFillSymbolLayerV2* sl = new QgsSimpleFillSymbolLayerV2( color, brushStyle, borderColor, borderStyle, borderWidth );

      QgsSymbolLayerV2List layers;
      layers.append( sl );
      return new QgsFillSymbolV2( layers );
    }

    default:
      return NULL;
  }
}

QgsSymbol* QgsSymbologyV2Conversion::symbolV2toV1( QgsSymbolV2* s )
{
  if ( s == NULL || s->symbolLayerCount() == 0 )
    return NULL;

  // we will use only the first symbol layer
  QgsSymbolLayerV2* sl = s->symbolLayer( 0 );

  switch ( sl->type() )
  {
    case QgsSymbolV2::Marker:
    {
      QgsMarkerSymbolLayerV2* msl = static_cast<QgsMarkerSymbolLayerV2*>( sl );
      QgsSymbol* sOld = new QgsSymbol( QGis::Point );
      sOld->setFillColor( sl->color() );
      sOld->setFillStyle( Qt::SolidPattern );
      sOld->setPointSize( PIXELS2MM( msl->size() ) );
      if ( sl->layerType() == "SimpleMarker" )
      {
        QgsSimpleMarkerSymbolLayerV2* smsl = static_cast<QgsSimpleMarkerSymbolLayerV2*>( sl );
        sOld->setColor( smsl->borderColor() );
        sOld->setNamedPointSymbol( "hard:" + smsl->name() );
      }
      else if ( sl->layerType() == "SvgMarker" )
      {
        QgsSvgMarkerSymbolLayerV2* smsl = static_cast<QgsSvgMarkerSymbolLayerV2*>( sl );
        sOld->setNamedPointSymbol( "svg:" + smsl->path() );
      }
      return sOld;
    }
    break;

    case QgsSymbolV2::Line:
    {
      QgsLineSymbolLayerV2* lsl = static_cast<QgsLineSymbolLayerV2*>( sl );
      QgsSymbol* sOld = new QgsSymbol( QGis::Line );
      sOld->setColor( sl->color() );
      sOld->setLineWidth( PIXELS2MM( lsl->width() ) );
      if ( sl->layerType() == "SimpleLine" )
      {
        // add specific settings
        QgsSimpleLineSymbolLayerV2* slsl = static_cast<QgsSimpleLineSymbolLayerV2*>( sl );
        sOld->setLineStyle( slsl->penStyle() );
      }
      return sOld;
    }

    case QgsSymbolV2::Fill:
    {
      QgsSymbol* sOld = new QgsSymbol( QGis::Polygon );
      sOld->setFillColor( sl->color() );
      if ( sl->layerType() == "SimpleFill" )
      {
        // add specifc settings
        QgsSimpleFillSymbolLayerV2* sfsl = static_cast<QgsSimpleFillSymbolLayerV2*>( sl );
        sOld->setColor( sfsl->borderColor() );
        sOld->setLineWidth( PIXELS2MM( sfsl->borderWidth() ) );
        sOld->setLineStyle( sfsl->borderStyle() );
        sOld->setFillStyle( sfsl->brushStyle() );
      }
      return sOld;
    }
  }

  return NULL; // should never get here
}

void QgsSymbologyV2Conversion::rendererV1toV2( QgsVectorLayer* layer )
{
  if ( layer->isUsingRendererV2() )
    return;

  const QgsRenderer* r = layer->renderer();
  if ( r == NULL )
    return;

  QgsFeatureRendererV2* r2final = NULL;

  QString rtype = r->name();
  if ( rtype == "Single Symbol" )
  {
    const QgsSingleSymbolRenderer* ssr = dynamic_cast<const QgsSingleSymbolRenderer*>( r );
    if ( ssr == NULL )
      return;
    QgsSymbolV2* symbol = symbolV1toV2( ssr->symbol() );
    QgsSingleSymbolRendererV2* r2 = new QgsSingleSymbolRendererV2( symbol );
    r2final = r2;
  }
  else if ( rtype == "Graduated Symbol" )
  {
    const QgsGraduatedSymbolRenderer* gsr = dynamic_cast<const QgsGraduatedSymbolRenderer*>( r );
    if ( gsr == NULL )
      return;

    QString attrName;
    if ( layer->pendingFields().contains( gsr->classificationField() ) )
    {
      attrName = layer->pendingFields()[ gsr->classificationField()].name();
    }

    QgsRangeList ranges;
    foreach( const QgsSymbol* sym, gsr->symbols() )
    {
      double lowerValue = sym->lowerValue().toDouble();
      double upperValue = sym->upperValue().toDouble();
      QString label = sym->label();
      if ( label.isEmpty() )
        label = QString( "%1 - %2" ).arg( lowerValue, -1, 'f', 3 ).arg( upperValue, -1, 'f', 3 );
      QgsSymbolV2* symbolv2 = symbolV1toV2( sym );
      ranges.append( QgsRendererRangeV2( lowerValue, upperValue, symbolv2, label ) );
    }

    QgsGraduatedSymbolRendererV2* r2 = new QgsGraduatedSymbolRendererV2( attrName, ranges );

    // find out mode
    QgsGraduatedSymbolRendererV2::Mode m = QgsGraduatedSymbolRendererV2::Custom;
    switch ( gsr->mode() )
    {
      case QgsGraduatedSymbolRenderer::EqualInterval: m = QgsGraduatedSymbolRendererV2::EqualInterval; break;
      case QgsGraduatedSymbolRenderer::Quantile: m = QgsGraduatedSymbolRendererV2::Quantile; break;
      case QgsGraduatedSymbolRenderer::Empty: m = QgsGraduatedSymbolRendererV2::Custom; break;
    }
    r2->setMode( m );
    // source symbol, color ramp not set (unknown)
    r2final = r2;
  }
  else if ( rtype == "Continuous Color" )
  {
    // TODO
  }
  else if ( rtype == "Unique Value" )
  {
    const QgsUniqueValueRenderer* uvr = dynamic_cast<const QgsUniqueValueRenderer*>( r );
    if ( uvr == NULL )
      return;

    QString attrName;
    if ( layer->pendingFields().contains( uvr->classificationField() ) )
    {
      attrName = layer->pendingFields()[ uvr->classificationField()].name();
    }

    QgsCategoryList cats;
    foreach( QgsSymbol* sym, uvr->symbols() )
    {
      QVariant value = QVariant( sym->lowerValue() );
      QString label = sym->label();
      if ( label.isEmpty() )
        label = value.toString();
      QgsSymbolV2* symbolv2 = symbolV1toV2( sym );
      cats.append( QgsRendererCategoryV2( value, symbolv2, label ) );
    }

    QgsCategorizedSymbolRendererV2* r2 = new QgsCategorizedSymbolRendererV2( attrName, cats );
    // source symbol and color ramp are not set (unknown)
    r2final = r2;
  }

  if ( r2final == NULL )
  {
    r2final = QgsFeatureRendererV2::defaultRenderer( layer->geometryType() );
  }

  // change of renderers
  layer->setUsingRendererV2( true );
  layer->setRendererV2( r2final );
  layer->setRenderer( NULL );
}

void QgsSymbologyV2Conversion::rendererV2toV1( QgsVectorLayer* layer )
{
  if ( !layer->isUsingRendererV2() )
    return;

  QgsFeatureRendererV2* r2 = layer->rendererV2();
  if ( r2 == NULL )
    return;

  QgsRenderer* rfinal = NULL;

  QString r2type = r2->type();
  if ( r2type == "singleSymbol" )
  {
    QgsSingleSymbolRendererV2* ssr2 = static_cast<QgsSingleSymbolRendererV2*>( r2 );

    QgsSingleSymbolRenderer* r = new QgsSingleSymbolRenderer( layer->geometryType() );
    r->addSymbol( symbolV2toV1( ssr2->symbol() ) );
    rfinal = r;
  }
  else if ( r2type == "graduatedSymbol" )
  {
    QgsGraduatedSymbolRendererV2* gsr2 = static_cast<QgsGraduatedSymbolRendererV2*>( r2 );

    QgsGraduatedSymbolRenderer::Mode m;
    switch ( gsr2->mode() )
    {
      case QgsGraduatedSymbolRendererV2::EqualInterval: m = QgsGraduatedSymbolRenderer::EqualInterval; break;
      case QgsGraduatedSymbolRendererV2::Quantile: m = QgsGraduatedSymbolRenderer::Quantile; break;
      default: m = QgsGraduatedSymbolRenderer::Empty; break;
    }

    QgsGraduatedSymbolRenderer* r = new QgsGraduatedSymbolRenderer( layer->geometryType(), m );

    r->setClassificationField( layer->fieldNameIndex( gsr2->classAttribute() ) );

    foreach( QgsRendererRangeV2 range, gsr2->ranges() )
    {
      QgsSymbol* s = symbolV2toV1( range.symbol() );
      s->setLowerValue( QString::number( range.lowerValue(), 'f', 5 ) );
      s->setUpperValue( QString::number( range.upperValue(), 'f', 5 ) );
      s->setLabel( range.label() );
      r->addSymbol( s );
    }

    rfinal = r;
  }
  else if ( r2type == "categorizedSymbol" )
  {
    QgsCategorizedSymbolRendererV2* csr2 = static_cast<QgsCategorizedSymbolRendererV2*>( r2 );

    QgsUniqueValueRenderer* r = new QgsUniqueValueRenderer( layer->geometryType() );

    r->setClassificationField( layer->fieldNameIndex( csr2->classAttribute() ) );

    foreach( QgsRendererCategoryV2 cat, csr2->categories() )
    {
      QgsSymbol* s = symbolV2toV1( cat.symbol() );
      QString val = cat.value().toString();
      s->setLowerValue( val );
      s->setUpperValue( val );
      r->insertValue( val, s );
    }

    rfinal = r;
  }


  if ( rfinal == NULL )
  {
    rfinal = new QgsSingleSymbolRenderer( layer->geometryType() );
  }

  layer->setUsingRendererV2( false );
  layer->setRendererV2( NULL );
  layer->setRenderer( rfinal );
}
