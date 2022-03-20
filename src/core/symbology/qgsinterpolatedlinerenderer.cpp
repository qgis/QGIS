/***************************************************************************
  qgsinterpolatedlinerenderer.cpp
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "qgsinterpolatedlinerenderer.h"
#include "qgscolorramplegendnode.h"
#include "qgssymbollayerutils.h"
#include "qgsstyle.h"


void QgsInterpolatedLineRenderer::setInterpolatedWidth( const QgsInterpolatedLineWidth &strokeWidth )
{
  mStrokeWidth = strokeWidth;
}

QgsInterpolatedLineWidth QgsInterpolatedLineRenderer::interpolatedLineWidth() const
{
  return mStrokeWidth;
}

void QgsInterpolatedLineRenderer::setInterpolatedColor( const QgsInterpolatedLineColor &strokeColoring )
{
  mStrokeColoring = strokeColoring;
}

QgsInterpolatedLineColor QgsInterpolatedLineRenderer::interpolatedColor() const
{
  return mStrokeColoring;
}

void QgsInterpolatedLineRenderer::setWidthUnit( QgsUnitTypes::RenderUnit strokeWidthUnit )
{
  mStrokeWidthUnit = strokeWidthUnit;
}

QgsUnitTypes::RenderUnit QgsInterpolatedLineRenderer::widthUnit() const
{
  return mStrokeWidthUnit;
}

void QgsInterpolatedLineRenderer::renderInDeviceCoordinates( double valueColor1, double valueColor2, double valueWidth1, double valueWidth2, QPointF p1, QPointF p2, QgsRenderContext &context ) const
{
  QPainter *painter = context.painter();
  QgsScopedQPainterState painterState( painter );
  context.setPainterFlagsUsingContext( painter );

  QPointF dir = p2 - p1;
  double length = sqrt( pow( dir.x(), 2 ) + pow( dir.y(), 2 ) );
  QPointF diru = dir / length;
  QPointF orthu = QPointF( -diru.y(), diru.x() );

  QList<double> breakValues;
  QList<QColor> breakColors;
  QList<QLinearGradient> gradients;

  mStrokeColoring.graduatedColors( valueColor1, valueColor2, breakValues, breakColors, gradients );
  QColor selectedColor = context.selectionColor();

  if ( gradients.isEmpty() && !breakValues.empty() && !breakColors.isEmpty() ) //exact colors to render
  {
    Q_ASSERT( breakColors.count() == breakValues.count() );
    for ( int i = 0; i < breakValues.count(); ++i )
    {
      const bool widthIsInverted { valueWidth1 > valueWidth2 };
      const double value = breakValues.at( i );
      const double width = context.convertToPainterUnits( mStrokeWidth.strokeWidth( widthIsInverted ? mStrokeWidth.maximumValue() - value : value ), mStrokeWidthUnit );
      QPen pen( mSelected ? selectedColor : breakColors.at( i ) );
      pen.setWidthF( width );
      pen.setCapStyle( Qt::PenCapStyle::RoundCap );
      painter->setPen( pen );
      const QPointF point = p1 + dir * ( value - valueColor1 ) / ( valueColor2 - valueColor1 );
      painter->drawPoint( point );
    }
  }
  else
  {
    double width1 = mStrokeWidth.strokeWidth( valueWidth1 );
    double width2 = mStrokeWidth.strokeWidth( valueWidth2 );

    if ( !std::isnan( width1 ) && !std::isnan( width2 ) ) // the two widths on extremity are not out of range and ignored
    {
      //Draw line cap
      QBrush brush( Qt::SolidPattern );
      QPen pen;
      int startAngle;
      startAngle = ( acos( -orthu.x() ) / M_PI ) * 180;
      if ( orthu.y() < 0 )
        startAngle = 360 - startAngle;

      bool outOfRange1 = std::isnan( width1 );
      bool outOfRange2 = std::isnan( width2 );

      if ( !outOfRange1 )
      {
        width1 = context.convertToPainterUnits( width1, mStrokeWidthUnit );
        QRectF capBox1( p1.x() - width1 / 2, p1.y() - width1 / 2, width1, width1 );
        brush.setColor( mSelected ? selectedColor : mStrokeColoring.color( valueColor1 ) );
        painter->setBrush( brush );
        pen.setBrush( brush );
        painter->setPen( pen );
        painter->drawPie( capBox1, ( startAngle - 1 ) * 16, 182 * 16 );
      }

      if ( !outOfRange2 )
      {
        width2 = context.convertToPainterUnits( width2, mStrokeWidthUnit ) ;
        QRectF capBox2( p2.x() - width2 / 2, p2.y() - width2 / 2, width2, width2 );
        brush.setColor( mSelected ? selectedColor : mStrokeColoring.color( valueColor2 ) );
        pen.setBrush( brush );
        painter->setBrush( brush );
        painter->setPen( pen );
        painter->drawPie( capBox2, ( startAngle + 179 ) * 16, 182 * 16 );
      }

      if ( ( gradients.isEmpty() && breakValues.empty() && breakColors.count() == 1 ) || mSelected ) //only one color to render
      {
        double startAdjusting = 0;
        if ( outOfRange1 )
          adjustLine( valueColor1, valueColor1, valueColor2, width1, startAdjusting );


        double endAdjusting = 0;
        if ( outOfRange2 )
          adjustLine( valueColor2, valueColor1, valueColor2, width2, endAdjusting );

        QPointF pointStartAdjusted = p1 + dir * startAdjusting;
        QPointF pointEndAdjusted  = p2 - dir * endAdjusting;

        QPolygonF varLine;
        double semiWidth1 = width1 / 2;
        double semiWidth2 = width2 / 2;

        varLine.append( pointStartAdjusted + orthu * semiWidth1 );
        varLine.append( pointEndAdjusted + orthu * semiWidth2 );
        varLine.append( pointEndAdjusted - orthu * semiWidth2 );
        varLine.append( pointStartAdjusted - orthu * semiWidth1 );

        QBrush brush( Qt::SolidPattern );
        brush.setColor( mSelected ? selectedColor : breakColors.first() );
        painter->setBrush( brush );
        painter->setPen( pen );

        QPen pen;
        pen.setBrush( brush );
        pen.setWidthF( 0 );
        painter->setPen( pen );

        painter->drawPolygon( varLine );

      }
      else if ( !gradients.isEmpty() && !breakValues.isEmpty() && !breakColors.isEmpty() )
      {
        Q_ASSERT( breakColors.count() == breakValues.count() );
        Q_ASSERT( breakColors.count() == gradients.count() + 1 );
        double widthColorVariationValueRatio = ( valueWidth2 - valueWidth1 ) / ( valueColor2 - valueColor1 );

        for ( int i = 0; i < gradients.count(); ++i )
        {
          double firstValue = breakValues.at( i );
          double secondValue = breakValues.at( i + 1 );
          double w1 =  mStrokeWidth.strokeWidth( widthColorVariationValueRatio * ( firstValue - valueColor1 ) + valueWidth1 );
          double w2 =  mStrokeWidth.strokeWidth( widthColorVariationValueRatio * ( secondValue - valueColor1 ) + valueWidth1 );

          if ( std::isnan( w1 ) && std::isnan( w2 ) )
            continue;

          double firstAdjusting = 0;
          if ( std::isnan( w1 ) )
            adjustLine( firstValue, valueColor1, valueColor2, w1, firstAdjusting );


          double secondAdjusting = 0;
          if ( std::isnan( w2 ) )
            adjustLine( secondValue, valueColor1, valueColor2, w2, secondAdjusting );

          w1 = context.convertToPainterUnits( w1, mStrokeWidthUnit );
          w2 = context.convertToPainterUnits( w2, mStrokeWidthUnit ) ;

          QPointF pointStart = p1 + dir * ( firstValue - valueColor1 ) / ( valueColor2 - valueColor1 );
          QPointF pointEnd = p1 + dir * ( secondValue - valueColor1 ) / ( valueColor2 - valueColor1 );

          QPointF pointStartAdjusted = pointStart + dir * firstAdjusting;
          QPointF pointEndAdjusted  = pointEnd - dir * secondAdjusting;

          QPolygonF varLine;
          double sw1 = w1 / 2;
          double sw2 = w2 / 2;

          varLine.append( pointStartAdjusted + orthu * sw1 );
          varLine.append( pointEndAdjusted + orthu * sw2 );
          varLine.append( pointEndAdjusted - orthu * sw2 );
          varLine.append( pointStartAdjusted - orthu * sw1 );

          QLinearGradient gradient = gradients.at( i );
          gradient.setStart( pointStart );
          gradient.setFinalStop( pointEnd );
          QBrush brush( gradient );
          painter->setBrush( brush );

          QPen pen;
          pen.setBrush( brush );
          pen.setWidthF( 0 );
          painter->setPen( pen );

          painter->drawPolygon( varLine );
        }
      }
    }
  }
}


void QgsInterpolatedLineRenderer::render( double value1, double value2, const QgsPointXY &pt1, const QgsPointXY &pt2, QgsRenderContext &context ) const
{
  const QgsMapToPixel &mapToPixel = context.mapToPixel();

  QgsPointXY point1 = pt1;
  QgsPointXY point2 = pt2;

  if ( value1 > value2 )
  {
    std::swap( value1, value2 );
    std::swap( point1, point2 );
  }

  QPointF p1 = mapToPixel.transform( point1 ).toQPointF();
  QPointF p2 = mapToPixel.transform( point2 ).toQPointF();

  renderInDeviceCoordinates( value1, value2, value1, value2, p1, p2, context );
}

void QgsInterpolatedLineRenderer::render( double valueColor1, double valueColor2, double valueWidth1, double valueWidth2, const QgsPointXY &pt1, const QgsPointXY &pt2, QgsRenderContext &context ) const
{
  const QgsMapToPixel &mapToPixel = context.mapToPixel();

  QgsPointXY point1 = pt1;
  QgsPointXY point2 = pt2;

  if ( valueColor1 > valueColor2 )
  {
    std::swap( valueColor1, valueColor2 );
    std::swap( valueWidth1, valueWidth2 );
    std::swap( point1, point2 );
  }

  QPointF p1 = mapToPixel.transform( point1 ).toQPointF();
  QPointF p2 = mapToPixel.transform( point2 ).toQPointF();

  renderInDeviceCoordinates( valueColor1, valueColor2, valueWidth1, valueWidth2, p1, p2, context );
}

void QgsInterpolatedLineRenderer::setSelected( bool selected )
{
  mSelected = selected;
}

void QgsInterpolatedLineRenderer::adjustLine( const double value, const double value1, const double value2, double &width, double &adjusting ) const
{
  if ( value > mStrokeWidth.maximumValue() )
  {
    adjusting = fabs( ( value - mStrokeWidth.maximumValue() ) / ( value2 - value1 ) );
    width = mStrokeWidth.maximumWidth();
  }
  else
  {
    adjusting = fabs( ( value - mStrokeWidth.minimumValue() ) / ( value2 - value1 ) );
    width = mStrokeWidth.minimumWidth();
  }
}

double QgsInterpolatedLineWidth::minimumValue() const
{
  return mMinimumValue;
}

void QgsInterpolatedLineWidth::setMinimumValue( double minimumValue )
{
  mMinimumValue = minimumValue;
  mNeedUpdateFormula = true;
}

double QgsInterpolatedLineWidth::maximumValue() const
{
  return mMaximumValue;
}

void QgsInterpolatedLineWidth::setMaximumValue( double maximumValue )
{
  mMaximumValue = maximumValue;
  mNeedUpdateFormula = true;
}

double QgsInterpolatedLineWidth::minimumWidth() const
{
  return mMinimumWidth;
}

void QgsInterpolatedLineWidth::setMinimumWidth( double minimumWidth )
{
  mMinimumWidth = minimumWidth;
  mNeedUpdateFormula = true;
}

double QgsInterpolatedLineWidth::maximumWidth() const
{
  return mMaximumWidth;
}

void QgsInterpolatedLineWidth::setMaximumWidth( double maximumWidth )
{
  mMaximumWidth = maximumWidth;
  mNeedUpdateFormula = true;
}

double QgsInterpolatedLineWidth::strokeWidth( double value ) const
{
  if ( mIsWidthVariable )
  {
    if ( mNeedUpdateFormula )
      updateLinearFormula();

    if ( mUseAbsoluteValue )
      value = std::fabs( value );

    if ( value > mMaximumValue )
    {
      if ( mIgnoreOutOfRange )
        return std::numeric_limits<double>::quiet_NaN();
      else
        return mMaximumWidth;
    }

    if ( value < mMinimumValue )
    {
      if ( mIgnoreOutOfRange )
        return std::numeric_limits<double>::quiet_NaN();
      else
        return mMinimumWidth;
    }

    return ( value - mMinimumValue ) * mLinearCoef + mMinimumWidth;
  }
  else
    return fixedStrokeWidth();
}

QDomElement QgsInterpolatedLineWidth::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "mesh-stroke-width" ) );

  elem.setAttribute( QStringLiteral( "width-varying" ), mIsWidthVariable ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "fixed-width" ), mFixedWidth );
  elem.setAttribute( QStringLiteral( "minimum-value" ), mMinimumValue );
  elem.setAttribute( QStringLiteral( "maximum-value" ), mMaximumValue );
  elem.setAttribute( QStringLiteral( "minimum-width" ), mMinimumWidth );
  elem.setAttribute( QStringLiteral( "maximum-width" ), mMaximumWidth );
  elem.setAttribute( QStringLiteral( "ignore-out-of-range" ), mIgnoreOutOfRange ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "use-absolute-value" ), mUseAbsoluteValue ? 1 : 0 );

  return elem;
}

void QgsInterpolatedLineWidth::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mIsWidthVariable = elem.attribute( QStringLiteral( "width-varying" ) ).toInt();
  mFixedWidth = elem.attribute( QStringLiteral( "fixed-width" ) ).toDouble();
  mMinimumValue = elem.attribute( QStringLiteral( "minimum-value" ) ).toDouble();
  mMaximumValue = elem.attribute( QStringLiteral( "maximum-value" ) ).toDouble();
  mMinimumWidth = elem.attribute( QStringLiteral( "minimum-width" ) ).toDouble();
  mMaximumWidth = elem.attribute( QStringLiteral( "maximum-width" ) ).toDouble();
  mIgnoreOutOfRange = elem.attribute( QStringLiteral( "ignore-out-of-range" ) ).toInt();
  mUseAbsoluteValue = elem.attribute( QStringLiteral( "use-absolute-value" ) ).toInt();
}

bool QgsInterpolatedLineWidth::useAbsoluteValue() const
{
  return mUseAbsoluteValue;
}

void QgsInterpolatedLineWidth::setUseAbsoluteValue( bool useAbsoluteValue )
{
  mUseAbsoluteValue = useAbsoluteValue;
}

double QgsInterpolatedLineWidth::fixedStrokeWidth() const
{
  return mFixedWidth;
}

bool QgsInterpolatedLineWidth::ignoreOutOfRange() const
{
  return mIgnoreOutOfRange;
}

void QgsInterpolatedLineWidth::setIgnoreOutOfRange( bool ignoreOutOfRange )
{
  mIgnoreOutOfRange = ignoreOutOfRange;
}

bool QgsInterpolatedLineWidth::isVariableWidth() const
{
  return mIsWidthVariable;
}

void QgsInterpolatedLineWidth::setIsVariableWidth( bool isWidthVarying )
{
  mIsWidthVariable = isWidthVarying;
}

void QgsInterpolatedLineWidth::setFixedStrokeWidth( double fixedWidth )
{
  mFixedWidth = fixedWidth;
}

void QgsInterpolatedLineWidth::updateLinearFormula() const
{
  if ( !qgsDoubleNear( mMaximumWidth, mMinimumWidth ) )
    mLinearCoef = ( mMaximumWidth - mMinimumWidth ) / ( mMaximumValue - mMinimumValue ) ;
  else
    mLinearCoef = 0;
  mNeedUpdateFormula = false;
}

QgsInterpolatedLineColor::QgsInterpolatedLineColor()
{
  mColorRampShader.setMinimumValue( std::numeric_limits<double>::quiet_NaN() );
  mColorRampShader.setMaximumValue( std::numeric_limits<double>::quiet_NaN() );
}

QgsInterpolatedLineColor::QgsInterpolatedLineColor( const QgsColorRampShader &colorRampShader )
{
  setColor( colorRampShader );
}

QgsInterpolatedLineColor::QgsInterpolatedLineColor( const QColor &color )
{
  setColor( color );
  mColoringMethod = SingleColor;
  mColorRampShader.setMinimumValue( std::numeric_limits<double>::quiet_NaN() );
  mColorRampShader.setMaximumValue( std::numeric_limits<double>::quiet_NaN() );
}

void QgsInterpolatedLineColor::setColor( const QgsColorRampShader &colorRampShader )
{
  mColorRampShader = colorRampShader;
  if ( ( mColorRampShader.sourceColorRamp() ) )
    mColoringMethod = ColorRamp;
  else
    mColoringMethod = SingleColor;
}

void QgsInterpolatedLineColor::setColor( const QColor &color )
{
  mSingleColor = color;
}

QColor QgsInterpolatedLineColor::color( double magnitude ) const
{
  QgsColorRamp *lSourceColorRamp = mColorRampShader.sourceColorRamp();
  if ( mColoringMethod == ColorRamp && lSourceColorRamp )
  {
    if ( mColorRampShader.isEmpty() )
      return lSourceColorRamp->color( 0 );

    int r, g, b, a;
    if ( mColorRampShader.shade( magnitude, &r, &g, &b, &a ) )
      return QColor( r, g, b, a );
    else
      return QColor( 0, 0, 0, 0 );
  }
  else
  {
    return mSingleColor;
  }
}

QgsInterpolatedLineColor::ColoringMethod QgsInterpolatedLineColor::coloringMethod() const
{
  return mColoringMethod;
}

QgsColorRampShader QgsInterpolatedLineColor::colorRampShader() const
{
  return mColorRampShader;
}

QColor QgsInterpolatedLineColor::singleColor() const
{
  return mSingleColor;
}

QDomElement QgsInterpolatedLineColor::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "mesh-stroke-color" ) );

  elem.setAttribute( QStringLiteral( "single-color" ), QgsSymbolLayerUtils::encodeColor( mSingleColor ) );
  elem.setAttribute( QStringLiteral( "coloring-method" ), mColoringMethod );
  elem.appendChild( mColorRampShader.writeXml( doc ) );

  return elem;
}

void QgsInterpolatedLineColor::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  QDomElement shaderElem = elem.firstChildElement( QStringLiteral( "colorrampshader" ) );
  mColorRampShader.readXml( shaderElem );

  mSingleColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "single-color" ) ) );
  mColoringMethod = static_cast<QgsInterpolatedLineColor::ColoringMethod>(
                      elem.attribute( QStringLiteral( "coloring-method" ) ).toInt() );
}

void QgsInterpolatedLineColor::graduatedColors( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, QList<QLinearGradient> &gradients ) const
{
  breakValues.clear();
  breakColors.clear();
  gradients.clear();
  if ( mColoringMethod == SingleColor )
  {
    breakColors.append( mSingleColor );
    return;
  }

  switch ( mColorRampShader.colorRampType() )
  {
    case QgsColorRampShader::Interpolated:
      graduatedColorsInterpolated( value1, value2, breakValues, breakColors, gradients );
      break;
    case QgsColorRampShader::Discrete:
      graduatedColorsDiscrete( value1, value2, breakValues, breakColors, gradients );
      break;
    case QgsColorRampShader::Exact:
      graduatedColorsExact( value1, value2, breakValues, breakColors, gradients );
      break;
  }

}

void QgsInterpolatedLineColor::setColoringMethod( QgsInterpolatedLineColor::ColoringMethod coloringMethod )
{
  mColoringMethod = coloringMethod;
}

QLinearGradient QgsInterpolatedLineColor::makeSimpleLinearGradient( const QColor &color1, const QColor &color2 ) const
{
  QLinearGradient gradient;
  gradient.setColorAt( 0, color1 );
  gradient.setColorAt( 1, color2 );

  return gradient;
}

int QgsInterpolatedLineColor::itemColorIndexInf( double value ) const
{
  QList<QgsColorRampShader::ColorRampItem> itemList = mColorRampShader.colorRampItemList();

  if ( itemList.isEmpty() || itemList.first().value > value )
    return -1;

  if ( mColorRampShader.colorRampType() == QgsColorRampShader::Discrete )
    itemList.removeLast(); //remove the inf value

  if ( value > itemList.last().value )
    return itemList.count() - 1;

  int indSup = itemList.count() - 1;
  int indInf = 0;

  while ( true )
  {
    if ( abs( indSup - indInf ) <= 1 ) //always indSup>indInf, but abs to prevent infinity loop
      return indInf;

    int newInd = ( indInf + indSup ) / 2;

    if ( std::isnan( itemList.at( newInd ).value ) )
      return -1;

    if ( itemList.at( newInd ).value <= value )
      indInf = newInd;
    else
      indSup = newInd;
  }
}

void QgsInterpolatedLineColor::graduatedColorsExact( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, const QList<QLinearGradient> &gradients ) const
{
  Q_ASSERT( mColorRampShader.colorRampType() == QgsColorRampShader::Exact );
  Q_ASSERT( breakValues.isEmpty() );
  Q_ASSERT( breakColors.isEmpty() );
  Q_ASSERT( gradients.isEmpty() );

  const QList<QgsColorRampShader::ColorRampItem> &itemList = mColorRampShader.colorRampItemList();
  if ( itemList.isEmpty() )
    return;

  int index = itemColorIndexInf( value1 );
  if ( index < 0 || !qgsDoubleNear( value1, itemList.at( index ).value ) )
    index++;

  if ( qgsDoubleNear( value1, value2 ) && qgsDoubleNear( value1, itemList.at( index ).value ) )
  {
    //the two value are the same and are equal to the value in the item list --> render only one color
    breakColors.append( itemList.at( index ).color );
    return;
  }

  while ( index < itemList.count() && itemList.at( index ).value <= value2 )
  {
    breakValues.append( itemList.at( index ).value );
    breakColors.append( itemList.at( index ).color );
    index++;
  }
}

void QgsInterpolatedLineColor::graduatedColorsInterpolated( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, QList<QLinearGradient> &gradients ) const
{
  Q_ASSERT( mColorRampShader.colorRampType() == QgsColorRampShader::Interpolated );
  Q_ASSERT( breakValues.isEmpty() );
  Q_ASSERT( breakColors.isEmpty() );
  Q_ASSERT( gradients.isEmpty() );


  const QList<QgsColorRampShader::ColorRampItem> &itemList = mColorRampShader.colorRampItemList();
  if ( itemList.empty() )
    return;

  if ( itemList.count() == 1 )
  {
    breakColors.append( itemList.first().color );
    return;
  }

  if ( value2 <= itemList.first().value ) // completely out of range and less
  {
    if ( !mColorRampShader.clip() )
      breakColors.append( itemList.first().color ); // render only the first color in the whole range if not clipped
    return;
  }

  if ( value1 > itemList.last().value ) // completely out of range and greater
  {
    if ( !mColorRampShader.clip() )
      breakColors.append( itemList.last().color ); // render only the last color in the whole range if not clipped
    return;
  }

  if ( qgsDoubleNear( value1, value2 ) )
  {
    // the two values are the same
    //  --> render only one color
    int r, g, b, a;
    QColor color;
    if ( mColorRampShader.shade( value1, &r, &g, &b, &a ) )
      color = QColor( r, g, b, a );
    breakColors.append( color );
    return;
  }

  // index of the inf value of the interval where value1 is in the color ramp shader
  int index = itemColorIndexInf( value1 );
  if ( index < 0 ) // value1 out of range
  {
    QColor color = itemList.first().color;
    breakColors.append( color );
    if ( mColorRampShader.clip() ) // The first value/color returned is the first of the item list
      breakValues.append( itemList.first().value );
    else // The first value/color returned is the first color of the item list and value1
      breakValues.append( value1 );
  }
  else
  {
    // shade the color
    int r, g, b, a;
    QColor color;
    if ( mColorRampShader.shade( value1, &r, &g, &b, &a ) )
      color = QColor( r, g, b, a );
    breakValues.append( value1 );
    breakColors.append( color );
  }

  index++; // increment the index before go through the intervals

  while ( index <  itemList.count() && itemList.at( index ).value < value2 )
  {
    QColor color1 = breakColors.last();
    QColor color2 = itemList.at( index ).color;
    breakValues.append( itemList.at( index ).value );
    breakColors.append( color2 );
    gradients.append( makeSimpleLinearGradient( color1, color2 ) );
    index++;
  }

  // close the lists with value2 or last item if >value2
  QColor color1 = breakColors.last();
  QColor color2;
  if ( value2 < itemList.last().value )
  {
    int r, g, b, a;
    if ( mColorRampShader.shade( value2, &r, &g, &b, &a ) )
      color2 = QColor( r, g, b, a );
    breakValues.append( value2 );
  }
  else
  {
    color2 = itemList.last().color;
    if ( mColorRampShader.clip() )
      breakValues.append( itemList.last().value );
    else
      breakValues.append( value2 );
  }
  breakColors.append( color2 );
  gradients.append( makeSimpleLinearGradient( color1, color2 ) );
}


void QgsInterpolatedLineColor::graduatedColorsDiscrete( double value1, double value2, QList<double> &breakValues, QList<QColor> &breakColors, QList<QLinearGradient> &gradients ) const
{
  Q_ASSERT( mColorRampShader.colorRampType() == QgsColorRampShader::Discrete );
  Q_ASSERT( breakValues.isEmpty() );
  Q_ASSERT( breakColors.isEmpty() );
  Q_ASSERT( gradients.isEmpty() );

  const QList<QgsColorRampShader::ColorRampItem> &itemList = mColorRampShader.colorRampItemList();
  if ( itemList.empty() )
    return;

  if ( itemList.count() == 1 )
  {
    breakColors.append( itemList.first().color );
    return;
  }

  double lastValue = itemList.at( itemList.count() - 2 ).value;


  if ( value2 <= itemList.first().value ) // completely out of range and less
  {
    breakColors.append( itemList.first().color ); // render only the first color in the whole range
    return;
  }

  if ( value1 > lastValue ) // completely out of range and greater
  {
    breakColors.append( itemList.last().color ); // render only the last color in the whole range
    return;
  }

  // index of the inf value of the interval where value1 is in the color ramp shader
  int index = itemColorIndexInf( value1 );

  if ( qgsDoubleNear( value1, value2 ) )
  {
    // the two values are the same and are equal to the value in the item list
    //  --> render only one color, the sup one
    breakColors.append( itemList.at( index + 1 ).color );
    return;
  }

  if ( index < 0 ) // value1 out of range
  {
    breakValues.append( value1 );
    breakColors.append( itemList.first().color );
  }
  else // append the first value with corresponding color
  {
    QColor color = itemList.at( index ).color;
    breakValues.append( value1 );
    breakColors.append( color );
  }

  index++; // increment the index before go through the intervals

  while ( index < ( itemList.count() - 1 ) && itemList.at( index ).value < value2 )
  {
    QColor color = itemList.at( index ).color;
    breakValues.append( itemList.at( index ).value );
    breakColors.append( color );
    gradients.append( makeSimpleLinearGradient( color, color ) );
    index++;
  }

  // add value2 to close
  QColor lastColor = itemList.at( index ).color;
  breakColors.append( lastColor );
  breakValues.append( value2 );
  gradients.append( makeSimpleLinearGradient( lastColor, lastColor ) );

}

QString QgsInterpolatedLineSymbolLayer::layerType() const {return QStringLiteral( "InterpolatedLine" );}

void QgsInterpolatedLineSymbolLayer::startRender( QgsSymbolRenderContext & )
{
}

void QgsInterpolatedLineSymbolLayer::stopRender( QgsSymbolRenderContext & )
{
}

QgsInterpolatedLineSymbolLayer *QgsInterpolatedLineSymbolLayer::clone() const
{
  QgsInterpolatedLineSymbolLayer *l = static_cast<QgsInterpolatedLineSymbolLayer *>( create( properties() ) );
  copyDataDefinedProperties( l );
  copyPaintEffect( l );
  return l;
}

QgsSymbolLayer *QgsInterpolatedLineSymbolLayer::create( const QVariantMap &properties )
{
  std::unique_ptr<QgsInterpolatedLineSymbolLayer> symbolLayer;
  symbolLayer.reset( new QgsInterpolatedLineSymbolLayer() );

  if ( properties.contains( QStringLiteral( "start_width_expression" ) ) )
    symbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineStartWidthValue, QgsProperty::fromExpression( properties.value( QStringLiteral( "start_width_expression" ) ).toString() ) );
  if ( properties.contains( QStringLiteral( "end_width_expression" ) ) )
    symbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineEndWidthValue, QgsProperty::fromExpression( properties.value( QStringLiteral( "end_width_expression" ) ).toString() ) );

  if ( properties.contains( QStringLiteral( "start_color_expression" ) ) )
    symbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineStartColorValue, QgsProperty::fromExpression( properties.value( QStringLiteral( "start_color_expression" ) ).toString() ) );
  if ( properties.contains( QStringLiteral( "end_color_expression" ) ) )
    symbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLineEndColorValue, QgsProperty::fromExpression( properties.value( QStringLiteral( "end_color_expression" ) ).toString() ) );

  if ( properties.contains( QStringLiteral( "line_width" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setFixedStrokeWidth( properties.value( QStringLiteral( "line_width" ) ).toDouble() ) ;
  if ( properties.contains( QStringLiteral( "line_width_unit" ) ) )
    symbolLayer->mLineRender.setWidthUnit( QgsUnitTypes::decodeRenderUnit( properties.value( QStringLiteral( "line_width_unit" ) ).toString() ) );
  if ( properties.contains( QStringLiteral( "width_varying_minimum_value" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setMinimumValue( properties.value( QStringLiteral( "width_varying_minimum_value" ) ).toDouble() );
  if ( properties.contains( QStringLiteral( "width_varying_maximum_value" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setMaximumValue( properties.value( QStringLiteral( "width_varying_maximum_value" ) ).toDouble() );
  if ( properties.contains( QStringLiteral( "width_varying_use_absolute_value" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setUseAbsoluteValue( properties.value( QStringLiteral( "width_varying_use_absolute_value" ) ).toInt() );
  if ( properties.contains( QStringLiteral( "width_varying_minimum_width" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setMinimumWidth( properties.value( QStringLiteral( "width_varying_minimum_width" ) ).toDouble() );
  if ( properties.contains( QStringLiteral( "width_varying_maximum_width" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setMaximumWidth( properties.value( QStringLiteral( "width_varying_maximum_width" ) ).toDouble() );
  if ( properties.contains( QStringLiteral( "width_varying_ignore_out_of_range" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setIgnoreOutOfRange( properties.value( QStringLiteral( "width_varying_ignore_out_of_range" ) ).toInt() );
  if ( properties.contains( QStringLiteral( "width_varying_is_variable_width" ) ) )
    symbolLayer->mLineRender.mStrokeWidth.setIsVariableWidth( properties.value( QStringLiteral( "width_varying_is_variable_width" ) ).toInt() );

  if ( properties.contains( QStringLiteral( "single_color" ) ) )
    symbolLayer->mLineRender.mStrokeColoring.setColor( QgsSymbolLayerUtils::decodeColor( properties.value( QStringLiteral( "single_color" ) ).toString() ) );
  if ( properties.contains( QStringLiteral( "color_ramp_shader" ) ) )
    symbolLayer->mLineRender.mStrokeColoring.setColor( createColorRampShaderFromProperties( properties.value( QStringLiteral( "color_ramp_shader" ) ) ) );
  if ( properties.contains( QStringLiteral( "coloring_method" ) ) )
    symbolLayer->mLineRender.mStrokeColoring.setColoringMethod(
      static_cast<QgsInterpolatedLineColor::ColoringMethod>( properties.value( QStringLiteral( "coloring_method" ) ).toInt() ) );

  return symbolLayer.release();
}

Qgis::SymbolLayerFlags QgsInterpolatedLineSymbolLayer::flags() const
{
  return Qgis::SymbolLayerFlag::DisableFeatureClipping;
}

QVariantMap QgsInterpolatedLineSymbolLayer::properties() const
{
  QVariantMap props;

  // Line width varying
  props.insert( QStringLiteral( "line_width" ), QString::number( mLineRender.mStrokeWidth.fixedStrokeWidth() ) );
  props.insert( QStringLiteral( "line_width_unit" ), QgsUnitTypes::encodeUnit( mLineRender.widthUnit() ) );
  props.insert( QStringLiteral( "width_varying_minimum_value" ), mLineRender.mStrokeWidth.minimumValue() );
  props.insert( QStringLiteral( "width_varying_maximum_value" ), mLineRender.mStrokeWidth.maximumValue() );
  props.insert( QStringLiteral( "width_varying_use_absolute_value" ), mLineRender.mStrokeWidth.useAbsoluteValue() ? 1 : 0 );
  props.insert( QStringLiteral( "width_varying_minimum_width" ), mLineRender.mStrokeWidth.minimumWidth() );
  props.insert( QStringLiteral( "width_varying_maximum_width" ), mLineRender.mStrokeWidth.maximumWidth() );
  props.insert( QStringLiteral( "width_varying_ignore_out_of_range" ), mLineRender.mStrokeWidth.ignoreOutOfRange() ? 1 : 0 );
  props.insert( QStringLiteral( "width_varying_is_variable_width" ), mLineRender.mStrokeWidth.isVariableWidth() ? 1 : 0 );

  // Color varying
  props.insert( QStringLiteral( "coloring_method" ), mLineRender.mStrokeColoring.coloringMethod() );
  props.insert( QStringLiteral( "single_color" ), QgsSymbolLayerUtils::encodeColor( mLineRender.mStrokeColoring.singleColor() ) );
  props.insert( QStringLiteral( "color_ramp_shader" ), colorRampShaderProperties() );

  return props;
}

void QgsInterpolatedLineSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  QgsGeometry geometry = context.patchShape() ? context.patchShape()->geometry()
                         : QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Line, size ).geometry();

  startRender( context );
  double min = std::min( mLineRender.interpolatedLineWidth().minimumValue(), mLineRender.interpolatedColor().colorRampShader().minimumValue() );
  double max = std::max( mLineRender.interpolatedLineWidth().maximumValue(), mLineRender.interpolatedColor().colorRampShader().maximumValue() );

  double totalLength = geometry.length();
  if ( qgsDoubleNear( totalLength, 0 ) )
    return;

  double variation = ( max - min ) / totalLength;

  QPolygonF points = geometry.asQPolygonF();
  double lengthFromStart = 0;
  for ( int i = 1; i < points.count(); ++i )
  {
    QPointF p1 = points.at( i - 1 );
    QPointF p2 = points.at( i );

    double v1 = min + variation * lengthFromStart;
    QPointF vectDist = p2 - p1;
    lengthFromStart += sqrt( pow( vectDist.x(), 2 ) + pow( vectDist.y(), 2 ) );
    double v2 = min + variation * lengthFromStart;
    mLineRender.renderInDeviceCoordinates( v1, v2, v1, v2, p1, p2, context.renderContext() );
  }

  renderPolyline( points, context );

}


void QgsInterpolatedLineSymbolLayer::setExpressionsStringForWidth( const QString &start, const QString &end )
{
  if ( start.isEmpty() )
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineStartWidthValue, QgsProperty() );
  else
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineStartWidthValue, QgsProperty::fromExpression( start ) );

  if ( end.isEmpty() )
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineEndWidthValue, QgsProperty() );
  else
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineEndWidthValue, QgsProperty::fromExpression( end ) );
}

QString QgsInterpolatedLineSymbolLayer::startValueExpressionForWidth() const
{
  return mDataDefinedProperties.property( QgsSymbolLayer::PropertyLineStartWidthValue ).asExpression();
}

QString QgsInterpolatedLineSymbolLayer::endValueExpressionForWidth() const
{
  return mDataDefinedProperties.property( QgsSymbolLayer::PropertyLineEndWidthValue ).asExpression();
}

void QgsInterpolatedLineSymbolLayer::setWidthUnit( QgsUnitTypes::RenderUnit strokeWidthUnit )
{
  mLineRender.mStrokeWidthUnit = strokeWidthUnit;
}

QgsUnitTypes::RenderUnit QgsInterpolatedLineSymbolLayer::widthUnit() const {return mLineRender.widthUnit();}

void QgsInterpolatedLineSymbolLayer::setInterpolatedWidth( const QgsInterpolatedLineWidth &interpolatedLineWidth )
{
  mLineRender.mStrokeWidth = interpolatedLineWidth;
}

QgsInterpolatedLineWidth QgsInterpolatedLineSymbolLayer::interpolatedWidth() const { return mLineRender.interpolatedLineWidth();}

void QgsInterpolatedLineSymbolLayer::setExpressionsStringForColor( const QString &start, const QString &end )
{
  if ( start.isEmpty() )
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineStartColorValue, QgsProperty() );
  else
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineStartColorValue, QgsProperty::fromExpression( start ) );

  if ( end.isEmpty() )
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineEndColorValue, QgsProperty() );
  else
    mDataDefinedProperties.setProperty( QgsSymbolLayer::PropertyLineEndColorValue, QgsProperty::fromExpression( end ) );
}

QString QgsInterpolatedLineSymbolLayer::startValueExpressionForColor() const
{
  return mDataDefinedProperties.property( QgsSymbolLayer::PropertyLineStartColorValue ).asExpression();
}

QString QgsInterpolatedLineSymbolLayer::endValueExpressionForColor() const
{
  return mDataDefinedProperties.property( QgsSymbolLayer::PropertyLineEndColorValue ).asExpression();
}

void QgsInterpolatedLineSymbolLayer::setInterpolatedColor( const QgsInterpolatedLineColor &interpolatedLineColor )
{
  mLineRender.setInterpolatedColor( interpolatedLineColor );
}

QgsInterpolatedLineColor QgsInterpolatedLineSymbolLayer::interpolatedColor() const
{
  return mLineRender.interpolatedColor();
}

QVariant QgsInterpolatedLineSymbolLayer::colorRampShaderProperties() const
{
  const QgsColorRampShader &colorRampShader = mLineRender.mStrokeColoring.colorRampShader();

  QVariantMap props;
  if ( colorRampShader.sourceColorRamp() )
    props.insert( QStringLiteral( "color_ramp_source" ), QgsSymbolLayerUtils::colorRampToVariant( QString(), colorRampShader.sourceColorRamp() ) );
  props.insert( QStringLiteral( "color_ramp_shader_type" ), colorRampShader.colorRampType() );
  props.insert( QStringLiteral( "color_ramp_shader_classification_mode" ), colorRampShader.classificationMode() );
  QVariantList colorRampItemListVariant;

  const QList<QgsColorRampShader::ColorRampItem> colorRampItemList = colorRampShader.colorRampItemList();
  for ( const QgsColorRampShader::ColorRampItem &item : colorRampItemList )
  {
    QVariantMap itemVar;
    itemVar[QStringLiteral( "label" )] = item.label;
    itemVar[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( item.color );
    itemVar[QStringLiteral( "value" )] = item.value;
    colorRampItemListVariant.append( itemVar );
  }
  props.insert( QStringLiteral( "color_ramp_shader_items_list" ), colorRampItemListVariant );

  props.insert( QStringLiteral( "color_ramp_shader_minimum_value" ), colorRampShader.minimumValue() );
  props.insert( QStringLiteral( "color_ramp_shader_maximum_value" ), colorRampShader.maximumValue() );
  props.insert( QStringLiteral( "color_ramp_shader_value_out_of_range" ), colorRampShader.clip() ? 1 : 0 );
  props.insert( QStringLiteral( "color_ramp_shader_label_precision" ), colorRampShader.labelPrecision() );

  return props;
}

QgsColorRampShader QgsInterpolatedLineSymbolLayer::createColorRampShaderFromProperties( const QVariant &properties )
{
  QgsColorRampShader colorRampShader;

  if ( properties.type() != QVariant::Map )
    return colorRampShader;

  QVariantMap shaderVariantMap = properties.toMap();

  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_source" ) ) )
    colorRampShader.setSourceColorRamp( QgsSymbolLayerUtils::loadColorRamp( shaderVariantMap.value( QStringLiteral( "color_ramp_source" ) ) ) );

  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_shader_type" ) ) )
    colorRampShader.setColorRampType( static_cast<QgsColorRampShader::Type>( shaderVariantMap.value( QStringLiteral( "color_ramp_shader_type" ) ).toInt() ) );
  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_shader_classification_mode" ) ) )
    colorRampShader.setClassificationMode( static_cast<QgsColorRampShader::ClassificationMode>(
        shaderVariantMap.value( QStringLiteral( "color_ramp_shader_classification_mode" ) ).toInt() ) );

  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_shader_items_list" ) ) )
  {
    QVariant colorRampItemsVar = shaderVariantMap.value( QStringLiteral( "color_ramp_shader_items_list" ) );
    if ( colorRampItemsVar.type() == QVariant::List )
    {
      QVariantList itemVariantList = colorRampItemsVar.toList();
      QList<QgsColorRampShader::ColorRampItem> colorRampItemList;
      for ( const QVariant &itemVar : std::as_const( itemVariantList ) )
      {
        QgsColorRampShader::ColorRampItem item;
        if ( itemVar.type() != QVariant::Map )
          continue;
        QVariantMap itemVarMap = itemVar.toMap();
        if ( !itemVarMap.contains( QStringLiteral( "label" ) ) || !itemVarMap.contains( QStringLiteral( "color" ) ) || !itemVarMap.contains( QStringLiteral( "value" ) ) )
          continue;

        item.label = itemVarMap.value( QStringLiteral( "label" ) ).toString();
        item.color = QgsSymbolLayerUtils::decodeColor( itemVarMap.value( QStringLiteral( "color" ) ).toString() );
        item.value = itemVarMap.value( QStringLiteral( "value" ) ).toDouble();

        colorRampItemList.append( item );
      }
      colorRampShader.setColorRampItemList( colorRampItemList );
    }
  }

  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_shader_minimum_value" ) ) )
    colorRampShader.setMinimumValue( shaderVariantMap.value( QStringLiteral( "color_ramp_shader_minimum_value" ) ).toDouble() );
  else
    colorRampShader.setMinimumValue( std::numeric_limits<double>::quiet_NaN() );

  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_shader_maximum_value" ) ) )
    colorRampShader.setMaximumValue( shaderVariantMap.value( QStringLiteral( "color_ramp_shader_maximum_value" ) ).toDouble() );
  else
    colorRampShader.setMaximumValue( std::numeric_limits<double>::quiet_NaN() );

  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_shader_value_out_of_range" ) ) )
    colorRampShader.setClip( shaderVariantMap.value( QStringLiteral( "color_ramp_shader_value_out_of_range" ) ).toInt() == 1 );
  if ( shaderVariantMap.contains( QStringLiteral( "color_ramp_shader_label_precision" ) ) )
    colorRampShader.setLabelPrecision( shaderVariantMap.value( QStringLiteral( "color_ramp_shader_label_precision" ) ).toInt() );

  return colorRampShader;
}

QgsInterpolatedLineSymbolLayer::QgsInterpolatedLineSymbolLayer(): QgsLineSymbolLayer( true ) {}


void QgsInterpolatedLineSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = true;
  mLineParts.clear();
}

void QgsInterpolatedLineSymbolLayer::stopFeatureRender( const QgsFeature &, QgsRenderContext &context )
{
  mRenderingFeature = false;

  if ( mLineParts.empty() )
    return;

  render( mLineParts, context );
  mLineParts.clear();
}

void QgsInterpolatedLineSymbolLayer::render( const QVector< QPolygonF > &parts, QgsRenderContext &context )
{
  const double totalLength = std::accumulate( parts.begin(), parts.end(), 0.0, []( double total, const QPolygonF & part )
  {
    return total + QgsSymbolLayerUtils::polylineLength( part );
  } );

  if ( qgsDoubleNear( totalLength, 0 ) )
    return;

  double startValWidth = 0;
  double variationPerMapUnitWidth = 0;
  double startValColor = 0;
  double variationPerMapUnitColor = 0;

  bool ok = true;

  if ( mLineRender.interpolatedLineWidth().isVariableWidth() )
  {
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineStartWidthValue ) )
    {
      startValWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineStartWidthValue, context.expressionContext(), 0, &ok );
      if ( !ok )
        return;
    }

    double endValWidth = 0;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineEndWidthValue ) )
    {
      endValWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineEndWidthValue, context.expressionContext(), 0, &ok );
      if ( !ok )
        return;
    }

    variationPerMapUnitWidth = ( endValWidth - startValWidth ) / totalLength;
  }

  if ( mLineRender.interpolatedColor().coloringMethod() == QgsInterpolatedLineColor::ColorRamp )
  {
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineStartColorValue ) )
    {
      startValColor = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineStartColorValue, context.expressionContext(), 0, &ok );
      if ( !ok )
        return;
    }

    double endValColor = 0;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineEndColorValue ) )
    {
      endValColor = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineEndColorValue, context.expressionContext(), 0, &ok );
      if ( !ok )
        return;
    }

    variationPerMapUnitColor = ( endValColor - startValColor ) / totalLength;
  }

  for ( const QPolygonF &poly : parts )
  {
    double lengthFromStart = 0;
    for ( int i = 1; i < poly.count(); ++i )
    {
      const QPointF p1 = poly.at( i - 1 );
      const QPointF p2 = poly.at( i );

      const double v1c = startValColor + variationPerMapUnitColor * lengthFromStart;
      const double v1w = startValWidth + variationPerMapUnitWidth * lengthFromStart;
      lengthFromStart += std::sqrt( ( p1.x() - p2.x() ) * ( p1.x() - p2.x() ) + ( p1.y() - p2.y() ) * ( p1.y() - p2.y() ) );
      const double v2c = startValColor + variationPerMapUnitColor * lengthFromStart;
      const double v2w = startValWidth + variationPerMapUnitWidth * lengthFromStart;
      mLineRender.renderInDeviceCoordinates( v1c, v2c, v1w, v2w, p1, p2, context );
    }
  }
}

void QgsInterpolatedLineSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  mLineRender.setSelected( context.selected() );

  if ( points.empty() )
    return;

  if ( mRenderingFeature )
  {
    // in the middle of rendering a possibly multi-part feature, so we collect all the parts and defer the actual rendering
    // until after we've received the final part
    mLineParts.append( points );
  }
  else
  {
    // not rendering a feature, so we can just render the polyline immediately
    render( { points }, context.renderContext() );
  }
}

bool QgsInterpolatedLineSymbolLayer::isCompatibleWithSymbol( QgsSymbol *symbol ) const
{
  return symbol && symbol->type() == Qgis::SymbolType::Line;
}

bool QgsInterpolatedLineSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return true;
}
