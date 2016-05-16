/***************************************************************************
    qgscolorwidgets.cpp - color selection widgets
    ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorwidgets.h"
#include "qgsapplication.h"
#include "qgssymbollayerv2utils.h"
#include <QResizeEvent>
#include <QStyleOptionFrameV3>
#include <QPainter>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QFontMetrics>
#include <QToolButton>
#include <QMenu>
#include <QSettings>
#include <QDrag>
#include <cmath>
#include "qgslogger.h"


//
// QgsColorWidget
//

QgsColorWidget::QgsColorWidget( QWidget* parent, const ColorComponent component )
    : QWidget( parent )
    , mCurrentColor( Qt::red )
    , mComponent( component )
    , mExplicitHue( 0 )
{
  setAcceptDrops( true );
}

QgsColorWidget::~QgsColorWidget()
{

}

int QgsColorWidget::componentValue() const
{
  return componentValue( mComponent );
}

QPixmap QgsColorWidget::createDragIcon( const QColor &color )
{
  //craft a pixmap for the drag icon
  QPixmap pixmap( 50, 50 );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  //start with a light gray background
  painter.fillRect( QRect( 0, 0, 50, 50 ), QBrush( QColor( 200, 200, 200 ) ) );
  //draw rect with white border, filled with current color
  QColor pixmapColor = color;
  pixmapColor.setAlpha( 255 );
  painter.setBrush( QBrush( pixmapColor ) );
  painter.setPen( QPen( Qt::white ) );
  painter.drawRect( QRect( 1, 1, 47, 47 ) );
  painter.end();
  return pixmap;
}

int QgsColorWidget::componentValue( const QgsColorWidget::ColorComponent component ) const
{
  if ( !mCurrentColor.isValid() )
  {
    return -1;
  }

  switch ( component )
  {
    case QgsColorWidget::Red:
      return mCurrentColor.red();
    case QgsColorWidget::Green:
      return mCurrentColor.green();
    case QgsColorWidget::Blue:
      return mCurrentColor.blue();
    case QgsColorWidget::Hue:
      //hue is treated specially, to avoid -1 hues values from QColor for ambiguous hues
      return hue();
    case QgsColorWidget::Saturation:
      return mCurrentColor.hsvSaturation();
    case QgsColorWidget::Value:
      return mCurrentColor.value();
    case QgsColorWidget::Alpha:
      return mCurrentColor.alpha();
    default:
      return -1;
  }
}

int QgsColorWidget::componentRange() const
{
  return componentRange( mComponent );
}

int QgsColorWidget::componentRange( const QgsColorWidget::ColorComponent component ) const
{
  if ( component == QgsColorWidget::Multiple )
  {
    //no component
    return -1;
  }

  if ( component == QgsColorWidget::Hue )
  {
    //hue ranges to 359
    return 359;
  }
  else
  {
    //all other components range to 255
    return 255;
  }
}

int QgsColorWidget::hue() const
{
  if ( mCurrentColor.hue() >= 0 )
  {
    return mCurrentColor.hue();
  }
  else
  {
    return mExplicitHue;
  }
}

void QgsColorWidget::alterColor( QColor& color, const QgsColorWidget::ColorComponent component, const int newValue ) const
{
  int h, s, v, a;
  color.getHsv( &h, &s, &v, &a );

  //clip value to sensible range
  int clippedValue = qMin( qMax( 0, newValue ), componentRange( component ) );

  switch ( component )
  {
    case QgsColorWidget::Red:
      color.setRed( clippedValue );
      return;
    case QgsColorWidget::Green:
      color.setGreen( clippedValue );
      return;
    case QgsColorWidget::Blue:
      color.setBlue( clippedValue );
      return;
    case QgsColorWidget::Hue:
      color.setHsv( clippedValue, s, v, a );
      return;
    case QgsColorWidget::Saturation:
      color.setHsv( h, clippedValue, v, a );
      return;
    case QgsColorWidget::Value:
      color.setHsv( h, s, clippedValue, a );
      return;
    case QgsColorWidget::Alpha:
      color.setAlpha( clippedValue );
      return;
    default:
      return;
  }
}

const QPixmap &QgsColorWidget::transparentBackground()
{
  static QPixmap transpBkgrd;

  if ( transpBkgrd.isNull() )
    transpBkgrd = QgsApplication::getThemePixmap( "/transp-background_8x8.png" );

  return transpBkgrd;
}

void QgsColorWidget::dragEnterEvent( QDragEnterEvent *e )
{
  //is dragged data valid color data?
  bool hasAlpha;
  QColor mimeColor = QgsSymbolLayerV2Utils::colorFromMimeData( e->mimeData(), hasAlpha );

  if ( mimeColor.isValid() )
  {
    //if so, we accept the drag
    e->acceptProposedAction();
  }
}

void QgsColorWidget::dropEvent( QDropEvent *e )
{
  //is dropped data valid color data?
  bool hasAlpha = false;
  QColor mimeColor = QgsSymbolLayerV2Utils::colorFromMimeData( e->mimeData(), hasAlpha );

  if ( mimeColor.isValid() )
  {
    //accept drop and set new color
    e->acceptProposedAction();

    if ( !hasAlpha )
    {
      //mime color has no explicit alpha component, so keep existing alpha
      mimeColor.setAlpha( mCurrentColor.alpha() );
    }

    setColor( mimeColor );
    emit colorChanged( mCurrentColor );
  }

  //could not get color from mime data
}

void QgsColorWidget::mouseMoveEvent( QMouseEvent *e )
{
  emit hovered();
  e->accept();
  //don't pass to QWidget::mouseMoveEvent, causes issues with widget used in QWidgetAction
}

void QgsColorWidget::mousePressEvent( QMouseEvent *e )
{
  e->accept();
  //don't pass to QWidget::mousePressEvent, causes issues with widget used in QWidgetAction
}

void QgsColorWidget::mouseReleaseEvent( QMouseEvent *e )
{
  e->accept();
  //don't pass to QWidget::mouseReleaseEvent, causes issues with widget used in QWidgetAction
}

QColor QgsColorWidget::color() const
{
  return mCurrentColor;
}

void QgsColorWidget::setComponent( const QgsColorWidget::ColorComponent component )
{
  if ( component == mComponent )
  {
    return;
  }

  mComponent = component;
  update();
}

void QgsColorWidget::setComponentValue( const int value )
{
  if ( mComponent == QgsColorWidget::Multiple )
  {
    return;
  }

  //clip value to valid range
  int valueClipped = qMin( value, componentRange() );
  valueClipped = qMax( valueClipped, 0 );

  int r, g, b, a;
  mCurrentColor.getRgb( &r, &g, &b, &a );
  int h, s, v;
  mCurrentColor.getHsv( &h, &s, &v );
  //overwrite hue with explicit hue if required
  h = hue();

  switch ( mComponent )
  {
    case QgsColorWidget::Red:
      if ( r == valueClipped )
      {
        return;
      }
      mCurrentColor.setRed( valueClipped );
      break;
    case QgsColorWidget::Green:
      if ( g == valueClipped )
      {
        return;
      }
      mCurrentColor.setGreen( valueClipped );
      break;
    case QgsColorWidget::Blue:
      if ( b == valueClipped )
      {
        return;
      }
      mCurrentColor.setBlue( valueClipped );
      break;
    case QgsColorWidget::Hue:
      if ( h == valueClipped )
      {
        return;
      }
      mCurrentColor.setHsv( valueClipped, s, v, a );
      break;
    case QgsColorWidget::Saturation:
      if ( s == valueClipped )
      {
        return;
      }
      mCurrentColor.setHsv( h, valueClipped, v, a );
      break;
    case QgsColorWidget::Value:
      if ( v == valueClipped )
      {
        return;
      }
      mCurrentColor.setHsv( h, s, valueClipped, a );
      break;
    case QgsColorWidget::Alpha:
      if ( a == valueClipped )
      {
        return;
      }
      mCurrentColor.setAlpha( valueClipped );
      break;
    default:
      return;
  }

  //update recorded hue
  if ( mCurrentColor.hue() >= 0 )
  {
    mExplicitHue = mCurrentColor.hue();
  }

  update();
}

void QgsColorWidget::setColor( const QColor &color, const bool emitSignals )
{
  if ( color == mCurrentColor )
  {
    return;
  }

  mCurrentColor = color;

  //update recorded hue
  if ( color.hue() >= 0 )
  {
    mExplicitHue = color.hue();
  }

  if ( emitSignals )
  {
    emit colorChanged( mCurrentColor );
  }

  update();
}


//
// QgsColorWheel
//

QgsColorWheel::QgsColorWheel( QWidget *parent )
    : QgsColorWidget( parent )
    , mMargin( 4 )
    , mWheelThickness( 18 )
    , mClickedPart( QgsColorWheel::None )
    , mWheelImage( nullptr )
    , mTriangleImage( nullptr )
    , mWidgetImage( nullptr )
    , mWheelDirty( true )
    , mTriangleDirty( true )
{
  //create wheel hue brush - only do this once
  QConicalGradient wheelGradient = QConicalGradient( 0, 0, 0 );
  int wheelStops = 20;
  QColor gradColor = QColor::fromHsvF( 1.0, 1.0, 1.0 );
  for ( int pos = 0; pos <= wheelStops; ++pos )
  {
    double relativePos = ( double )pos / wheelStops;
    gradColor.setHsvF( relativePos, 1, 1 );
    wheelGradient.setColorAt( relativePos, gradColor );
  }
  mWheelBrush = QBrush( wheelGradient );
}

QgsColorWheel::~QgsColorWheel()
{
  delete mWheelImage;
  delete mTriangleImage;
  delete mWidgetImage;
}

QSize QgsColorWheel::sizeHint() const
{
  return QSize( 200, 200 );
}

void QgsColorWheel::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );
  QPainter painter( this );

  //draw a frame
  QStyleOptionFrameV3 option = QStyleOptionFrameV3();
  option.initFrom( this );
  option.state = this->hasFocus() ? QStyle::State_Active : QStyle::State_None;
  style()->drawPrimitive( QStyle::PE_Frame, &option, &painter );

  if ( !mWidgetImage || !mWheelImage || !mTriangleImage )
  {
    createImages( size() );
  }

  //draw everything in an image
  mWidgetImage->fill( Qt::transparent );
  QPainter imagePainter( mWidgetImage );
  imagePainter.setRenderHint( QPainter::Antialiasing );

  if ( mWheelDirty )
  {
    //need to redraw the wheel image
    createWheel();
  }

  //draw wheel centered on widget
  QPointF center = QPointF( width() / 2.0, height() / 2.0 );
  imagePainter.drawImage( QPointF( center.x() - ( mWheelImage->width() / 2.0 ), center.y() - ( mWheelImage->height() / 2.0 ) ), *mWheelImage );

  //draw hue marker
  int h = hue();
  double length = mWheelImage->width() / 2.0;
  QLineF hueMarkerLine = QLineF( center.x(), center.y(), center.x() + length, center.y() );
  hueMarkerLine.setAngle( h );
  imagePainter.save();
  //use sourceIn mode for nicer antialiasing
  imagePainter.setCompositionMode( QPainter::CompositionMode_SourceIn );
  QPen pen;
  pen.setWidth( 2 );
  //adapt pen color for hue
  pen.setColor( h > 20 && h < 200 ? Qt::black : Qt::white );
  imagePainter.setPen( pen );
  imagePainter.drawLine( hueMarkerLine );
  imagePainter.restore();

  //draw triangle
  if ( mTriangleDirty )
  {
    createTriangle();
  }
  imagePainter.drawImage( QPointF( center.x() - ( mWheelImage->width() / 2.0 ), center.y() - ( mWheelImage->height() / 2.0 ) ), *mTriangleImage );

  //draw current color marker
  double triangleRadius = length - mWheelThickness - 1;

  //adapted from equations at https://github.com/timjb/colortriangle/blob/master/colortriangle.js by Tim Baumann
  double lightness = mCurrentColor.lightnessF();
  double hueRadians = ( h * M_PI / 180.0 );
  double hx = cos( hueRadians ) * triangleRadius;
  double hy = -sin( hueRadians ) * triangleRadius;
  double sx = -cos( -hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  double sy = -sin( -hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  double vx = -cos( hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  double vy = sin( hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  double mx = ( sx + vx ) / 2.0;
  double  my = ( sy + vy ) / 2.0;

  double a = ( 1 - 2.0 * fabs( lightness - 0.5 ) ) * mCurrentColor.hslSaturationF();
  double x = sx + ( vx - sx ) * lightness + ( hx - mx ) * a;
  double y = sy + ( vy - sy ) * lightness + ( hy - my ) * a;

  //adapt pen color for lightness
  pen.setColor( lightness > 0.7 ? Qt::black : Qt::white );
  imagePainter.setPen( pen );
  imagePainter.setBrush( Qt::NoBrush );
  imagePainter.drawEllipse( QPointF( x + center.x(), y + center.y() ), 4.0, 4.0 );
  imagePainter.end();

  //draw image onto widget
  painter.drawImage( QPoint( 0, 0 ), *mWidgetImage );
  painter.end();
}

void QgsColorWheel::setColor( const QColor &color, const bool emitSignals )
{
  if ( color.hue() >= 0 && color.hue() != hue() )
  {
    //hue has changed, need to redraw the triangle
    mTriangleDirty = true;
  }

  QgsColorWidget::setColor( color, emitSignals );
}

void QgsColorWheel::createImages( const QSizeF size )
{
  double wheelSize = qMin( size.width(), size.height() ) - mMargin * 2.0;
  mWheelThickness = wheelSize / 15.0;

  //recreate cache images at correct size
  delete mWheelImage;
  mWheelImage = new QImage( wheelSize, wheelSize, QImage::Format_ARGB32 );
  delete mTriangleImage;
  mTriangleImage = new QImage( wheelSize, wheelSize, QImage::Format_ARGB32 );
  delete mWidgetImage;
  mWidgetImage = new QImage( size.width(), size.height(), QImage::Format_ARGB32 );

  //trigger a redraw for the images
  mWheelDirty = true;
  mTriangleDirty = true;
}

void QgsColorWheel::resizeEvent( QResizeEvent *event )
{
  //recreate images for new size
  createImages( event->size() );
  QgsColorWidget::resizeEvent( event );
}

void QgsColorWheel::setColorFromPos( const QPointF pos )
{
  QPointF center = QPointF( width() / 2.0, height() / 2.0 );
  //line from center to mouse position
  QLineF line = QLineF( center.x(), center.y(), pos.x(), pos.y() );

  QColor newColor = QColor();

  int h, s, l, alpha;
  mCurrentColor.getHsl( &h, &s, &l, &alpha );
  //override hue with explicit hue, so we don't get -1 values from QColor for hue
  h = hue();

  if ( mClickedPart == QgsColorWheel::Triangle )
  {
    //adapted from equations at https://github.com/timjb/colortriangle/blob/master/colortriangle.js by Tim Baumann

    //position of event relative to triangle center
    double x = pos.x() - center.x();
    double y = pos.y() - center.y();

    double eventAngleRadians = line.angle() * M_PI / 180.0;
    double hueRadians = h * M_PI / 180.0;
    double rad0 = fmod( eventAngleRadians + 2.0 * M_PI - hueRadians, 2.0 * M_PI );
    double rad1 = fmod( rad0, (( 2.0 / 3.0 ) * M_PI ) ) - ( M_PI / 3.0 );
    double length = mWheelImage->width() / 2.0;
    double triangleLength = length - mWheelThickness - 1;

    double a = 0.5 * triangleLength;
    double b = tan( rad1 ) * a;
    double r = sqrt( x * x + y * y );
    double maxR = sqrt( a * a + b * b );

    if ( r > maxR )
    {
      double dx = tan( rad1 ) * r;
      double rad2 = atan( dx / maxR );
      rad2 = qMin( rad2, M_PI / 3.0 );
      rad2 = qMax( rad2, -M_PI / 3.0 );
      eventAngleRadians += rad2 - rad1;
      rad0 = fmod( eventAngleRadians + 2.0 * M_PI - hueRadians, 2.0 * M_PI );
      rad1 = fmod( rad0, (( 2.0 / 3.0 ) * M_PI ) ) - ( M_PI / 3.0 );
      b = tan( rad1 ) * a;
      r = sqrt( a * a + b * b );
    }

    double triangleSideLength = sqrt( 3.0 ) * triangleLength;
    double newL = (( -sin( rad0 ) * r ) / triangleSideLength ) + 0.5;
    double widthShare = 1.0 - ( fabs( newL - 0.5 ) * 2.0 );
    double newS = ((( cos( rad0 ) * r ) + ( triangleLength / 2.0 ) ) / ( 1.5 * triangleLength ) ) / widthShare;
    s = qMin( qRound( qMax( 0.0, newS ) * 255.0 ), 255 );
    l = qMin( qRound( qMax( 0.0, newL ) * 255.0 ), 255 );
    newColor = QColor::fromHsl( h, s, l );
    //explicitly set the hue again, so that it's exact
    newColor.setHsv( h, newColor.hsvSaturation(), newColor.value(), alpha );
  }
  else if ( mClickedPart == QgsColorWheel::Wheel )
  {
    //use hue angle
    s = mCurrentColor.hsvSaturation();
    int v = mCurrentColor.value();
    int newHue = line.angle();
    newColor = QColor::fromHsv( newHue, s, v, alpha );
    //hue has changed, need to redraw triangle
    mTriangleDirty = true;
  }

  if ( newColor.isValid() && newColor != mCurrentColor )
  {
    //color has changed
    mCurrentColor = QColor( newColor );

    if ( mCurrentColor.hue() >= 0 )
    {
      //color has a valid hue, so update the QgsColorWidget's explicit hue
      mExplicitHue = mCurrentColor.hue();
    }

    update();
    emit colorChanged( mCurrentColor );
  }
}

void QgsColorWheel::mouseMoveEvent( QMouseEvent *event )
{
  setColorFromPos( event->posF() );
  QgsColorWidget::mouseMoveEvent( event );
}

void QgsColorWheel::mousePressEvent( QMouseEvent *event )
{
  //calculate where the event occurred -- on the wheel or inside the triangle?

  //create a line from the widget's center to the event
  QLineF line = QLineF( width() / 2.0, height() / 2.0, event->pos().x(), event->pos().y() );

  double innerLength = mWheelImage->width() / 2.0 - mWheelThickness;
  if ( line.length() < innerLength )
  {
    mClickedPart = QgsColorWheel::Triangle;
  }
  else
  {
    mClickedPart = QgsColorWheel::Wheel;
  }
  setColorFromPos( event->posF() );
}

void QgsColorWheel::mouseReleaseEvent( QMouseEvent *event )
{
  Q_UNUSED( event );
  mClickedPart = QgsColorWheel::None;
}

void QgsColorWheel::createWheel()
{
  if ( !mWheelImage )
  {
    return;
  }

  int maxSize = qMin( mWheelImage->width(),  mWheelImage->height() );
  double wheelRadius = maxSize / 2.0;

  mWheelImage->fill( Qt::transparent );
  QPainter p( mWheelImage );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( mWheelBrush );
  p.setPen( Qt::NoPen );

  //draw hue wheel as a circle
  p.translate( wheelRadius, wheelRadius );
  p.drawEllipse( QPointF( 0.0, 0.0 ), wheelRadius, wheelRadius );

  //cut hole in center of circle to make a ring
  p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
  p.setBrush( QBrush( Qt::black ) );
  p.drawEllipse( QPointF( 0.0, 0.0 ), wheelRadius - mWheelThickness, wheelRadius  - mWheelThickness );
  p.end();

  mWheelDirty = false;
}

void QgsColorWheel::createTriangle()
{
  if ( !mWheelImage || !mTriangleImage )
  {
    return;
  }

  QPointF center = QPointF( mWheelImage->width() / 2.0, mWheelImage->height() / 2.0 );
  mTriangleImage->fill( Qt::transparent );

  QPainter imagePainter( mTriangleImage );
  imagePainter.setRenderHint( QPainter::Antialiasing );

  int angle = hue();
  double wheelRadius = mWheelImage->width() / 2.0;
  double triangleRadius = wheelRadius - mWheelThickness - 1;

  //pure version of hue (at full saturation and value)
  QColor pureColor = QColor::fromHsv( angle, 255, 255 );
  //create copy of color but with 0 alpha
  QColor alphaColor = QColor( pureColor );
  alphaColor.setAlpha( 0 );

  //some rather ugly shortcuts to obtain corners and midpoints of triangle
  QLineF line1 = QLineF( center.x(), center.y(), center.x() - triangleRadius * cos( M_PI / 3.0 ), center.y() - triangleRadius * sin( M_PI / 3.0 ) );
  QLineF line2 = QLineF( center.x(), center.y(), center.x() + triangleRadius, center.y() );
  QLineF line3 = QLineF( center.x(), center.y(), center.x() - triangleRadius * cos( M_PI / 3.0 ), center.y() + triangleRadius * sin( M_PI / 3.0 ) );
  QLineF line4 = QLineF( center.x(), center.y(), center.x() - triangleRadius * cos( M_PI / 3.0 ), center.y() );
  QLineF line5 = QLineF( center.x(), center.y(), ( line2.p2().x() + line1.p2().x() ) / 2.0, ( line2.p2().y() + line1.p2().y() ) / 2.0 );
  line1.setAngle( line1.angle() + angle );
  line2.setAngle( line2.angle() + angle );
  line3.setAngle( line3.angle() + angle );
  line4.setAngle( line4.angle() + angle );
  line5.setAngle( line5.angle() + angle );
  QPointF p1 = line1.p2();
  QPointF p2 = line2.p2();
  QPointF p3 = line3.p2();
  QPointF p4 = line4.p2();
  QPointF p5 = line5.p2();

  //inspired by Tim Baumann's work at https://github.com/timjb/colortriangle/blob/master/colortriangle.js
  QLinearGradient colorGrad = QLinearGradient( p4.x(), p4.y(), p2.x(), p2.y() );
  colorGrad.setColorAt( 0, alphaColor );
  colorGrad.setColorAt( 1, pureColor );
  QLinearGradient whiteGrad = QLinearGradient( p3.x(), p3.y(), p5.x(), p5.y() );
  whiteGrad.setColorAt( 0, QColor( 255, 255, 255, 255 ) );
  whiteGrad.setColorAt( 1, QColor( 255, 255, 255, 0 ) );

  QPolygonF triangle;
  triangle << p2 << p1 << p3 << p2;
  imagePainter.setPen( Qt::NoPen );
  //start with a black triangle
  imagePainter.setBrush( QBrush( Qt::black ) );
  imagePainter.drawPolygon( triangle );
  //draw a gradient from transparent to the pure color at the triangle's tip
  imagePainter.setBrush( QBrush( colorGrad ) );
  imagePainter.drawPolygon( triangle );
  //draw a white gradient using additive composition mode
  imagePainter.setCompositionMode( QPainter::CompositionMode_Plus );
  imagePainter.setBrush( QBrush( whiteGrad ) );
  imagePainter.drawPolygon( triangle );

  //above process results in some small artifacts on the edge of the triangle. Let's clear these up
  //use source composition mode and draw an outline using a transparent pen
  //this clears the edge pixels and leaves a nice smooth image
  imagePainter.setCompositionMode( QPainter::CompositionMode_Source );
  imagePainter.setBrush( Qt::NoBrush );
  imagePainter.setPen( QPen( Qt::transparent ) );
  imagePainter.drawPolygon( triangle );

  imagePainter.end();
  mTriangleDirty = false;
}



//
// QgsColorBox
//

QgsColorBox::QgsColorBox( QWidget *parent, const ColorComponent component )
    : QgsColorWidget( parent, component )
    , mMargin( 2 )
    , mBoxImage( nullptr )
    , mDirty( true )
{
  setFocusPolicy( Qt::StrongFocus );
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

  mBoxImage = new QImage( width() - mMargin * 2, height() - mMargin * 2, QImage::Format_RGB32 );
}

QgsColorBox::~QgsColorBox()
{
  delete mBoxImage;
}

QSize QgsColorBox::sizeHint() const
{
  return QSize( 200, 200 );
}

void QgsColorBox::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );
  QPainter painter( this );

  QStyleOptionFrameV3 option;
  option.initFrom( this );
  option.state = hasFocus() ? QStyle::State_Active :  QStyle::State_None;
  style()->drawPrimitive( QStyle::PE_Frame, &option, &painter );

  if ( mDirty )
  {
    createBox();
  }

  //draw background image
  painter.drawImage( QPoint( mMargin, mMargin ), *mBoxImage );

  //draw cross lines
  double xPos = mMargin + ( width() - 2 * mMargin - 1 ) * ( double )xComponentValue() / ( double )valueRangeX();
  double yPos = mMargin + ( height() - 2 * mMargin - 1 ) - ( height() - 2 * mMargin - 1 ) * ( double )yComponentValue() / ( double )valueRangeY();

  painter.setBrush( Qt::white );
  painter.setPen( Qt::NoPen );

  painter.drawRect( xPos - 1, mMargin, 3, height() - 2 * mMargin - 1 );
  painter.drawRect( mMargin, yPos - 1, width() - 2 * mMargin - 1, 3 );
  painter.setPen( Qt::black );
  painter.drawLine( xPos, mMargin, xPos, height() - mMargin - 1 );
  painter.drawLine( mMargin, yPos, width() - mMargin - 1, yPos );

  painter.end();
}

void QgsColorBox::setComponent( const QgsColorWidget::ColorComponent component )
{
  if ( component != mComponent )
  {
    //need to redraw
    mDirty = true;
  }
  QgsColorWidget::setComponent( component );
}

void QgsColorBox::setColor( const QColor &color, const bool emitSignals )
{
  //check if we need to redraw the box image
  if ( mComponent == QgsColorWidget::Red && mCurrentColor.red() != color.red() )
  {
    mDirty = true;
  }
  else if ( mComponent == QgsColorWidget::Green && mCurrentColor.green() != color.green() )
  {
    mDirty = true;
  }
  else if ( mComponent == QgsColorWidget::Blue && mCurrentColor.blue() != color.blue() )
  {
    mDirty = true;
  }
  else if ( mComponent == QgsColorWidget::Hue && color.hsvHue() >= 0 && hue() != color.hsvHue() )
  {
    mDirty = true;
  }
  else if ( mComponent == QgsColorWidget::Saturation && mCurrentColor.hsvSaturation() != color.hsvSaturation() )
  {
    mDirty = true;
  }
  else if ( mComponent == QgsColorWidget::Value && mCurrentColor.value() != color.value() )
  {
    mDirty = true;
  }
  QgsColorWidget::setColor( color, emitSignals );
}

void QgsColorBox::resizeEvent( QResizeEvent *event )
{
  mDirty = true;
  delete mBoxImage;
  mBoxImage = new QImage( event->size().width() - mMargin * 2, event->size().height() - mMargin * 2, QImage::Format_RGB32 );
  QgsColorWidget::resizeEvent( event );
}

void QgsColorBox::mouseMoveEvent( QMouseEvent *event )
{
  setColorFromPoint( event->pos() );
  QgsColorWidget::mouseMoveEvent( event );
}

void QgsColorBox::mousePressEvent( QMouseEvent *event )
{
  setColorFromPoint( event->pos() );
}

void QgsColorBox::createBox()
{
  int maxValueX = mBoxImage->width();
  int maxValueY = mBoxImage->height();

  //create a temporary color object
  QColor currentColor = QColor( mCurrentColor );
  int colorComponentValue;

  for ( int y = 0; y < maxValueY; ++y )
  {
    QRgb* scanLine = ( QRgb* )mBoxImage->scanLine( y );

    colorComponentValue = int( valueRangeY() - valueRangeY() * ( double( y ) / maxValueY ) );
    alterColor( currentColor, yComponent(), colorComponentValue );
    for ( int x = 0; x < maxValueX; ++x )
    {
      colorComponentValue = int( valueRangeX() * ( double( x ) / maxValueX ) );
      alterColor( currentColor, xComponent(), colorComponentValue );
      scanLine[x] = currentColor.rgb();
    }
  }
  mDirty = false;
}

int QgsColorBox::valueRangeX() const
{
  return componentRange( xComponent() );
}

int QgsColorBox::valueRangeY() const
{
  return componentRange( yComponent() );
}

QgsColorWidget::ColorComponent QgsColorBox::yComponent() const
{
  switch ( mComponent )
  {
    case  QgsColorWidget::Red:
      return QgsColorWidget::Green;
    case QgsColorWidget:: Green:
    case QgsColorWidget:: Blue:
      return QgsColorWidget::Red;
    case QgsColorWidget::Hue:
      return QgsColorWidget:: Saturation;
    case QgsColorWidget:: Saturation:
    case QgsColorWidget:: Value:
      return  QgsColorWidget::Hue;
    default:
      //should not occur
      return QgsColorWidget::Red;
  }
}

int QgsColorBox::yComponentValue() const
{
  return componentValue( yComponent() );
}

QgsColorWidget::ColorComponent QgsColorBox::xComponent() const
{
  switch ( mComponent )
  {
    case  QgsColorWidget::Red:
    case QgsColorWidget:: Green:
      return QgsColorWidget::Blue;
    case QgsColorWidget:: Blue:
      return QgsColorWidget::Green;
    case QgsColorWidget::Hue:
    case QgsColorWidget:: Saturation:
      return QgsColorWidget:: Value;
    case QgsColorWidget:: Value:
      return  QgsColorWidget::Saturation;
    default:
      //should not occur
      return QgsColorWidget::Red;
  }
}

int QgsColorBox::xComponentValue() const
{
  return componentValue( xComponent() );
}

void QgsColorBox::setColorFromPoint( QPoint point )
{
  int valX = valueRangeX() * ( point.x() - mMargin ) / ( width() - 2 * mMargin - 1 );
  valX = qMin( qMax( valX, 0 ), valueRangeX() );

  int valY = valueRangeY() - valueRangeY() * ( point.y() - mMargin ) / ( height() - 2 * mMargin - 1 );
  valY = qMin( qMax( valY, 0 ), valueRangeY() );

  QColor color = QColor( mCurrentColor );
  alterColor( color, xComponent(), valX );
  alterColor( color, yComponent(), valY );

  if ( color == mCurrentColor )
  {
    return;
  }

  if ( color.hue() >= 0 )
  {
    mExplicitHue = color.hue();
  }

  mCurrentColor = color;
  update();
  emit colorChanged( color );
}


//
// QgsColorRampWidget
//

QgsColorRampWidget::QgsColorRampWidget( QWidget *parent,
                                        const QgsColorWidget::ColorComponent component,
                                        const Orientation orientation )
    : QgsColorWidget( parent, component )
    , mMargin( 4 )
    , mShowFrame( false )
{
  setFocusPolicy( Qt::StrongFocus );
  setOrientation( orientation );

  //create triangle polygons
  setMarkerSize( 5 );
}

QgsColorRampWidget::~QgsColorRampWidget()
{

}

QSize QgsColorRampWidget::sizeHint() const
{
  if ( mOrientation == QgsColorRampWidget::Horizontal )
  {
    //horizontal
    return QSize( 200, 28 );
  }
  else
  {
    //vertical
    return QSize( 18, 200 );
  }
}

void QgsColorRampWidget::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );
  QPainter painter( this );

  if ( mShowFrame )
  {
    //draw frame
    QStyleOptionFrameV3 option;
    option.initFrom( this );
    option.state = hasFocus() ? QStyle::State_KeyboardFocusChange : QStyle::State_None;
    style()->drawPrimitive( QStyle::PE_Frame, &option, &painter );
  }

  if ( hasFocus() )
  {
    //draw focus rect
    QStyleOptionFocusRect option;
    option.initFrom( this );
    option.state = QStyle::State_KeyboardFocusChange;
    style()->drawPrimitive( QStyle::PE_FrameFocusRect, &option, &painter );
  }

  if ( mComponent != QgsColorWidget::Alpha )
  {
    int maxValue = ( mOrientation == QgsColorRampWidget::Horizontal ? width() : height() ) - 1 - 2 * mMargin;
    QColor color = QColor( mCurrentColor );
    color.setAlpha( 255 );
    QPen pen;
    pen.setWidth( 0 );
    painter.setPen( pen );
    painter.setBrush( Qt::NoBrush );

    //draw background ramp
    for ( int c = 0; c <= maxValue; ++c )
    {
      int colorVal = componentRange() * ( double )c / maxValue;
      //vertical sliders are reversed
      if ( mOrientation == QgsColorRampWidget::Vertical )
      {
        colorVal = componentRange() - colorVal;
      }
      alterColor( color, mComponent, colorVal );
      if ( color.hue() < 0 )
      {
        color.setHsv( hue(), color.saturation(), color.value() );
      }
      pen.setColor( color );
      painter.setPen( pen );
      if ( mOrientation == QgsColorRampWidget::Horizontal )
      {
        //horizontal
        painter.drawLine( c + mMargin, mMargin, c + mMargin, height() - mMargin - 1 );
      }
      else
      {
        //vertical
        painter.drawLine( mMargin, c + mMargin, width() - mMargin - 1, c + mMargin );
      }
    }
  }
  else
  {
    //alpha ramps are drawn differently
    //start with the checkboard pattern
    QBrush checkBrush = QBrush( transparentBackground() );
    painter.setBrush( checkBrush );
    painter.setPen( Qt::NoPen );
    painter.drawRect( mMargin, mMargin, width() - 2 * mMargin - 1, height() - 2 * mMargin - 1 );
    QLinearGradient colorGrad;
    if ( mOrientation == QgsColorRampWidget::Horizontal )
    {
      //horizontal
      colorGrad = QLinearGradient( mMargin, 0, width() - mMargin - 1, 0 );
    }
    else
    {
      //vertical
      colorGrad = QLinearGradient( 0, mMargin, 0, height() - mMargin - 1 );
    }
    QColor transparent = QColor( mCurrentColor );
    transparent.setAlpha( 0 );
    colorGrad.setColorAt( 0, transparent );
    QColor opaque = QColor( mCurrentColor );
    opaque.setAlpha( 255 );
    colorGrad.setColorAt( 1, opaque );
    QBrush colorBrush = QBrush( colorGrad );
    painter.setBrush( colorBrush );
    painter.drawRect( mMargin, mMargin, width() - 2 * mMargin - 1, height() - 2 * mMargin - 1 );
  }

  if ( mOrientation == QgsColorRampWidget::Horizontal )
  {
    //draw marker triangles for horizontal ramps
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setBrush( QBrush( Qt::black ) );
    painter.setPen( Qt::NoPen );
    painter.translate( mMargin + ( width() - 2 * mMargin ) * ( double )componentValue() / componentRange(), mMargin - 1 );
    painter.drawPolygon( mTopTriangle );
    painter.translate( 0, height() - mMargin - 2 );
    painter.setBrush( QBrush( Qt::white ) );
    painter.drawPolygon( mBottomTriangle );
    painter.end();
  }
  else
  {
    //draw cross lines for vertical ramps
    int ypos = mMargin + ( height() - 2 * mMargin - 1 ) - ( height() - 2 * mMargin - 1 ) * ( double )componentValue() / componentRange();
    painter.setBrush( Qt::white );
    painter.setPen( Qt::NoPen );
    painter.drawRect( mMargin, ypos - 1, width() - 2 * mMargin - 1, 3 );
    painter.setPen( Qt::black );
    painter.drawLine( mMargin, ypos, width() - mMargin - 1, ypos );
  }
}

void QgsColorRampWidget::setOrientation( const QgsColorRampWidget::Orientation orientation )
{
  mOrientation = orientation;
  if ( orientation == QgsColorRampWidget::Horizontal )
  {
    //horizontal
    setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  }
  else
  {
    //vertical
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
  }
  updateGeometry();
}

void QgsColorRampWidget::setInteriorMargin( const int margin )
{
  if ( margin == mMargin )
  {
    return;
  }
  mMargin = margin;
  update();
}

void QgsColorRampWidget::setShowFrame( const bool showFrame )
{
  if ( showFrame == mShowFrame )
  {
    return;
  }
  mShowFrame = showFrame;
  update();
}

void QgsColorRampWidget::setMarkerSize( const int markerSize )
{
  //create triangle polygons
  mTopTriangle << QPoint( -markerSize, 0 ) <<  QPoint( markerSize, 0 ) << QPoint( 0, markerSize );
  mBottomTriangle << QPoint( -markerSize, 0 ) << QPoint( markerSize, 0 ) << QPoint( 0, -markerSize );
  update();
}

void QgsColorRampWidget::mouseMoveEvent( QMouseEvent *event )
{
  setColorFromPoint( event->posF() );
  QgsColorWidget::mouseMoveEvent( event );
}

void QgsColorRampWidget::mousePressEvent( QMouseEvent *event )
{
  setColorFromPoint( event->posF() );
}

void QgsColorRampWidget::keyPressEvent( QKeyEvent *event )
{
  int oldValue = componentValue();
  if (( mOrientation == QgsColorRampWidget::Horizontal && ( event->key() == Qt::Key_Right || event->key() == Qt::Key_Up ) )
      || ( mOrientation == QgsColorRampWidget::Vertical && ( event->key() == Qt::Key_Left || event->key() == Qt::Key_Up ) ) )
  {
    setComponentValue( componentValue() + 1 );
  }
  else if (( mOrientation == QgsColorRampWidget::Horizontal && ( event->key() == Qt::Key_Left || event->key() == Qt::Key_Down ) )
           || ( mOrientation == QgsColorRampWidget::Vertical && ( event->key() == Qt::Key_Right || event->key() == Qt::Key_Down ) ) )
  {
    setComponentValue( componentValue() - 1 );
  }
  else if (( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_PageDown )
           || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_PageUp ) )
  {
    setComponentValue( componentValue() + 10 );
  }
  else if (( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_PageUp )
           || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_PageDown ) )
  {
    setComponentValue( componentValue() - 10 );
  }
  else if (( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_Home )
           || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_End ) )
  {
    setComponentValue( 0 );
  }
  else if (( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_End )
           || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_Home ) )
  {
    //set to maximum value
    setComponentValue( componentRange() );
  }
  else
  {
    QgsColorWidget::keyPressEvent( event );
    return;
  }

  if ( componentValue() != oldValue )
  {
    //value has changed
    emit colorChanged( mCurrentColor );
    emit valueChanged( componentValue() );
  }
}

void QgsColorRampWidget::setColorFromPoint( QPointF point )
{
  int oldValue = componentValue();
  int val;
  if ( mOrientation == QgsColorRampWidget::Horizontal )
  {
    val = componentRange() * ( point.x() - mMargin ) / ( width() - 2 * mMargin );
  }
  else
  {
    val = componentRange() - componentRange() * ( point.y() - mMargin ) / ( height() - 2 * mMargin );
  }
  val = qMax( 0, qMin( val, componentRange() ) );
  setComponentValue( val );

  if ( componentValue() != oldValue )
  {
    //value has changed
    emit colorChanged( mCurrentColor );
    emit valueChanged( componentValue() );
  }
}

//
// QgsColorSliderWidget
//

QgsColorSliderWidget::QgsColorSliderWidget( QWidget *parent, const ColorComponent component )
    : QgsColorWidget( parent, component )
    , mRampWidget( nullptr )
    , mSpinBox( nullptr )
{
  QHBoxLayout* hLayout = new QHBoxLayout();
  hLayout->setMargin( 0 );
  hLayout->setSpacing( 5 );

  mRampWidget = new QgsColorRampWidget( nullptr, component );
  mRampWidget->setColor( mCurrentColor );
  hLayout->addWidget( mRampWidget, 1 );

  mSpinBox = new QSpinBox();
  //set spinbox to a reasonable width
  int largestCharWidth = mSpinBox->fontMetrics().width( "888%" );
  mSpinBox->setMinimumWidth( largestCharWidth + 35 );
  mSpinBox->setMinimum( 0 );
  mSpinBox->setMaximum( convertRealToDisplay( componentRange() ) );
  mSpinBox->setValue( convertRealToDisplay( componentValue() ) );
  if ( component == QgsColorWidget::Hue )
  {
    //degrees suffix for hue
    mSpinBox->setSuffix( QChar( 176 ) );
  }
  else if ( component == QgsColorWidget::Saturation || component == QgsColorWidget::Value || component == QgsColorWidget::Alpha )
  {
    mSpinBox->setSuffix( tr( "%" ) );
  }
  hLayout->addWidget( mSpinBox );
  setLayout( hLayout );

  connect( mRampWidget, SIGNAL( valueChanged( int ) ), this, SLOT( rampChanged( int ) ) );
  connect( mRampWidget, SIGNAL( colorChanged( const QColor ) ), this, SLOT( rampColorChanged( const QColor ) ) );
  connect( mSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( spinChanged( int ) ) );
}

QgsColorSliderWidget::~QgsColorSliderWidget()
{
}

void QgsColorSliderWidget::setComponent( const QgsColorWidget::ColorComponent component )
{
  QgsColorWidget::setComponent( component );
  mRampWidget->setComponent( component );
  mSpinBox->setMaximum( convertRealToDisplay( componentRange() ) );
  if ( component == QgsColorWidget::Hue )
  {
    //degrees suffix for hue
    mSpinBox->setSuffix( QChar( 176 ) );
  }
  else if ( component == QgsColorWidget::Saturation || component == QgsColorWidget::Value || component == QgsColorWidget::Alpha )
  {
    //saturation, value and alpha are in %
    mSpinBox->setSuffix( tr( "%" ) );
  }
  else
  {
    //clear suffix
    mSpinBox->setSuffix( QString() );
  }
}

void QgsColorSliderWidget::setComponentValue( const int value )
{
  QgsColorWidget::setComponentValue( value );
  mRampWidget->blockSignals( true );
  mRampWidget->setComponentValue( value );
  mRampWidget->blockSignals( false );
  mSpinBox->blockSignals( true );
  mSpinBox->setValue( convertRealToDisplay( value ) );
  mSpinBox->blockSignals( false );
}

void QgsColorSliderWidget::setColor( const QColor &color, bool emitSignals )
{
  QgsColorWidget::setColor( color, emitSignals );
  mRampWidget->setColor( color );
  mSpinBox->blockSignals( true );
  mSpinBox->setValue( convertRealToDisplay( componentValue() ) );
  mSpinBox->blockSignals( false );
}

void QgsColorSliderWidget::rampColorChanged( const QColor &color )
{
  emit colorChanged( color );
}

void QgsColorSliderWidget::spinChanged( int value )
{
  int convertedValue = convertDisplayToReal( value );
  QgsColorWidget::setComponentValue( convertedValue );
  mRampWidget->setComponentValue( convertedValue );
  emit colorChanged( mCurrentColor );
}

void QgsColorSliderWidget::rampChanged( int value )
{
  mSpinBox->blockSignals( true );
  mSpinBox->setValue( convertRealToDisplay( value ) );
  mSpinBox->blockSignals( false );
}


int QgsColorSliderWidget::convertRealToDisplay( const int realValue ) const
{
  //scale saturation, value or alpha to 0->100 range. This makes more sense for users
  //for whom "255" is a totally arbitrary value!
  if ( mComponent == QgsColorWidget::Saturation || mComponent == QgsColorWidget::Value || mComponent == QgsColorWidget::Alpha )
  {
    return qRound( 100.0 * realValue / 255.0 );
  }

  //leave all other values intact
  return realValue;
}

int QgsColorSliderWidget::convertDisplayToReal( const int displayValue ) const
{
  //scale saturation, value or alpha from 0->100 range (see note in convertRealToDisplay)
  if ( mComponent == QgsColorWidget::Saturation || mComponent == QgsColorWidget::Value || mComponent == QgsColorWidget::Alpha )
  {
    return qRound( 255.0 * displayValue / 100.0 );
  }

  //leave all other values intact
  return displayValue;
}

//
// QgsColorTextWidget
//

QgsColorTextWidget::QgsColorTextWidget( QWidget *parent )
    : QgsColorWidget( parent )
    , mLineEdit( nullptr )
    , mMenuButton( nullptr )
    , mFormat( QgsColorTextWidget::HexRgb )
{
  QHBoxLayout* hLayout = new QHBoxLayout();
  hLayout->setMargin( 0 );
  hLayout->setSpacing( 0 );

  mLineEdit = new QLineEdit( nullptr );
  hLayout->addWidget( mLineEdit );

  mMenuButton = new QToolButton( mLineEdit );
  mMenuButton->setIcon( QgsApplication::getThemeIcon( "/mIconDropDownMenu.svg" ) );
  mMenuButton->setCursor( Qt::ArrowCursor );
  mMenuButton->setFocusPolicy( Qt::NoFocus );
  mMenuButton->setStyleSheet( "QToolButton { border: none; padding: 0px; }" );

  setLayout( hLayout );

  int frameWidth = mLineEdit->style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  mLineEdit->setStyleSheet( QString( "QLineEdit { padding-right: %1px; } " )
                            .arg( mMenuButton->sizeHint().width() + frameWidth + 1 ) );

  connect( mLineEdit, SIGNAL( editingFinished() ), this, SLOT( textChanged() ) );
  connect( mMenuButton, SIGNAL( clicked() ), this, SLOT( showMenu() ) );

  //restore format setting
  QSettings settings;
  mFormat = ( ColorTextFormat )settings.value( "/ColorWidgets/textWidgetFormat", 0 ).toInt();

  updateText();
}

QgsColorTextWidget::~QgsColorTextWidget()
{

}

void QgsColorTextWidget::setColor( const QColor &color, const bool emitSignals )
{
  QgsColorWidget::setColor( color, emitSignals );
  updateText();
}

void QgsColorTextWidget::resizeEvent( QResizeEvent * event )
{
  Q_UNUSED( event );
  QSize sz = mMenuButton->sizeHint();
  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  mMenuButton->move( mLineEdit->rect().right() - frameWidth - sz.width(),
                     ( mLineEdit->rect().bottom() + 1 - sz.height() ) / 2 );
}

void QgsColorTextWidget::updateText()
{
  switch ( mFormat )
  {
    case HexRgb:
      mLineEdit->setText( mCurrentColor.name() );
      break;
    case HexRgbA:
      mLineEdit->setText( mCurrentColor.name() + QString( "%1" ).arg( mCurrentColor.alpha(), 2, 16, QChar( '0' ) ) );
      break;
    case Rgb:
      mLineEdit->setText( QString( tr( "rgb( %1, %2, %3 )" ) ).arg( mCurrentColor.red() ).arg( mCurrentColor.green() ).arg( mCurrentColor.blue() ) );
      break;
    case Rgba:
      mLineEdit->setText( QString( tr( "rgba( %1, %2, %3, %4 )" ) ).arg( mCurrentColor.red() ).arg( mCurrentColor.green() ).arg( mCurrentColor.blue() ).arg( QString::number( mCurrentColor.alphaF(), 'f', 2 ) ) );
      break;
  }
}

void QgsColorTextWidget::textChanged()
{
  QString testString = mLineEdit->text();
  bool containsAlpha;
  QColor color = QgsSymbolLayerV2Utils::parseColorWithAlpha( testString, containsAlpha );
  if ( !color.isValid() )
  {
    //bad color string
    updateText();
    return;
  }

  //good color string
  if ( color != mCurrentColor )
  {
    //retain alpha if no explicit alpha set
    if ( !containsAlpha )
    {
      color.setAlpha( mCurrentColor.alpha() );
    }
    //color has changed
    mCurrentColor = color;
    emit colorChanged( mCurrentColor );
  }
  updateText();
}

void QgsColorTextWidget::showMenu()
{
  QMenu colorContextMenu;

  QAction* hexRgbAction = new QAction( tr( "#RRGGBB" ), nullptr );
  colorContextMenu.addAction( hexRgbAction );
  QAction* hexRgbaAction = new QAction( tr( "#RRGGBBAA" ), nullptr );
  colorContextMenu.addAction( hexRgbaAction );
  QAction* rgbAction = new QAction( tr( "rgb( r, g, b )" ), nullptr );
  colorContextMenu.addAction( rgbAction );
  QAction* rgbaAction = new QAction( tr( "rgba( r, g, b, a )" ), nullptr );
  colorContextMenu.addAction( rgbaAction );

  QAction* selectedAction = colorContextMenu.exec( QCursor::pos() );
  if ( selectedAction == hexRgbAction )
  {
    mFormat = QgsColorTextWidget::HexRgb;
  }
  else if ( selectedAction == hexRgbaAction )
  {
    mFormat = QgsColorTextWidget::HexRgbA;
  }
  else if ( selectedAction == rgbAction )
  {
    mFormat = QgsColorTextWidget::Rgb;
  }
  else if ( selectedAction == rgbaAction )
  {
    mFormat = QgsColorTextWidget::Rgba;
  }

  //save format setting
  QSettings settings;
  settings.setValue( "/ColorWidgets/textWidgetFormat", ( int )mFormat );

  updateText();
}


//
// QgsColorPreviewWidget
//

QgsColorPreviewWidget::QgsColorPreviewWidget( QWidget *parent )
    : QgsColorWidget( parent )
    , mColor2( QColor() )
{

}

QgsColorPreviewWidget::~QgsColorPreviewWidget()
{

}

void QgsColorPreviewWidget::drawColor( const QColor &color, QRect rect, QPainter& painter )
{
  painter.setPen( Qt::NoPen );
  //if color has an alpha, start with a checkboard pattern
  if ( color.alpha() < 255 )
  {
    QBrush checkBrush = QBrush( transparentBackground() );
    painter.setBrush( checkBrush );
    painter.drawRect( rect );

    //draw half of widget showing solid color, the other half showing color with alpha

    //ensure at least a 1px overlap to avoid artifacts
    QBrush colorBrush = QBrush( color );
    painter.setBrush( colorBrush );
    painter.drawRect( floor( rect.width() / 2.0 ) + rect.left(), rect.top(), rect.width() - floor( rect.width() / 2.0 ), rect.height() );

    QColor opaqueColor = QColor( color );
    opaqueColor.setAlpha( 255 );
    QBrush opaqueBrush = QBrush( opaqueColor );
    painter.setBrush( opaqueBrush );
    painter.drawRect( rect.left(), rect.top(), ceil( rect.width() / 2.0 ), rect.height() );
  }
  else
  {
    //no alpha component, just draw a solid rectangle
    QBrush brush = QBrush( color );
    painter.setBrush( brush );
    painter.drawRect( rect );
  }
}

void QgsColorPreviewWidget::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );
  QPainter painter( this );

  if ( mColor2.isValid() )
  {
    //drawing with two color sections
    int verticalSplit = qRound( height() / 2.0 );
    drawColor( mCurrentColor, QRect( 0, 0, width(), verticalSplit ), painter );
    drawColor( mColor2, QRect( 0, verticalSplit, width(), height() - verticalSplit ), painter );
  }
  else if ( mCurrentColor.isValid() )
  {
    drawColor( mCurrentColor, QRect( 0, 0, width(), height() ), painter );
  }

  painter.end();
}

QSize QgsColorPreviewWidget::sizeHint() const
{
  return QSize( 200, 150 );
}

void QgsColorPreviewWidget::setColor2( const QColor &color )
{
  if ( color == mColor2 )
  {
    return;
  }
  mColor2 = color;
  update();
}

void QgsColorPreviewWidget::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    mDragStartPosition = e->pos();
  }
  QgsColorWidget::mousePressEvent( e );
}

void QgsColorPreviewWidget::mouseReleaseEvent( QMouseEvent *e )
{
  if (( e->pos() - mDragStartPosition ).manhattanLength() >= QApplication::startDragDistance() )
  {
    //mouse moved, so a drag. nothing to do here
    QgsColorWidget::mouseReleaseEvent( e );
    return;
  }

  //work out which color was clicked
  QColor clickedColor = mCurrentColor;
  if ( mColor2.isValid() )
  {
    //two color sections, check if dragged color was the second color
    int verticalSplit = qRound( height() / 2.0 );
    if ( mDragStartPosition.y() >= verticalSplit )
    {
      clickedColor = mColor2;
    }
  }
  emit colorChanged( clickedColor );

}

void QgsColorPreviewWidget::mouseMoveEvent( QMouseEvent *e )
{
  //handle dragging colors from button

  if ( !( e->buttons() & Qt::LeftButton ) )
  {
    //left button not depressed, so not a drag
    QgsColorWidget::mouseMoveEvent( e );
    return;
  }

  if (( e->pos() - mDragStartPosition ).manhattanLength() < QApplication::startDragDistance() )
  {
    //mouse not moved, so not a drag
    QgsColorWidget::mouseMoveEvent( e );
    return;
  }

  //user is dragging color

  //work out which color is being dragged
  QColor dragColor = mCurrentColor;
  if ( mColor2.isValid() )
  {
    //two color sections, check if dragged color was the second color
    int verticalSplit = qRound( height() / 2.0 );
    if ( mDragStartPosition.y() >= verticalSplit )
    {
      dragColor = mColor2;
    }
  }

  QDrag *drag = new QDrag( this );
  drag->setMimeData( QgsSymbolLayerV2Utils::colorToMimeData( dragColor ) );
  drag->setPixmap( createDragIcon( dragColor ) );
  drag->exec( Qt::CopyAction );
}


//
// QgsColorWidgetAction
//

QgsColorWidgetAction::QgsColorWidgetAction( QgsColorWidget* colorWidget, QMenu* menu, QWidget* parent )
    : QWidgetAction( parent )
    , mMenu( menu )
    , mColorWidget( colorWidget )
    , mSuppressRecurse( false )
    , mDismissOnColorSelection( true )
{
  setDefaultWidget( mColorWidget );
  connect( mColorWidget, SIGNAL( colorChanged( QColor ) ), this, SLOT( setColor( QColor ) ) );

  connect( this, SIGNAL( hovered() ), this, SLOT( onHover() ) );
  connect( mColorWidget, SIGNAL( hovered() ), this, SLOT( onHover() ) );
}

QgsColorWidgetAction::~QgsColorWidgetAction()
{

}

void QgsColorWidgetAction::onHover()
{
  //see https://bugreports.qt-project.org/browse/QTBUG-10427?focusedCommentId=185610&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-185610
  if ( mSuppressRecurse )
  {
    return;
  }

  if ( mMenu )
  {
    mSuppressRecurse = true;
    mMenu->setActiveAction( this );
    mSuppressRecurse = false;
  }
}

void QgsColorWidgetAction::setColor( const QColor& color )
{
  emit colorChanged( color );
  QAction::trigger();
  if ( mMenu && mDismissOnColorSelection )
  {
    mMenu->hide();
  }
}
