/***************************************************************************
  qgstextrenderer.cpp
  -------------------
   begin                : August 2014
   copyright            : (C) Nyall Dawson, Martin Dobias
   email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextrenderer.h"
#include "qgssymbollayerv2utils.h"
#include "qgsmarkersymbollayerv2.h"
#include <QApplication>
#include <QPicture>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

static void _fixQPictureDPI( QPainter* p )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  p->scale(( double )qt_defaultDpiX() / p->device()->logicalDpiX(),
           ( double )qt_defaultDpiY() / p->device()->logicalDpiY() );
}

QgsTextRendererSettings::QgsTextRendererSettings()
    : ct( NULL )
{
  // text style
  textFont = QApplication::font();
  textNamedStyle = QString( "" );
  fontSizeInMapUnits = false;
  textColor = Qt::black;
  textTransp = 0;
  blendMode = QPainter::CompositionMode_SourceOver;
  previewBkgrdColor = Qt::white;
  // font processing info
  mTextFontFound = true;
  mTextFontFamily = QApplication::font().family();

  // text formatting
  wrapChar = "";
  multilineHeight = 1.0;
  multilineAlign = MultiLeft;

  // text buffer
  bufferDraw = false;
  bufferSize = 1.0;
  bufferSizeInMapUnits = false;
  bufferColor = Qt::white;
  bufferTransp = 0;
  bufferNoFill = false;
  bufferJoinStyle = Qt::BevelJoin;
  bufferBlendMode = QPainter::CompositionMode_SourceOver;

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
  shadowOffsetDist = 1.0;
  shadowOffsetUnits = MM;
  shadowOffsetGlobal = true;
  shadowRadius = 1.5;
  shadowRadiusUnits = MM;
  shadowRadiusAlphaOnly = false;
  shadowTransparency = 30;
  shadowScale = 100;
  shadowColor = Qt::black;
  shadowBlendMode = QPainter::CompositionMode_Multiply;

  // scale factors
  vectorScaleFactor = 1.0;
  rasterCompressFactor = 1.0;

  // temp stuff for when drawing label components (don't copy)
  showingShadowRects = false;
}

QgsTextRendererSettings::QgsTextRendererSettings( const QgsTextRendererSettings& s )
{
  // copy only permanent stuff

  // text style
  textFont = s.textFont;
  textNamedStyle = s.textNamedStyle;
  fontSizeInMapUnits = s.fontSizeInMapUnits;
  fontSizeMapUnitScale = s.fontSizeMapUnitScale;
  textColor = s.textColor;
  textTransp = s.textTransp;
  blendMode = s.blendMode;
  previewBkgrdColor = s.previewBkgrdColor;
  // font processing info
  mTextFontFound = s.mTextFontFound;
  mTextFontFamily = s.mTextFontFamily;

  // text formatting
  wrapChar = s.wrapChar;
  multilineHeight = s.multilineHeight;
  multilineAlign = s.multilineAlign;

  // text buffer
  bufferDraw = s.bufferDraw;
  bufferSize = s.bufferSize;
  bufferSizeInMapUnits = s.bufferSizeInMapUnits;
  bufferSizeMapUnitScale = s.bufferSizeMapUnitScale;
  bufferColor = s.bufferColor;
  bufferTransp = s.bufferTransp;
  bufferNoFill = s.bufferNoFill;
  bufferJoinStyle = s.bufferJoinStyle;
  bufferBlendMode = s.bufferBlendMode;

  // shape background
  shapeDraw = s.shapeDraw;
  shapeType = s.shapeType;
  shapeSVGFile = s.shapeSVGFile;
  shapeSizeType = s.shapeSizeType;
  shapeSize = s.shapeSize;
  shapeSizeUnits = s.shapeSizeUnits;
  shapeSizeMapUnitScale = s.shapeSizeMapUnitScale;
  shapeRotationType = s.shapeRotationType;
  shapeRotation = s.shapeRotation;
  shapeOffset = s.shapeOffset;
  shapeOffsetUnits = s.shapeOffsetUnits;
  shapeOffsetMapUnitScale = s.shapeOffsetMapUnitScale;
  shapeRadii = s.shapeRadii;
  shapeRadiiUnits = s.shapeRadiiUnits;
  shapeRadiiMapUnitScale = s.shapeRadiiMapUnitScale;
  shapeFillColor = s.shapeFillColor;
  shapeBorderColor = s.shapeBorderColor;
  shapeBorderWidth = s.shapeBorderWidth;
  shapeBorderWidthUnits = s.shapeBorderWidthUnits;
  shapeBorderWidthMapUnitScale = s.shapeBorderWidthMapUnitScale;
  shapeJoinStyle = s.shapeJoinStyle;
  shapeTransparency = s.shapeTransparency;
  shapeBlendMode = s.shapeBlendMode;

  // drop shadow
  shadowDraw = s.shadowDraw;
  shadowUnder = s.shadowUnder;
  shadowOffsetAngle = s.shadowOffsetAngle;
  shadowOffsetDist = s.shadowOffsetDist;
  shadowOffsetUnits = s.shadowOffsetUnits;
  shadowOffsetMapUnitScale = s.shadowOffsetMapUnitScale;
  shadowOffsetGlobal = s.shadowOffsetGlobal;
  shadowRadius = s.shadowRadius;
  shadowRadiusUnits = s.shadowRadiusUnits;
  shadowRadiusMapUnitScale = s.shadowRadiusMapUnitScale;
  shadowRadiusAlphaOnly = s.shadowRadiusAlphaOnly;
  shadowTransparency = s.shadowTransparency;
  shadowScale = s.shadowScale;
  shadowColor = s.shadowColor;
  shadowBlendMode = s.shadowBlendMode;

  // scale factors
  vectorScaleFactor = s.vectorScaleFactor;
  rasterCompressFactor = s.rasterCompressFactor;

  ct = NULL;
}


QgsTextRendererSettings::~QgsTextRendererSettings()
{
  delete ct;
}

double QgsTextRendererSettings::scaleToPixelContext( double size, const QgsRenderContext &c, QgsTextRendererSettings::SizeUnit unit, bool rasterfactor, const QgsMapUnitScale &mapUnitScale ) const
{
  // if render context is that of device (i.e. not a scaled map), just return size
  double mapUnitsPerPixel = mapUnitScale.computeMapUnitsPerPixel( c );

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


void QgsTextRenderer::drawText( const QRectF rect, const double rotation, const QStringList textLines, QgsRenderContext &context, QgsTextRendererSettings &textSettings, const double dpiRatio, const bool drawAsOutlines )
{
  if ( !context.painter() )
  {
    return;
  }

  // Render the components of a label in reverse order
  //   (backgrounds -> text)

  //copy scale factor from context
  textSettings.vectorScaleFactor = context.scaleFactor();

  if ( textSettings.shadowDraw && textSettings.shadowUnder == QgsTextRendererSettings::ShadowLowest )
  {
    if ( textSettings.shapeDraw )
    {
      textSettings.shadowUnder = QgsTextRendererSettings::ShadowShape;
    }
    else if ( textSettings.bufferDraw )
    {
      textSettings.shadowUnder = QgsTextRendererSettings::ShadowBuffer;
    }
    else
    {
      textSettings.shadowUnder = QgsTextRendererSettings::ShadowText;
    }
  }

  if ( textSettings.shapeDraw )
  {
    drawLabel( rect, rotation, textLines, context, textSettings, LabelShape, dpiRatio, drawAsOutlines );
  }

  if ( textSettings.bufferDraw )
  {
    drawLabel( rect, rotation, textLines, context, textSettings, LabelBuffer, dpiRatio, drawAsOutlines );
  }

  drawLabel( rect, rotation, textLines, context, textSettings, LabelText, dpiRatio, drawAsOutlines );
}

void QgsTextRenderer::drawLabel( const QRectF rect, const double rotation, const QStringList textLines,
                                 QgsRenderContext &context, QgsTextRendererSettings& textSettings,
                                 const DrawLabelType drawType, const double dpiRatio, const bool drawAsOutlines )
{
  if ( !context.painter() )
  {
    return;
  }

  QgsPoint origin = QgsPoint( rect.left(), rect.top() );

  QgsLabelComponent component;
  component.setDpiRatio( dpiRatio );
  component.setOrigin( origin );
  component.setRotation( rotation );

  if ( drawType == LabelShape )
  {
    // get rotated label's center point
    QgsPoint centerPt( origin );
    QgsPoint outPt2 = QgsPoint( origin.x() + rect.width() / 2, origin.y() + rect.height() / 2 );

    double xc = outPt2.x() - origin.x();
    double yc = outPt2.y() - origin.y();

    double angle = -rotation;
    double xd = xc * cos( angle ) - yc * sin( angle );
    double yd = xc * sin( angle ) + yc * cos( angle );

    centerPt.setX( centerPt.x() + xd );
    centerPt.setY( centerPt.y() + yd );

    component.setCenter( centerPt );
    component.setSize( QgsPoint( rect.width(), rect.height() ) );

    drawLabelBackground( context, component, textSettings );
  }

  if ( drawType == LabelBuffer || drawType == LabelText )
  {
    QFontMetricsF fontMetrics = QFontMetricsF( textSettings.textFont );
    drawLabelText( origin, rect.size(), true, textLines, drawType, component, textSettings, &fontMetrics, context, drawAsOutlines );
  }

}

void QgsTextRenderer::drawLabelBackground( QgsRenderContext &context, QgsLabelComponent component, const QgsTextRendererSettings &textSettings )
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter* p = context.painter();
  double labelWidth = component.size().x(), labelHeight = component.size().y();
  //QgsDebugMsgLevel( QString( "Background label rotation: %1" ).arg( component.rotation() ), 4 );

  // shared calculations between shapes and SVG

  // configure angles, set component rotation and rotationOffset
  if ( textSettings.shapeRotationType != QgsTextRendererSettings::RotationFixed )
  {
    component.setRotation( -( component.rotation() * 180 / M_PI ) ); // RotationSync
    component.setRotationOffset(
      textSettings.shapeRotationType == QgsTextRendererSettings::RotationOffset ? textSettings.shapeRotation : 0.0 );
  }
  else // RotationFixed
  {
    component.setRotation( 0.0 ); // don't use label's rotation
    component.setRotationOffset( textSettings.shapeRotation );
  }

  // mm to map units conversion factor
  double mmToMapUnits = textSettings.shapeSizeMapUnitScale.computeMapUnitsPerPixel( context ) * context.scaleFactor();

  // TODO: the following label-buffered generated shapes and SVG symbols should be moved into marker symbology classes

  if ( textSettings.shapeType == QgsTextRendererSettings::ShapeSVG )
  {
    // all calculations done in shapeSizeUnits, which are then passed to symbology class for painting

    if ( textSettings.shapeSVGFile.isEmpty() )
      return;

    double sizeOut = 0.0;
    // only one size used for SVG sizing/scaling (no use of shapeSize.y() or Y field in gui)
    if ( textSettings.shapeSizeType == QgsTextRendererSettings::SizeFixed )
    {
      sizeOut = textSettings.shapeSize.x();
    }
    else if ( textSettings.shapeSizeType == QgsTextRendererSettings::SizeBuffer )
    {
      // add buffer to greatest dimension of label
      if ( labelWidth >= labelHeight )
        sizeOut = labelWidth;
      else if ( labelHeight > labelWidth )
        sizeOut = labelHeight;

      // label size in map units, convert to shapeSizeUnits, if different
      if ( textSettings.shapeSizeUnits == QgsTextRendererSettings::MM )
      {
        sizeOut /= mmToMapUnits;
      }

      // add buffer
      sizeOut += textSettings.shapeSize.x() * 2;
    }

    // don't bother rendering symbols smaller than 1x1 pixels in size
    // TODO: add option to not show any svgs under/over a certian size
    if ( textSettings.scaleToPixelContext( sizeOut, context, textSettings.shapeSizeUnits, false, textSettings.shapeSizeMapUnitScale ) < 1.0 )
      return;

    QgsStringMap map; // for SVG symbology marker
    map["name"] = QgsSymbolLayerV2Utils::symbolNameToPath( textSettings.shapeSVGFile.trimmed() );
    map["size"] = QString::number( sizeOut );
    map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
                         textSettings.shapeSizeUnits == QgsTextRendererSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );
    map["angle"] = QString::number( 0.0 ); // angle is handled by this local painter

    // offset is handled by this local painter
    // TODO: see why the marker renderer doesn't seem to translate offset *after* applying rotation
    //map["offset"] = QgsSymbolLayerV2Utils::encodePoint( tmpLyr.shapeOffset );
    //map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
    //                       tmpLyr.shapeOffsetUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );

    map["fill"] = textSettings.shapeFillColor.name();
    map["outline"] = textSettings.shapeBorderColor.name();
    map["outline-width"] = QString::number( textSettings.shapeBorderWidth );

    // TODO: fix overriding SVG symbol's border width/units in QgsSvgCache
    // currently broken, fall back to symbol's
    //map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit(
    //                              tmpLyr.shapeBorderWidthUnits == QgsPalLayerSettings::MapUnits ? QgsSymbolV2::MapUnit : QgsSymbolV2::MM );

    if ( textSettings.shadowDraw && textSettings.shadowUnder == QgsTextRendererSettings::ShadowShape )
    {
      // configure SVG shadow specs
      QgsStringMap shdwmap( map );
      shdwmap["fill"] = textSettings.shadowColor.name();
      shdwmap["outline"] = textSettings.shadowColor.name();
      shdwmap["size"] = QString::number( sizeOut * textSettings.rasterCompressFactor );

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
          ( 100.0 - ( double )( textSettings.shapeTransparency ) ) / 100.0 );

      double svgSize = textSettings.scaleToPixelContext( sizeOut, context, textSettings.shapeSizeUnits, true, textSettings.shapeSizeMapUnitScale );
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
      p->scale( 1.0 / textSettings.rasterCompressFactor, 1.0 / textSettings.rasterCompressFactor );
      double xoff = textSettings.scaleToPixelContext( textSettings.shapeOffset.x(), context, textSettings.shapeOffsetUnits, true, textSettings.shapeOffsetMapUnitScale );
      double yoff = textSettings.scaleToPixelContext( textSettings.shapeOffset.y(), context, textSettings.shapeOffsetUnits, true, textSettings.shapeOffsetMapUnitScale );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset() );
      p->translate( -svgSize / 2, svgSize / 2 );

      drawLabelShadow( context, component, textSettings );
      p->restore();

      delete svgShdwM;
      svgShdwM = 0;
    }

    // draw the actual symbol
    QgsSymbolLayerV2* symL = QgsSvgMarkerSymbolLayerV2::create( map );
    QgsSvgMarkerSymbolLayerV2* svgM = static_cast<QgsSvgMarkerSymbolLayerV2*>( symL );
    QgsSymbolV2RenderContext svgContext( context, QgsSymbolV2::Mixed,
                                         ( 100.0 - ( double )( textSettings.shapeTransparency ) ) / 100.0 );

    p->save();
    if ( context.useAdvancedEffects() )
    {
      p->setCompositionMode( textSettings.shapeBlendMode );
    }
    p->translate( component.center().x(), component.center().y() );
    p->rotate( component.rotation() );
    double xoff = textSettings.scaleToPixelContext( textSettings.shapeOffset.x(), context, textSettings.shapeOffsetUnits, false, textSettings.shapeOffsetMapUnitScale );
    double yoff = textSettings.scaleToPixelContext( textSettings.shapeOffset.y(), context, textSettings.shapeOffsetUnits, false, textSettings.shapeOffsetMapUnitScale );
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

    double w = labelWidth / ( textSettings.shapeSizeUnits == QgsTextRendererSettings::MM ? mmToMapUnits : 1 );
    double h = labelHeight / ( textSettings.shapeSizeUnits == QgsTextRendererSettings::MM ? mmToMapUnits : 1 );

    double xsize = textSettings.shapeSize.x();
    double ysize = textSettings.shapeSize.y();

    if ( textSettings.shapeSizeType == QgsTextRendererSettings::SizeFixed )
    {
      w = xsize;
      h = ysize;
    }
    else if ( textSettings.shapeSizeType == QgsTextRendererSettings::SizeBuffer )
    {
      if ( textSettings.shapeType == QgsTextRendererSettings::ShapeSquare )
      {
        if ( w > h )
          h = w;
        else if ( h > w )
          w = h;
      }
      else if ( textSettings.shapeType == QgsTextRendererSettings::ShapeCircle )
      {
        // start with label bound by circle
        h = sqrt( pow( w, 2 ) + pow( h, 2 ) );
        w = h;
      }
      else if ( textSettings.shapeType == QgsTextRendererSettings::ShapeEllipse )
      {
        // start with label bound by ellipse
        h = h / sqrt( 2.0 ) * 2;
        w = w / sqrt( 2.0 ) * 2;
      }

      w += xsize * 2;
      h += ysize * 2;
    }

    // convert everything over to map pixels from here on
    w = textSettings.scaleToPixelContext( w, context, textSettings.shapeSizeUnits, true, textSettings.shapeSizeMapUnitScale );
    h = textSettings.scaleToPixelContext( h, context, textSettings.shapeSizeUnits, true, textSettings.shapeSizeMapUnitScale );

    // offsets match those of symbology: -x = left, -y = up
    QRectF rect( -w / 2.0, - h / 2.0, w, h );

    if ( rect.isNull() )
      return;

    p->save();
    p->translate( QPointF( component.center().x(), component.center().y() ) );
    p->rotate( component.rotation() );
    double xoff = textSettings.scaleToPixelContext( textSettings.shapeOffset.x(), context, textSettings.shapeOffsetUnits, false, textSettings.shapeOffsetMapUnitScale );
    double yoff = textSettings.scaleToPixelContext( textSettings.shapeOffset.y(), context, textSettings.shapeOffsetUnits, false, textSettings.shapeOffsetMapUnitScale );
    p->translate( QPointF( xoff, yoff ) );
    p->rotate( component.rotationOffset() );

    double penSize = textSettings.scaleToPixelContext( textSettings.shapeBorderWidth, context, textSettings.shapeBorderWidthUnits, true, textSettings.shapeBorderWidthMapUnitScale );

    QPen pen;
    if ( textSettings.shapeBorderWidth > 0 )
    {
      pen.setColor( textSettings.shapeBorderColor );
      pen.setWidthF( penSize );
      if ( textSettings.shapeType == QgsTextRendererSettings::ShapeRectangle )
        pen.setJoinStyle( textSettings.shapeJoinStyle );
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
    shapep.setBrush( textSettings.shapeFillColor );

    if ( textSettings.shapeType == QgsTextRendererSettings::ShapeRectangle
         || textSettings.shapeType == QgsTextRendererSettings::ShapeSquare )
    {
      if ( textSettings.shapeRadiiUnits == QgsTextRendererSettings::Percent )
      {
        shapep.drawRoundedRect( rect, textSettings.shapeRadii.x(), textSettings.shapeRadii.y(), Qt::RelativeSize );
      }
      else
      {
        double xRadius = textSettings.scaleToPixelContext( textSettings.shapeRadii.x(), context, textSettings.shapeRadiiUnits, true, textSettings.shapeRadiiMapUnitScale );
        double yRadius = textSettings.scaleToPixelContext( textSettings.shapeRadii.y(), context, textSettings.shapeRadiiUnits, true, textSettings.shapeRadiiMapUnitScale );
        shapep.drawRoundedRect( rect, xRadius, yRadius );
      }
    }
    else if ( textSettings.shapeType == QgsTextRendererSettings::ShapeEllipse
              || textSettings.shapeType == QgsTextRendererSettings::ShapeCircle )
    {
      shapep.drawEllipse( rect );
    }
    shapep.end();

    p->scale( 1.0 / textSettings.rasterCompressFactor, 1.0 / textSettings.rasterCompressFactor );

    if ( textSettings.shadowDraw && textSettings.shadowUnder == QgsTextRendererSettings::ShadowShape )
    {
      component.setPicture( &shapePict );
      component.setPictureBuffer( penSize / 2.0 );

      component.setSize( QgsPoint( rect.width(), rect.height() ) );
      component.setOffset( QgsPoint( rect.width() / 2, -rect.height() / 2 ) );
      drawLabelShadow( context, component, textSettings );
    }

    p->setOpacity(( 100.0 - ( double )( textSettings.shapeTransparency ) ) / 100.0 );
    if ( context.useAdvancedEffects() )
    {
      p->setCompositionMode( textSettings.shapeBlendMode );
    }

    // scale for any print output or image saving @ specific dpi
    p->scale( component.dpiRatio(), component.dpiRatio() );
    _fixQPictureDPI( p );
    p->drawPicture( 0, 0, shapePict );
    p->restore();
  }
}

void QgsTextRenderer::drawLabelShadow( QgsRenderContext &context, QgsLabelComponent component, const QgsTextRendererSettings &textSettings )
{
  if ( !context.painter() )
  {
    return;
  }

  // incoming component sizes should be multiplied by rasterCompressFactor, as
  // this allows shadows to be created at paint device dpi (e.g. high resolution),
  // then scale device painter by 1.0 / rasterCompressFactor for output

  QPainter* p = context.painter();
  double componentWidth = component.size().x(), componentHeight = component.size().y();
  double xOffset = component.offset().x(), yOffset = component.offset().y();
  double pictbuffer = component.pictureBuffer();

  // generate pixmap representation of label component drawing
  bool mapUnits = ( textSettings.shadowRadiusUnits == QgsTextRendererSettings::MapUnits );
  double radius = textSettings.scaleToPixelContext( textSettings.shadowRadius , context, textSettings.shadowRadiusUnits, !mapUnits, textSettings.shadowRadiusMapUnitScale );
  radius /= ( mapUnits ? textSettings.vectorScaleFactor / component.dpiRatio() : 1 );
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
  pictp.fillRect( blurImg.rect(), textSettings.shadowColor );
  pictp.end();

  // blur the QImage in-place
  if ( textSettings.shadowRadius > 0.0 && radius > 0 )
  {
    QgsSymbolLayerV2Utils::blurImageInPlace( blurImg, blurImg.rect(), radius, textSettings.shadowRadiusAlphaOnly );
  }

  if ( textSettings.showingShadowRects ) // engine setting, not per layer
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

  double offsetDist = textSettings.scaleToPixelContext( textSettings.shadowOffsetDist, context, textSettings.shadowOffsetUnits, true, textSettings.shadowOffsetMapUnitScale );
  double angleRad = textSettings.shadowOffsetAngle * M_PI / 180; // to radians
  if ( textSettings.shadowOffsetGlobal )
  {
    // TODO: check for differences in rotation origin and cw/ccw direction,
    //       when this shadow function is used for something other than labels

    // it's 0-->cw-->360 for labels
    //QgsDebugMsgLevel( QString( "Shadow aggregated label rotation (degrees): %1" ).arg( component.rotation() + component.rotationOffset() ), 4 );
    angleRad -= ( component.rotation() * M_PI / 180 + component.rotationOffset() * M_PI / 180 );
  }

  QPointF transPt( -offsetDist * cos( angleRad + M_PI / 2 ),
                   -offsetDist * sin( angleRad + M_PI / 2 ) );

  p->save();
  p->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( textSettings.shadowBlendMode );
  }
  p->setOpacity(( 100.0 - ( double )( textSettings.shadowTransparency ) ) / 100.0 );

  double scale = ( double )textSettings.shadowScale / 100.0;
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
  if ( textSettings.showingShadowRects ) // engine setting, not per layer
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

void QgsTextRenderer::drawLabelBuffer( QgsRenderContext &context, QgsLabelComponent component, const QgsTextRendererSettings &textSettings )
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter* p = context.painter();

  double penSize = textSettings.scaleToPixelContext( textSettings.bufferSize, context,
                   ( textSettings.bufferSizeInMapUnits ? QgsTextRendererSettings::MapUnits : QgsTextRendererSettings::MM ), true, textSettings.bufferSizeMapUnitScale );

  QPainterPath path;
  path.addText( 0, 0, textSettings.textFont, component.text() );
  QPen pen( textSettings.bufferColor );
  pen.setWidthF( penSize );
  pen.setJoinStyle( textSettings.bufferJoinStyle );
  QColor tmpColor( textSettings.bufferColor );
  // honor pref for whether to fill buffer interior
  if ( textSettings.bufferNoFill )
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

  if ( textSettings.shadowDraw && textSettings.shadowUnder == QgsTextRendererSettings::ShadowBuffer )
  {
    component.setOrigin( QgsPoint( 0.0, 0.0 ) );
    component.setPicture( &buffPict );
    component.setPictureBuffer( penSize / 2.0 );

    drawLabelShadow( context, component, textSettings );
  }

  p->save();
  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( textSettings.bufferBlendMode );
  }
  //  p->setPen( pen );
  //  p->setBrush( tmpColor );
  //  p->drawPath( path );

  // scale for any print output or image saving @ specific dpi
  p->scale( component.dpiRatio(), component.dpiRatio() );
  _fixQPictureDPI( p );
  p->drawPicture( 0, 0, buffPict );
  p->restore();
}

void QgsTextRenderer::drawLabelText( const QgsPoint point, const QSizeF size, const bool drawFromTop, const QStringList textLines, const DrawLabelType drawType, QgsLabelComponent component, const QgsTextRendererSettings &settings, const QFontMetricsF *fontMetrics, QgsRenderContext &context, const bool drawAsOutlines )
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter* painter = context.painter();

  int lines = textLines.size();

  double labelWidest = 0.0;
  if ( size.isValid() )
  {
    //size has been specified, use it as width for label
    labelWidest = size.width();
  }
  else
  {
    //calculate width of label using widest line
    for ( int i = 0; i < lines; ++i )
    {
      double labelWidth = fontMetrics->width( textLines.at( i ) );
      if ( labelWidth > labelWidest )
      {
        labelWidest = labelWidth;
      }
    }
  }

  double labelHeight = fontMetrics->ascent() + fontMetrics->descent(); // ignore +1 for baseline
  //  double labelHighest = fontMetrics->height() + ( double )(( lines - 1 ) * labelHeight * settings.multilineHeight );

  // needed to move bottom of text's descender to within bottom edge of label
  double ascentOffset = 0.25 * fontMetrics->ascent(); // fontMetrics->descent() is not enough

  for ( int i = 0; i < lines; ++i )
  {
    painter->save();
    painter->translate( QPointF( point.x(), point.y() ) );
    painter->rotate( -component.rotation() * 180 / M_PI );

    // scale down painter: the font size has been multiplied by raster scale factor
    // to workaround a Qt font scaling bug with small font sizes
    painter->scale( 1.0 / settings.rasterCompressFactor, 1.0 / settings.rasterCompressFactor );

    // figure x offset for horizontal alignment of multiple lines
    double xMultiLineOffset = 0.0;
    double labelWidth = fontMetrics->width( textLines.at( i ) );
    if ( lines > 1 && settings.multilineAlign != QgsTextRendererSettings::MultiLeft )
    {
      double labelWidthDiff = labelWidest - labelWidth;
      if ( settings.multilineAlign == QgsTextRendererSettings::MultiCenter )
      {
        labelWidthDiff /= 2;
      }
      xMultiLineOffset = labelWidthDiff;
      //QgsDebugMsgLevel( QString( "xMultiLineOffset: %1" ).arg( xMultiLineOffset ), 4 );
    }

    double yMultiLineOffset;
    if ( drawFromTop )
    {
      yMultiLineOffset = labelHeight + i * labelHeight * settings.multilineHeight;
    }
    else
    {
      yMultiLineOffset = - ascentOffset - ( lines - 1 - i ) * labelHeight * settings.multilineHeight;
    }
    painter->translate( QPointF( xMultiLineOffset, yMultiLineOffset ) );

    component.setText( textLines.at( i ) );
    component.setSize( QgsPoint( labelWidth, labelHeight ) );
    component.setOffset( QgsPoint( 0.0, -ascentOffset ) );
    component.setRotation( -component.rotation() * 180 / M_PI );
    component.setRotationOffset( 0.0 );

    if ( drawType == LabelBuffer )
    {
      // draw label's buffer
      drawLabelBuffer( context, component, settings );
    }
    else
    {
      // draw label's text, QPainterPath method
      QPainterPath path;
      path.addText( 0, 0, settings.textFont, component.text() );

      // store text's drawing in QPicture for drop shadow call
      QPicture textPict;
      QPainter textp;
      textp.begin( &textPict );
      textp.setPen( Qt::NoPen );
      textp.setBrush( settings.textColor );
      textp.drawPath( path );
      // TODO: why are some font settings lost on drawPicture() when using drawText() inside QPicture?
      //       e.g. some capitalization options, but not others
      //textp.setFont( tmpLyr.textFont );
      //textp.setPen( tmpLyr.textColor );
      //textp.drawText( 0, 0, component.text() );
      textp.end();

      if ( settings.shadowDraw && settings.shadowUnder == QgsTextRendererSettings::ShadowText )
      {
        component.setPicture( &textPict );
        component.setPictureBuffer( 0.0 ); // no pen width to deal with
        component.setOrigin( QgsPoint( 0.0, 0.0 ) );

        drawLabelShadow( context, component, settings );
      }

      // paint the text
      if ( context.useAdvancedEffects() )
      {
        painter->setCompositionMode( settings.blendMode );
      }

      // scale for any print output or image saving @ specific dpi
      painter->scale( component.dpiRatio(), component.dpiRatio() );

      if ( drawAsOutlines )
      {
        // draw outlined text
        _fixQPictureDPI( painter );
        painter->drawPicture( 0, 0, textPict );
      }
      else
      {
        // draw text as text (for SVG and PDF exports)
        painter->setFont( settings.textFont );
        painter->setPen( settings.textColor );
        painter->setRenderHint( QPainter::TextAntialiasing );
        painter->drawText( 0, 0, component.text() );
      }
    }
    painter->restore();
  }
}
