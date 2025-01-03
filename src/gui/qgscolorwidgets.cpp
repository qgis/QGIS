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
#include "moc_qgscolorwidgets.cpp"
#include "qgsapplication.h"
#include "qgssymbollayerutils.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsguiutils.h"
#include "qgsdoublespinbox.h"

#include <QResizeEvent>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <QStyleOptionFrameV3>
#else
#include <QStyleOptionFrame>
#endif
#include <QPainter>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFontMetrics>
#include <QToolButton>
#include <QMenu>
#include <QDrag>
#include <QRectF>
#include <QLineF>

#include <cmath>

#define HUE_MAX 359


// TODO QGIS 4 remove typedef, QColor was qreal (double) and is now float
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
typedef qreal float_type;
#else
typedef float float_type;
#endif


//
// QgsColorWidget
//

QgsColorWidget::QgsColorWidget( QWidget *parent, const ColorComponent component )
  : QWidget( parent )
  , mCurrentColor( Qt::red )
  , mComponent( component )
{
  setAcceptDrops( true );
}

int QgsColorWidget::componentValue() const
{
  return static_cast<int>( std::round( componentValueF( mComponent ) * static_cast<float>( componentRange() ) ) );
}

float QgsColorWidget::componentValueF() const
{
  return componentValueF( mComponent );
}

QPixmap QgsColorWidget::createDragIcon( const QColor &color )
{
  //craft a pixmap for the drag icon
  const int iconSize = QgsGuiUtils::scaleIconSize( 50 );
  QPixmap pixmap( iconSize, iconSize );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );
  //start with a light gray background
  painter.fillRect( QRect( 0, 0, iconSize, iconSize ), QBrush( QColor( 200, 200, 200 ) ) );
  //draw rect with white border, filled with current color
  QColor pixmapColor = color;
  pixmapColor.setAlpha( 255 );
  painter.setBrush( QBrush( pixmapColor ) );
  painter.setPen( QPen( Qt::white ) );
  painter.drawRect( QRect( 1, 1, iconSize - 2, iconSize - 2 ) );
  painter.end();
  return pixmap;
}

QgsColorWidget::ComponentUnit QgsColorWidget::componentUnit( ColorComponent component )
{
  switch ( component )
  {
    case QgsColorWidget::Hue:
      return ComponentUnit::Degree;
    case QgsColorWidget::Saturation:
    case QgsColorWidget::Value:
    case QgsColorWidget::Alpha:
    case QgsColorWidget::Cyan:
    case QgsColorWidget::Magenta:
    case QgsColorWidget::Yellow:
    case QgsColorWidget::Black:
      return ComponentUnit::Percent;

    case QgsColorWidget::Multiple:
    case QgsColorWidget::Red:
    case QgsColorWidget::Green:
    case QgsColorWidget::Blue:
      return ComponentUnit::Scaled0to255;
  }

  BUILTIN_UNREACHABLE
}

int QgsColorWidget::componentValue( const QgsColorWidget::ColorComponent component ) const
{
  return static_cast<int>( std::round( componentValueF( component ) * static_cast<float>( componentRange( component ) ) ) );
}

float QgsColorWidget::componentValueF( const QgsColorWidget::ColorComponent component ) const
{
  if ( !mCurrentColor.isValid() )
  {
    return -1;
  }

  // TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
  // NOLINTBEGIN(bugprone-narrowing-conversions)
  switch ( component )
  {
    case QgsColorWidget::Red:
      return mCurrentColor.redF();
    case QgsColorWidget::Green:
      return mCurrentColor.greenF();
    case QgsColorWidget::Blue:
      return mCurrentColor.blueF();
    case QgsColorWidget::Hue:
      //hue is treated specially, to avoid -1 hues values from QColor for ambiguous hues
      return hueF();
    case QgsColorWidget::Saturation:
      return mCurrentColor.hsvSaturationF();
    case QgsColorWidget::Value:
      return mCurrentColor.valueF();
    case QgsColorWidget::Alpha:
      return mCurrentColor.alphaF();
    case QgsColorWidget::Cyan:
      return mCurrentColor.cyanF();
    case QgsColorWidget::Yellow:
      return mCurrentColor.yellowF();
    case QgsColorWidget::Magenta:
      return mCurrentColor.magentaF();
    case QgsColorWidget::Black:
      return mCurrentColor.blackF();
    default:
      return -1;
  }
  // NOLINTEND(bugprone-narrowing-conversions)
}

int QgsColorWidget::componentRange() const
{
  return componentRange( mComponent );
}

int QgsColorWidget::componentRange( const QgsColorWidget::ColorComponent component )
{
  if ( component == QgsColorWidget::Multiple )
  {
    //no component
    return -1;
  }

  if ( component == QgsColorWidget::Hue )
  {
    //hue ranges to HUE_MAX
    return HUE_MAX;
  }
  else
  {
    //all other components range to 255
    return 255;
  }
}

int QgsColorWidget::hue() const
{
  return static_cast<int>( std::round( hueF() * HUE_MAX ) );
}

float QgsColorWidget::hueF() const
{
  if ( mCurrentColor.hueF() >= 0 )
  {
    return mCurrentColor.hueF(); // NOLINT(bugprone-narrowing-conversions): TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
  }
  else
  {
    return mExplicitHue;
  }
}

void QgsColorWidget::alterColor( QColor &color, const QgsColorWidget::ColorComponent component, const int newValue )
{
  //clip value to sensible range
  const float clippedValue = static_cast<float>( std::clamp( newValue, 0, componentRange( component ) ) ) / static_cast<float>( componentRange( component ) );
  alterColorF( color, component, clippedValue );
}

void QgsColorWidget::alterColorF( QColor &color, const QgsColorWidget::ColorComponent component, const float newValue )
{
  float_type clippedValue = std::clamp( newValue, 0.f, 1.f );

  if ( colorSpec( component ) == QColor::Spec::Cmyk )
  {
    float_type c, m, y, k, a;
    color.getCmykF( &c, &m, &y, &k, &a );

    switch ( component )
    {
      case QgsColorWidget::Cyan:
        color.setCmykF( clippedValue, m, y, k, a );
        break;
      case QgsColorWidget::Magenta:
        color.setCmykF( c, clippedValue, y, k, a );
        break;
      case QgsColorWidget::Yellow:
        color.setCmykF( c, m, clippedValue, k, a );
        break;
      case QgsColorWidget::Black:
        color.setCmykF( c, m, y, clippedValue, a );
        break;
      default:
        return;
    }
  }
  else
  {
    float_type r, g, b, a;
    color.getRgbF( &r, &g, &b, &a );
    float_type h, s, v;
    color.getHsvF( &h, &s, &v );

    switch ( component )
    {
      case QgsColorWidget::Red:
        color.setRedF( clippedValue );
        break;
      case QgsColorWidget::Green:
        color.setGreenF( clippedValue );
        break;
      case QgsColorWidget::Blue:
        color.setBlueF( clippedValue );
        break;
      case QgsColorWidget::Hue:
        color.setHsvF( clippedValue, s, v, a );
        break;
      case QgsColorWidget::Saturation:
        color.setHsvF( h, clippedValue, v, a );
        break;
      case QgsColorWidget::Value:
        color.setHsvF( h, s, clippedValue, a );
        break;
      case QgsColorWidget::Alpha:
        color.setAlphaF( clippedValue );
        break;
      default:
        return;
    }
  }
}

QColor::Spec QgsColorWidget::colorSpec( QgsColorWidget::ColorComponent component )
{
  switch ( component )
  {
    case Red:
    case Green:
    case Blue:
      return QColor::Spec::Rgb;

    case Hue:
    case Saturation:
    case Value:
      return QColor::Spec::Hsv;

    case Cyan:
    case Magenta:
    case Yellow:
    case Black:
      return QColor::Spec::Cmyk;

    default:
      return QColor::Spec::Invalid;
  }
}

QColor::Spec QgsColorWidget::colorSpec() const
{
  return colorSpec( mComponent );
}

const QPixmap &QgsColorWidget::transparentBackground()
{
  static QPixmap sTranspBkgrd;

  if ( sTranspBkgrd.isNull() )
    sTranspBkgrd = QgsApplication::getThemePixmap( QStringLiteral( "/transp-background_8x8.png" ) );

  return sTranspBkgrd;
}

void QgsColorWidget::dragEnterEvent( QDragEnterEvent *e )
{
  //is dragged data valid color data?
  bool hasAlpha;
  const QColor mimeColor = QgsSymbolLayerUtils::colorFromMimeData( e->mimeData(), hasAlpha );

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
  QColor mimeColor = QgsSymbolLayerUtils::colorFromMimeData( e->mimeData(), hasAlpha );

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
  setComponentValueF( static_cast<float>( value ) );
}

void QgsColorWidget::setComponentValueF( const float value )
{
  if ( mComponent == QgsColorWidget::Multiple )
  {
    return;
  }

  //overwrite hue with explicit hue if required
  if ( mComponent == QgsColorWidget::Saturation || mComponent == QgsColorWidget::Value )
  {
    float_type h, s, v, a;
    mCurrentColor.getHsvF( &h, &s, &v, &a );

    h = hueF();

    mCurrentColor.setHsvF( h, s, v, a );
  }

  alterColorF( mCurrentColor, mComponent, value );

  //update recorded hue
  if ( mCurrentColor.hue() >= 0 )
  {
    mExplicitHue = mCurrentColor.hueF(); // NOLINT(bugprone-narrowing-conversions): TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
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
    mExplicitHue = color.hueF(); // NOLINT(bugprone-narrowing-conversions): TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
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
{
  //create wheel hue brush - only do this once
  QConicalGradient wheelGradient = QConicalGradient( 0, 0, 0 );
  const int wheelStops = 20;
  QColor gradColor = QColor::fromHsvF( 1.0, 1.0, 1.0 );
  for ( int pos = 0; pos <= wheelStops; ++pos )
  {
    const double relativePos = static_cast<double>( pos ) / wheelStops;
    gradColor.setHsvF( relativePos, 1, 1 );
    wheelGradient.setColorAt( relativePos, gradColor );
  }
  mWheelBrush = QBrush( wheelGradient );
}

QgsColorWheel::~QgsColorWheel() = default;

QSize QgsColorWheel::sizeHint() const
{
  const int size = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 22;
  return QSize( size, size );
}

void QgsColorWheel::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event )
  QPainter painter( this );

  if ( mWidgetImage.isNull() || mWheelImage.isNull() || mTriangleImage.isNull() )
  {
    createImages( size() );
  }

  //draw everything in an image
  mWidgetImage.fill( Qt::transparent );
  QPainter imagePainter( &mWidgetImage );
  imagePainter.setRenderHint( QPainter::Antialiasing );

  if ( mWheelDirty )
  {
    //need to redraw the wheel image
    createWheel();
  }

  //draw wheel centered on widget
  const QPointF center = QPointF( mWidgetImage.width() / 2.0, mWidgetImage.height() / 2.0 );
  imagePainter.drawImage( QPointF( center.x() - ( mWheelImage.width() / 2.0 ), center.y() - ( mWheelImage.height() / 2.0 ) ), mWheelImage );

  //draw hue marker
  const float h = hueF() * HUE_MAX;
  const double length = mWheelImage.width() / 2.0;
  QLineF hueMarkerLine = QLineF( center.x(), center.y(), center.x() + length, center.y() );
  hueMarkerLine.setAngle( h );
  imagePainter.save();
  //use sourceIn mode for nicer antialiasing
  imagePainter.setCompositionMode( QPainter::CompositionMode_SourceIn );
  QPen pen;
  pen.setWidthF( 2 * devicePixelRatioF() );
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
  imagePainter.drawImage( QPointF( center.x() - ( mWheelImage.width() / 2.0 ), center.y() - ( mWheelImage.height() / 2.0 ) ), mTriangleImage );

  //draw current color marker
  const double triangleRadius = length - mWheelThickness * devicePixelRatioF() - 1;

  //adapted from equations at https://github.com/timjb/colortriangle/blob/master/colortriangle.js by Tim Baumann
  const double lightness = mCurrentColor.lightnessF();
  const double hueRadians = ( h * M_PI / 180.0 );
  const double hx = std::cos( hueRadians ) * triangleRadius;
  const double hy = -std::sin( hueRadians ) * triangleRadius;
  const double sx = -std::cos( -hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  const double sy = -std::sin( -hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  const double vx = -std::cos( hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  const double vy = std::sin( hueRadians + ( M_PI / 3.0 ) ) * triangleRadius;
  const double mx = ( sx + vx ) / 2.0;
  const double my = ( sy + vy ) / 2.0;

  const double a = ( 1 - 2.0 * std::fabs( lightness - 0.5 ) ) * mCurrentColor.hslSaturationF();
  const double x = sx + ( vx - sx ) * lightness + ( hx - mx ) * a;
  const double y = sy + ( vy - sy ) * lightness + ( hy - my ) * a;

  //adapt pen color for lightness
  pen.setColor( lightness > 0.7 ? Qt::black : Qt::white );
  imagePainter.setPen( pen );
  imagePainter.setBrush( Qt::NoBrush );
  imagePainter.drawEllipse( QPointF( x + center.x(), y + center.y() ), 4.0 * devicePixelRatioF(), 4.0 * devicePixelRatioF() );
  imagePainter.end();

  //draw image onto widget
  painter.drawImage( QRectF( 0, 0, width(), height() ), mWidgetImage );
  painter.end();
}

void QgsColorWheel::setColor( const QColor &color, const bool emitSignals )
{
  if ( color.hue() >= 0 && !qgsDoubleNear( color.hue(), hueF() ) )
  {
    //hue has changed, need to redraw the triangle
    mTriangleDirty = true;
  }

  QgsColorWidget::setColor( color, emitSignals );
}

void QgsColorWheel::createImages( const QSizeF size )
{
  const double wheelSize = std::min( size.width(), size.height() ) - mMargin * 2.0;
  mWheelThickness = wheelSize / 15.0;

  //recreate cache images at correct size
  const double pixelRatio = devicePixelRatioF();
  mWheelImage = QImage( wheelSize * pixelRatio, wheelSize * pixelRatio, QImage::Format_ARGB32 );
  mTriangleImage = QImage( wheelSize * pixelRatio, wheelSize * pixelRatio, QImage::Format_ARGB32 );
  mWidgetImage = QImage( size.width() * pixelRatio, size.height() * pixelRatio, QImage::Format_ARGB32 );

  //trigger a redraw for the images
  mWheelDirty = true;
  mTriangleDirty = true;
}

void QgsColorWheel::resizeEvent( QResizeEvent *event )
{
  QgsColorWidget::resizeEvent( event );
#ifdef Q_OS_WIN
  // For some reason the first reported size than that of the parent widget, leading to a cut-off color wheel
  if ( event->size().width() > parentWidget()->size().width() )
  {
    QSize newSize(
      std::min( event->size().width(), parentWidget()->size().width() - 2 ),
      std::min( event->size().height(), parentWidget()->size().height() - 2 )
    );
    resize( newSize );
    createImages( newSize );
  }
  else
  {
    createImages( event->size() );
  }
#else
  //recreate images for new size
  createImages( event->size() );
#endif
}

void QgsColorWheel::setColorFromPos( const QPointF pos )
{
  const QPointF center = QPointF( width() / 2.0, height() / 2.0 );
  //line from center to mouse position
  const QLineF line = QLineF( center.x(), center.y(), pos.x(), pos.y() );

  QColor newColor = QColor();

  float_type h, s, l, alpha;
  mCurrentColor.getHslF( &h, &s, &l, &alpha );
  //override hue with explicit hue, so we don't get -1 values from QColor for hue
  h = hueF();

  if ( mClickedPart == QgsColorWheel::Triangle )
  {
    //adapted from equations at https://github.com/timjb/colortriangle/blob/master/colortriangle.js by Tim Baumann

    //position of event relative to triangle center
    const double x = pos.x() - center.x();
    const double y = pos.y() - center.y();

    double eventAngleRadians = line.angle() * M_PI / 180.0;
    const double hueRadians = h * 2 * M_PI;
    double rad0 = std::fmod( eventAngleRadians + 2.0 * M_PI - hueRadians, 2.0 * M_PI );
    double rad1 = std::fmod( rad0, ( ( 2.0 / 3.0 ) * M_PI ) ) - ( M_PI / 3.0 );
    const double length = mWheelImage.width() / 2.0 / devicePixelRatioF();
    const double triangleLength = length - mWheelThickness - 1;

    const double a = 0.5 * triangleLength;
    double b = std::tan( rad1 ) * a;
    double r = std::sqrt( x * x + y * y );
    const double maxR = std::sqrt( a * a + b * b );

    if ( r > maxR )
    {
      const double dx = std::tan( rad1 ) * r;
      double rad2 = std::atan( dx / maxR );
      rad2 = std::min( rad2, M_PI / 3.0 );
      rad2 = std::max( rad2, -M_PI / 3.0 );
      eventAngleRadians += rad2 - rad1;
      rad0 = std::fmod( eventAngleRadians + 2.0 * M_PI - hueRadians, 2.0 * M_PI );
      rad1 = std::fmod( rad0, ( ( 2.0 / 3.0 ) * M_PI ) ) - ( M_PI / 3.0 );
      b = std::tan( rad1 ) * a;
      r = std::sqrt( a * a + b * b );
    }

    const double triangleSideLength = std::sqrt( 3.0 ) * triangleLength;
    const double newL = ( ( -std::sin( rad0 ) * r ) / triangleSideLength ) + 0.5;
    const double widthShare = 1.0 - ( std::fabs( newL - 0.5 ) * 2.0 );
    const double newS = ( ( ( std::cos( rad0 ) * r ) + ( triangleLength / 2.0 ) ) / ( 1.5 * triangleLength ) ) / widthShare;
    s = std::min( std::max( 0.f, static_cast<float>( newS ) ), 1.f );
    l = std::min( std::max( 0.f, static_cast<float>( newL ) ), 1.f );
    newColor = QColor::fromHslF( h, s, l );
    //explicitly set the hue again, so that it's exact
    newColor.setHsvF( h, newColor.hsvSaturationF(), newColor.valueF(), alpha );
  }
  else if ( mClickedPart == QgsColorWheel::Wheel )
  {
    //use hue angle
    s = mCurrentColor.hsvSaturationF();
    const float v = mCurrentColor.valueF(); // NOLINT(bugprone-narrowing-conversions): TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
    const qreal newHue = line.angle() / HUE_MAX;
    newColor = QColor::fromHsvF( static_cast<float>( newHue ), s, v, alpha );
    //hue has changed, need to redraw triangle
    mTriangleDirty = true;
  }

  if ( newColor.isValid() && newColor != mCurrentColor )
  {
    //color has changed
    mCurrentColor = QColor( newColor );

    if ( mCurrentColor.hueF() >= 0 )
    {
      //color has a valid hue, so update the QgsColorWidget's explicit hue
      mExplicitHue = mCurrentColor.hueF(); // NOLINT(bugprone-narrowing-conversions): TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
    }

    update();
    emit colorChanged( mCurrentColor );
  }
}

void QgsColorWheel::mouseMoveEvent( QMouseEvent *event )
{
  if ( mIsDragging )
    setColorFromPos( event->pos() );

  QgsColorWidget::mouseMoveEvent( event );
}

void QgsColorWheel::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    mIsDragging = true;
    //calculate where the event occurred -- on the wheel or inside the triangle?

    //create a line from the widget's center to the event
    const QLineF line = QLineF( width() / 2.0, height() / 2.0, event->pos().x(), event->pos().y() );

    const double innerLength = mWheelImage.width() / 2.0 / devicePixelRatioF() - mWheelThickness;
    if ( line.length() < innerLength )
    {
      mClickedPart = QgsColorWheel::Triangle;
    }
    else
    {
      mClickedPart = QgsColorWheel::Wheel;
    }
    setColorFromPos( event->pos() );
  }
  else
  {
    QgsColorWidget::mousePressEvent( event );
  }
}

void QgsColorWheel::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    mIsDragging = false;
    mClickedPart = QgsColorWheel::None;
  }
  else
  {
    QgsColorWidget::mouseReleaseEvent( event );
  }
}

void QgsColorWheel::createWheel()
{
  if ( mWheelImage.isNull() )
  {
    return;
  }

  const int maxSize = std::min( mWheelImage.width(), mWheelImage.height() );
  const double wheelRadius = maxSize / 2.0;

  mWheelImage.fill( Qt::transparent );
  QPainter p( &mWheelImage );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( mWheelBrush );
  p.setPen( Qt::NoPen );

  //draw hue wheel as a circle
  p.translate( wheelRadius, wheelRadius );
  p.drawEllipse( QPointF( 0.0, 0.0 ), wheelRadius, wheelRadius );

  //cut hole in center of circle to make a ring
  p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
  p.setBrush( QBrush( Qt::black ) );
  p.drawEllipse( QPointF( 0, 0 ), wheelRadius - mWheelThickness * devicePixelRatioF(), wheelRadius - mWheelThickness * devicePixelRatioF() );
  p.end();

  mWheelDirty = false;
}

void QgsColorWheel::createTriangle()
{
  if ( mWheelImage.isNull() || mTriangleImage.isNull() )
  {
    return;
  }

  const QPointF center = QPointF( mWheelImage.width() / 2.0, mWheelImage.height() / 2.0 );
  mTriangleImage.fill( Qt::transparent );

  QPainter imagePainter( &mTriangleImage );
  imagePainter.setRenderHint( QPainter::Antialiasing );

  const float angle = hueF();
  const float angleDegree = angle * HUE_MAX;
  const double wheelRadius = mWheelImage.width() / 2.0;
  const double triangleRadius = wheelRadius - mWheelThickness * devicePixelRatioF() - 1;

  //pure version of hue (at full saturation and value)
  const QColor pureColor = QColor::fromHsvF( angle, 1., 1. );
  //create copy of color but with 0 alpha
  QColor alphaColor = QColor( pureColor );
  alphaColor.setAlpha( 0 );

  //some rather ugly shortcuts to obtain corners and midpoints of triangle
  QLineF line1 = QLineF( center.x(), center.y(), center.x() - triangleRadius * std::cos( M_PI / 3.0 ), center.y() - triangleRadius * std::sin( M_PI / 3.0 ) );
  QLineF line2 = QLineF( center.x(), center.y(), center.x() + triangleRadius, center.y() );
  QLineF line3 = QLineF( center.x(), center.y(), center.x() - triangleRadius * std::cos( M_PI / 3.0 ), center.y() + triangleRadius * std::sin( M_PI / 3.0 ) );
  QLineF line4 = QLineF( center.x(), center.y(), center.x() - triangleRadius * std::cos( M_PI / 3.0 ), center.y() );
  QLineF line5 = QLineF( center.x(), center.y(), ( line2.p2().x() + line1.p2().x() ) / 2.0, ( line2.p2().y() + line1.p2().y() ) / 2.0 );
  line1.setAngle( line1.angle() + angleDegree );
  line2.setAngle( line2.angle() + angleDegree );
  line3.setAngle( line3.angle() + angleDegree );
  line4.setAngle( line4.angle() + angleDegree );
  line5.setAngle( line5.angle() + angleDegree );
  const QPointF p1 = line1.p2();
  const QPointF p2 = line2.p2();
  const QPointF p3 = line3.p2();
  const QPointF p4 = line4.p2();
  const QPointF p5 = line5.p2();

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
  const int size = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 22;
  return QSize( size, size );
}

void QgsColorBox::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event )
  QPainter painter( this );

  QStyleOptionFrame option;
  option.initFrom( this );
  option.state = hasFocus() ? QStyle::State_Active : QStyle::State_None;
  style()->drawPrimitive( QStyle::PE_Frame, &option, &painter );

  if ( mDirty )
  {
    createBox();
  }

  //draw background image
  painter.drawImage( QPoint( mMargin, mMargin ), *mBoxImage );

  //draw cross lines
  const double h = height();
  const double w = width();
  const double margin = mMargin;
  const double xPos = ( mMargin + ( w - 2 * mMargin - 1 ) * xComponentValue() );
  const double yPos = ( mMargin + ( h - 2 * mMargin - 1 ) - ( h - 2 * mMargin - 1 ) * yComponentValue() );

  painter.setBrush( Qt::white );
  painter.setPen( Qt::NoPen );

  painter.drawRect( QRectF( xPos - 1, mMargin, 3, height() - 2 * margin - 1 ) );
  painter.drawRect( QRectF( mMargin, yPos - 1, width() - 2 * margin - 1, 3 ) );
  painter.setPen( Qt::black );
  painter.drawLine( QLineF( xPos, mMargin, xPos, height() - margin - 1 ) );
  painter.drawLine( QLineF( mMargin, yPos, width() - margin - 1, yPos ) );

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
  mDirty |= ( ( mComponent == QgsColorWidget::Red && !qgsDoubleNear( mCurrentColor.redF(), color.redF() ) ) || ( mComponent == QgsColorWidget::Green && !qgsDoubleNear( mCurrentColor.greenF(), color.greenF() ) ) || ( mComponent == QgsColorWidget::Blue && !qgsDoubleNear( mCurrentColor.blueF(), color.blueF() ) ) || ( mComponent == QgsColorWidget::Hue && color.hsvHueF() >= 0 && !qgsDoubleNear( hueF(), color.hsvHueF() ) ) || ( mComponent == QgsColorWidget::Saturation && !qgsDoubleNear( mCurrentColor.hsvSaturationF(), color.hsvSaturationF() ) ) || ( mComponent == QgsColorWidget::Value && !qgsDoubleNear( mCurrentColor.valueF(), color.valueF() ) ) || ( mComponent == QgsColorWidget::Cyan && !qgsDoubleNear( mCurrentColor.cyanF(), color.cyanF() ) ) || ( mComponent == QgsColorWidget::Magenta && !qgsDoubleNear( mCurrentColor.magentaF(), color.magentaF() ) ) || ( mComponent == QgsColorWidget::Yellow && !qgsDoubleNear( mCurrentColor.yellowF(), color.yellowF() ) ) || ( mComponent == QgsColorWidget::Black && !qgsDoubleNear( mCurrentColor.blackF(), color.blackF() ) ) );

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
  if ( mIsDragging )
  {
    setColorFromPoint( event->pos() );
  }
  QgsColorWidget::mouseMoveEvent( event );
}

void QgsColorBox::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    mIsDragging = true;
    setColorFromPoint( event->pos() );
  }
  else
  {
    QgsColorWidget::mousePressEvent( event );
  }
}

void QgsColorBox::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    mIsDragging = false;
  }
  else
  {
    QgsColorWidget::mouseReleaseEvent( event );
  }
}

void QgsColorBox::createBox()
{
  const int maxValueX = mBoxImage->width();
  const int maxValueY = mBoxImage->height();

  //create a temporary color object
  QColor currentColor = QColor( mCurrentColor );
  float colorComponentValue;

  for ( int y = 0; y < maxValueY; ++y )
  {
    QRgb *scanLine = ( QRgb * ) mBoxImage->scanLine( y );

    colorComponentValue = 1.f - static_cast<float>( y ) / static_cast<float>( maxValueY );
    alterColorF( currentColor, yComponent(), colorComponentValue );
    for ( int x = 0; x < maxValueX; ++x )
    {
      colorComponentValue = static_cast<float>( x ) / static_cast<float>( maxValueY );
      alterColorF( currentColor, xComponent(), colorComponentValue );
      scanLine[x] = currentColor.rgb();
    }
  }
  mDirty = false;
}

float QgsColorBox::valueRangeX() const
{
  return static_cast<float>( componentRange( xComponent() ) );
}

float QgsColorBox::valueRangeY() const
{
  return static_cast<float>( componentRange( yComponent() ) );
}

QgsColorWidget::ColorComponent QgsColorBox::yComponent() const
{
  switch ( mComponent )
  {
    case QgsColorWidget::Red:
      return QgsColorWidget::Green;
    case QgsColorWidget::Green:
    case QgsColorWidget::Blue:
      return QgsColorWidget::Red;

    case QgsColorWidget::Hue:
      return QgsColorWidget::Saturation;
    case QgsColorWidget::Saturation:
    case QgsColorWidget::Value:
      return QgsColorWidget::Hue;

    case QgsColorWidget::Magenta:
      return QgsColorWidget::Yellow;
    case QgsColorWidget::Yellow:
    case QgsColorWidget::Cyan:
      return QgsColorWidget::Magenta;

    default:
      //should not occur
      return QgsColorWidget::Red;
  }
}

float QgsColorBox::yComponentValue() const
{
  return componentValueF( yComponent() );
}

QgsColorWidget::ColorComponent QgsColorBox::xComponent() const
{
  switch ( mComponent )
  {
    case QgsColorWidget::Red:
    case QgsColorWidget::Green:
      return QgsColorWidget::Blue;
    case QgsColorWidget::Blue:
      return QgsColorWidget::Green;

    case QgsColorWidget::Hue:
    case QgsColorWidget::Saturation:
      return QgsColorWidget::Value;
    case QgsColorWidget::Value:
      return QgsColorWidget::Saturation;

    case QgsColorWidget::Magenta:
    case QgsColorWidget::Yellow:
      return QgsColorWidget::Cyan;
    case QgsColorWidget::Cyan:
      return QgsColorWidget::Yellow;

    default:
      //should not occur
      return QgsColorWidget::Red;
  }
}

float QgsColorBox::xComponentValue() const
{
  return componentValueF( xComponent() );
}

void QgsColorBox::setColorFromPoint( QPoint point )
{
  const float x = static_cast<float>( point.x() );
  const float y = static_cast<float>( point.y() );
  const float w = static_cast<float>( width() );
  const float h = static_cast<float>( height() );

  float valX = ( x - mMargin ) / ( w - 2 * mMargin - 1 );
  float valY = 1.f - ( y - mMargin ) / ( h - 2 * mMargin - 1 );

  QColor color = QColor( mCurrentColor );
  alterColorF( color, xComponent(), valX );
  alterColorF( color, yComponent(), valY );

  if ( color == mCurrentColor )
  {
    return;
  }

  if ( color.hueF() >= 0 )
  {
    mExplicitHue = color.hueF(); // NOLINT(bugprone-narrowing-conversions): TODO QGIS 4 remove the nolint instructions, QColor was qreal (double) and is now float
  }

  mCurrentColor = color;
  update();
  emit colorChanged( color );
}


//
// QgsColorRampWidget
//

QgsColorRampWidget::QgsColorRampWidget( QWidget *parent, const QgsColorWidget::ColorComponent component, const Orientation orientation )
  : QgsColorWidget( parent, component )
{
  setFocusPolicy( Qt::StrongFocus );
  setOrientation( orientation );

  //create triangle polygons
  setMarkerSize( 5 );
}

QSize QgsColorRampWidget::sizeHint() const
{
  if ( mOrientation == QgsColorRampWidget::Horizontal )
  {
    //horizontal
    return QSize( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 22, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 1.3 );
  }
  else
  {
    //vertical
    return QSize( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 1.3, Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 22 );
  }
}

void QgsColorRampWidget::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event )
  QPainter painter( this );

  if ( mShowFrame )
  {
    //draw frame
    QStyleOptionFrame option;
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

  float w = static_cast<float>( width() );
  float h = static_cast<float>( height() );
  float margin = static_cast<float>( mMargin );
  if ( mComponent != QgsColorWidget::Alpha )
  {
    const int maxValue = ( mOrientation == QgsColorRampWidget::Horizontal ? width() : height() ) - 1 - 2 * mMargin;
    QColor color = QColor( mCurrentColor );
    color.setAlphaF( 1.f );
    QPen pen;
    // we need to set pen width to 1,
    // since on retina displays
    // pen.setWidth(0) <=> pen.width = 0.5
    // see https://github.com/qgis/QGIS/issues/23900
    pen.setWidth( 1 );
    painter.setPen( pen );
    painter.setBrush( Qt::NoBrush );

    //draw background ramp
    for ( int c = 0; c <= maxValue; ++c )
    {
      float colorVal = static_cast<float>( c ) / static_cast<float>( maxValue );
      //vertical sliders are reversed
      if ( mOrientation == QgsColorRampWidget::Vertical )
      {
        colorVal = 1.f - colorVal;
      }
      alterColorF( color, mComponent, colorVal );
      if ( color.hueF() < 0 )
      {
        color.setHsvF( hueF(), color.saturationF(), color.valueF() );
      }
      pen.setColor( color );
      painter.setPen( pen );
      if ( mOrientation == QgsColorRampWidget::Horizontal )
      {
        //horizontal
        painter.drawLine( QLineF( c + mMargin, mMargin, c + mMargin, height() - mMargin - 1 ) );
      }
      else
      {
        //vertical
        painter.drawLine( QLineF( mMargin, c + mMargin, width() - mMargin - 1, c + mMargin ) );
      }
    }
  }
  else
  {
    //alpha ramps are drawn differently
    //start with the checkboard pattern
    const QBrush checkBrush = QBrush( transparentBackground() );
    painter.setBrush( checkBrush );
    painter.setPen( Qt::NoPen );
    painter.drawRect( QRectF( margin, margin, w - 2 * margin - 1, h - 2 * margin - 1 ) );
    QLinearGradient colorGrad;
    if ( mOrientation == QgsColorRampWidget::Horizontal )
    {
      //horizontal
      colorGrad = QLinearGradient( margin, 0, w - margin - 1, 0 );
    }
    else
    {
      //vertical
      colorGrad = QLinearGradient( 0, margin, 0, h - margin - 1 );
    }
    QColor transparent = QColor( mCurrentColor );
    transparent.setAlpha( 0 );
    colorGrad.setColorAt( 0, transparent );
    QColor opaque = QColor( mCurrentColor );
    opaque.setAlpha( 255 );
    colorGrad.setColorAt( 1, opaque );
    const QBrush colorBrush = QBrush( colorGrad );
    painter.setBrush( colorBrush );
    painter.drawRect( QRectF( margin, margin, w - 2 * margin - 1, h - 2 * margin - 1 ) );
  }

  if ( mOrientation == QgsColorRampWidget::Horizontal )
  {
    //draw marker triangles for horizontal ramps
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setBrush( QBrush( Qt::black ) );
    painter.setPen( Qt::NoPen );
    painter.translate( margin + ( w - 2 * margin ) * componentValueF(), margin - 1 );
    painter.drawPolygon( mTopTriangle );
    painter.translate( 0, h - margin - 2 );
    painter.setBrush( QBrush( Qt::white ) );
    painter.drawPolygon( mBottomTriangle );
    painter.end();
  }
  else
  {
    //draw cross lines for vertical ramps
    const double ypos = margin + ( h - 2 * margin - 1 ) - ( h - 2 * margin - 1 ) * componentValueF();
    painter.setBrush( Qt::white );
    painter.setPen( Qt::NoPen );
    painter.drawRect( QRectF( margin, ypos - 1, w - 2 * margin - 1, 3 ) );
    painter.setPen( Qt::black );
    painter.drawLine( QLineF( margin, ypos, w - margin - 1, ypos ) );
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
  mTopTriangle << QPoint( -markerSize, 0 ) << QPoint( markerSize, 0 ) << QPoint( 0, markerSize );
  mBottomTriangle << QPoint( -markerSize, 0 ) << QPoint( markerSize, 0 ) << QPoint( 0, -markerSize );
  update();
}

void QgsColorRampWidget::mouseMoveEvent( QMouseEvent *event )
{
  if ( mIsDragging )
  {
    setColorFromPoint( event->pos() );
  }

  QgsColorWidget::mouseMoveEvent( event );
}

void QgsColorRampWidget::wheelEvent( QWheelEvent *event )
{
  const float oldValue = componentValueF();
  const float delta = 1.f / static_cast<float>( componentRange() );
  if ( event->angleDelta().y() > 0 )
  {
    setComponentValueF( oldValue + delta );
  }
  else
  {
    setComponentValueF( oldValue - delta );
  }

  if ( !qgsDoubleNear( componentValueF(), oldValue ) )
  {
    //value has changed
    emit colorChanged( mCurrentColor );
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( componentValue() );
    Q_NOWARN_DEPRECATED_POP
    emit valueChangedF( componentValueF() );
  }

  event->accept();
}

void QgsColorRampWidget::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    mIsDragging = true;
    setColorFromPoint( event->pos() );
  }
  else
  {
    QgsColorWidget::mousePressEvent( event );
  }
}

void QgsColorRampWidget::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    mIsDragging = false;
  }
  else
  {
    QgsColorWidget::mousePressEvent( event );
  }
}

void QgsColorRampWidget::keyPressEvent( QKeyEvent *event )
{
  const float oldValue = componentValueF();
  const float delta = 1.f / static_cast<float>( componentRange() );
  if ( ( mOrientation == QgsColorRampWidget::Horizontal && ( event->key() == Qt::Key_Right || event->key() == Qt::Key_Up ) )
       || ( mOrientation == QgsColorRampWidget::Vertical && ( event->key() == Qt::Key_Left || event->key() == Qt::Key_Up ) ) )
  {
    setComponentValueF( oldValue + delta );
  }
  else if ( ( mOrientation == QgsColorRampWidget::Horizontal && ( event->key() == Qt::Key_Left || event->key() == Qt::Key_Down ) )
            || ( mOrientation == QgsColorRampWidget::Vertical && ( event->key() == Qt::Key_Right || event->key() == Qt::Key_Down ) ) )
  {
    setComponentValueF( oldValue - delta );
  }
  else if ( ( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_PageDown )
            || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_PageUp ) )
  {
    setComponentValueF( oldValue + 10 * delta );
  }
  else if ( ( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_PageUp )
            || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_PageDown ) )
  {
    setComponentValueF( oldValue - 10 * delta );
  }
  else if ( ( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_Home )
            || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_End ) )
  {
    setComponentValueF( 0 );
  }
  else if ( ( mOrientation == QgsColorRampWidget::Horizontal && event->key() == Qt::Key_End )
            || ( mOrientation == QgsColorRampWidget::Vertical && event->key() == Qt::Key_Home ) )
  {
    //set to maximum value
    setComponentValueF( 1.f );
  }
  else
  {
    QgsColorWidget::keyPressEvent( event );
    return;
  }

  if ( !qgsDoubleNear( componentValueF(), oldValue ) )
  {
    //value has changed
    emit colorChanged( mCurrentColor );
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( componentValue() );
    Q_NOWARN_DEPRECATED_POP
    emit valueChangedF( componentValueF() );
  }
}

void QgsColorRampWidget::setColorFromPoint( QPointF point )
{
  const float oldValue = componentValueF();
  float val;
  const float margin = static_cast<float>( mMargin );
  const float w = static_cast<float>( width() );
  const float h = static_cast<float>( height() );

  if ( mOrientation == QgsColorRampWidget::Horizontal )
  {
    val = ( static_cast<float>( point.x() ) - margin ) / ( w - 2 * margin );
  }
  else
  {
    val = 1.f - ( static_cast<float>( point.y() ) - margin ) / ( h - 2 * margin );
  }
  setComponentValueF( val );

  if ( !qgsDoubleNear( componentValueF(), oldValue ) )
  {
    //value has changed
    emit colorChanged( mCurrentColor );
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( componentValue() );
    Q_NOWARN_DEPRECATED_POP
    emit valueChangedF( componentValueF() );
  }
}

//
// QgsColorSliderWidget
//

QgsColorSliderWidget::QgsColorSliderWidget( QWidget *parent, const ColorComponent component )
  : QgsColorWidget( parent, component )

{
  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->setSpacing( 5 );

  mRampWidget = new QgsColorRampWidget( nullptr, component );
  mRampWidget->setColor( mCurrentColor );
  hLayout->addWidget( mRampWidget, 1 );

  mSpinBox = new QgsDoubleSpinBox();
  mSpinBox->setShowClearButton( false );
  //set spinbox to a reasonable width
  const int largestCharWidth = mSpinBox->fontMetrics().horizontalAdvance( QStringLiteral( "888%" ) );
  mSpinBox->setMinimumWidth( largestCharWidth + 35 );
  mSpinBox->setMinimum( 0 );
  mSpinBox->setMaximum( convertRealToDisplay( 1.f ) );
  mSpinBox->setValue( convertRealToDisplay( componentValueF() ) );
  hLayout->addWidget( mSpinBox );
  setLayout( hLayout );

  connect( mRampWidget, &QgsColorRampWidget::valueChangedF, this, &QgsColorSliderWidget::rampChanged );
  connect( mRampWidget, &QgsColorWidget::colorChanged, this, &QgsColorSliderWidget::rampColorChanged );
  connect( mSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsColorSliderWidget::spinChanged );
}

void QgsColorSliderWidget::setComponent( const QgsColorWidget::ColorComponent component )
{
  QgsColorWidget::setComponent( component );
  mRampWidget->setComponent( component );
  mSpinBox->setMaximum( convertRealToDisplay( static_cast<float>( componentRange() ) ) );

  switch ( componentUnit( component ) )
  {
    case ComponentUnit::Degree:
      mSpinBox->setSuffix( QChar( 176 ) );
      break;

    case ComponentUnit::Percent:
      mSpinBox->setSuffix( tr( "%" ) );
      break;

    case ComponentUnit::Scaled0to255:
      //clear suffix
      mSpinBox->setSuffix( QString() );
  }
}

void QgsColorSliderWidget::setComponentValueF( const float value )
{
  QgsColorWidget::setComponentValueF( value );
  mRampWidget->blockSignals( true );
  mRampWidget->setComponentValueF( value );
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
  mSpinBox->setValue( convertRealToDisplay( componentValueF() ) );
  mSpinBox->blockSignals( false );
}

void QgsColorSliderWidget::rampColorChanged( const QColor &color )
{
  emit colorChanged( color );
}

void QgsColorSliderWidget::spinChanged( double value )
{
  const float convertedValue = convertDisplayToReal( static_cast<float>( value ) );
  QgsColorWidget::setComponentValueF( convertedValue );
  mRampWidget->setComponentValueF( convertedValue );
  emit colorChanged( mCurrentColor );
}

void QgsColorSliderWidget::rampChanged( float value )
{
  mSpinBox->blockSignals( true );
  mSpinBox->setValue( convertRealToDisplay( value ) );
  mSpinBox->blockSignals( false );
}


float QgsColorSliderWidget::convertRealToDisplay( const float realValue ) const
{
  switch ( componentUnit( mComponent ) )
  {
    case ComponentUnit::Percent:
      return realValue * 100.f;

    case ComponentUnit::Degree:
      return realValue * HUE_MAX;

    case ComponentUnit::Scaled0to255:
      return realValue * 255.f;
  }

  BUILTIN_UNREACHABLE
}

float QgsColorSliderWidget::convertDisplayToReal( const float displayValue ) const
{
  switch ( componentUnit( mComponent ) )
  {
    case ComponentUnit::Percent:
      return displayValue / 100.f;

    case ComponentUnit::Degree:
      return displayValue / HUE_MAX;

    case ComponentUnit::Scaled0to255:
      return displayValue / 255.f;
  }

  BUILTIN_UNREACHABLE
}

//
// QgsColorTextWidget
//

QgsColorTextWidget::QgsColorTextWidget( QWidget *parent )
  : QgsColorWidget( parent )
{
  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->setSpacing( 0 );

  mLineEdit = new QLineEdit( nullptr );
  hLayout->addWidget( mLineEdit );

  mMenuButton = new QToolButton( mLineEdit );
  mMenuButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconDropDownMenu.svg" ) ) );
  mMenuButton->setCursor( Qt::ArrowCursor );
  mMenuButton->setFocusPolicy( Qt::NoFocus );
  mMenuButton->setStyleSheet( QStringLiteral( "QToolButton { border: none; padding: 0px; }" ) );

  setLayout( hLayout );

  const int frameWidth = mLineEdit->style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  mLineEdit->setStyleSheet( QStringLiteral( "QLineEdit { padding-right: %1px; } " )
                              .arg( mMenuButton->sizeHint().width() + frameWidth + 1 ) );

  connect( mLineEdit, &QLineEdit::editingFinished, this, &QgsColorTextWidget::textChanged );
  connect( mMenuButton, &QAbstractButton::clicked, this, &QgsColorTextWidget::showMenu );

  //restore format setting
  QgsSettings settings;
  mFormat = settings.enumValue( QStringLiteral( "ColorWidgets/textWidgetFormat" ), HexRgb );

  updateText();
}

void QgsColorTextWidget::setColor( const QColor &color, const bool emitSignals )
{
  QgsColorWidget::setColor( color, emitSignals );
  updateText();
}

void QgsColorTextWidget::resizeEvent( QResizeEvent *event )
{
  Q_UNUSED( event )
  const QSize sz = mMenuButton->sizeHint();
  const int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  mMenuButton->move( mLineEdit->rect().right() - frameWidth - sz.width(), ( mLineEdit->rect().bottom() + 1 - sz.height() ) / 2 );
}

void QgsColorTextWidget::updateText()
{
  switch ( mFormat )
  {
    case HexRgb:
      mLineEdit->setText( mCurrentColor.name() );
      break;
    case HexRgbA:
      mLineEdit->setText( mCurrentColor.name() + QStringLiteral( "%1" ).arg( mCurrentColor.alpha(), 2, 16, QChar( '0' ) ) );
      break;
    case Rgb:
      mLineEdit->setText( tr( "rgb( %1, %2, %3 )" ).arg( mCurrentColor.red() ).arg( mCurrentColor.green() ).arg( mCurrentColor.blue() ) );
      break;
    case Rgba:
      mLineEdit->setText( tr( "rgba( %1, %2, %3, %4 )" ).arg( mCurrentColor.red() ).arg( mCurrentColor.green() ).arg( mCurrentColor.blue() ).arg( QString::number( mCurrentColor.alphaF(), 'f', 2 ) ) );
      break;
  }
}

void QgsColorTextWidget::textChanged()
{
  const QString testString = mLineEdit->text();
  bool containsAlpha;
  QColor color = QgsSymbolLayerUtils::parseColorWithAlpha( testString, containsAlpha );
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
  QAction *hexRgbaAction = nullptr;
  QAction *rgbaAction = nullptr;

  QAction *hexRgbAction = new QAction( tr( "#RRGGBB" ), nullptr );
  colorContextMenu.addAction( hexRgbAction );
  if ( mAllowAlpha )
  {
    hexRgbaAction = new QAction( tr( "#RRGGBBAA" ), nullptr );
    colorContextMenu.addAction( hexRgbaAction );
  }
  QAction *rgbAction = new QAction( tr( "rgb( r, g, b )" ), nullptr );
  colorContextMenu.addAction( rgbAction );
  if ( mAllowAlpha )
  {
    rgbaAction = new QAction( tr( "rgba( r, g, b, a )" ), nullptr );
    colorContextMenu.addAction( rgbaAction );
  }

  QAction *selectedAction = colorContextMenu.exec( QCursor::pos() );
  if ( selectedAction == hexRgbAction )
  {
    mFormat = QgsColorTextWidget::HexRgb;
  }
  else if ( hexRgbaAction && selectedAction == hexRgbaAction )
  {
    mFormat = QgsColorTextWidget::HexRgbA;
  }
  else if ( selectedAction == rgbAction )
  {
    mFormat = QgsColorTextWidget::Rgb;
  }
  else if ( rgbaAction && selectedAction == rgbaAction )
  {
    mFormat = QgsColorTextWidget::Rgba;
  }

  //save format setting
  QgsSettings settings;
  settings.setEnumValue( QStringLiteral( "ColorWidgets/textWidgetFormat" ), mFormat );

  updateText();
}

void QgsColorTextWidget::setAllowOpacity( const bool allowOpacity )
{
  mAllowAlpha = allowOpacity;
}

//
// QgsColorPreviewWidget
//

QgsColorPreviewWidget::QgsColorPreviewWidget( QWidget *parent )
  : QgsColorWidget( parent )
  , mColor2( QColor() )
{
}

void QgsColorPreviewWidget::drawColor( const QColor &color, QRect rect, QPainter &painter )
{
  painter.setPen( Qt::NoPen );
  //if color has an alpha, start with a checkboard pattern
  if ( color.alpha() < 255 )
  {
    const QBrush checkBrush = QBrush( transparentBackground() );
    painter.setBrush( checkBrush );
    painter.drawRect( rect );

    //draw half of widget showing solid color, the other half showing color with alpha

    //ensure at least a 1px overlap to avoid artifacts
    const QBrush colorBrush = QBrush( color );
    painter.setBrush( colorBrush );
    painter.drawRect( std::floor( rect.width() / 2.0 ) + rect.left(), rect.top(), rect.width() - std::floor( rect.width() / 2.0 ), rect.height() );

    QColor opaqueColor = QColor( color );
    opaqueColor.setAlpha( 255 );
    const QBrush opaqueBrush = QBrush( opaqueColor );
    painter.setBrush( opaqueBrush );
    painter.drawRect( rect.left(), rect.top(), std::ceil( rect.width() / 2.0 ), rect.height() );
  }
  else
  {
    //no alpha component, just draw a solid rectangle
    const QBrush brush = QBrush( color );
    painter.setBrush( brush );
    painter.drawRect( rect );
  }
}

void QgsColorPreviewWidget::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event )
  QPainter painter( this );

  if ( mColor2.isValid() )
  {
    //drawing with two color sections
    const int verticalSplit = std::round( height() / 2.0 );
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
  return QSize( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 22, Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 22 * 0.75 );
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
  if ( ( e->pos() - mDragStartPosition ).manhattanLength() >= QApplication::startDragDistance() )
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
    const int verticalSplit = std::round( height() / 2.0 );
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

  if ( ( e->pos() - mDragStartPosition ).manhattanLength() < QApplication::startDragDistance() )
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
    const int verticalSplit = std::round( height() / 2.0 );
    if ( mDragStartPosition.y() >= verticalSplit )
    {
      dragColor = mColor2;
    }
  }

  QDrag *drag = new QDrag( this );
  drag->setMimeData( QgsSymbolLayerUtils::colorToMimeData( dragColor ) );
  drag->setPixmap( createDragIcon( dragColor ) );
  drag->exec( Qt::CopyAction );
}


//
// QgsColorWidgetAction
//

QgsColorWidgetAction::QgsColorWidgetAction( QgsColorWidget *colorWidget, QMenu *menu, QWidget *parent )
  : QWidgetAction( parent )
  , mMenu( menu )
  , mColorWidget( colorWidget )
  , mSuppressRecurse( false )
  , mDismissOnColorSelection( true )
{
  setDefaultWidget( mColorWidget );
  connect( mColorWidget, &QgsColorWidget::colorChanged, this, &QgsColorWidgetAction::setColor );

  connect( this, &QAction::hovered, this, &QgsColorWidgetAction::onHover );
  connect( mColorWidget, &QgsColorWidget::hovered, this, &QgsColorWidgetAction::onHover );
}

void QgsColorWidgetAction::onHover()
{
  //see https://bugreports.qt.io/browse/QTBUG-10427?focusedCommentId=185610&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-185610
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

void QgsColorWidgetAction::setColor( const QColor &color )
{
  emit colorChanged( color );
  if ( mMenu && mDismissOnColorSelection )
  {
    QAction::trigger();
    mMenu->hide();
  }
}
