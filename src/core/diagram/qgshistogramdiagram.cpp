#include "qgshistogramdiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbol.h"

#include <QBrush>
#include <QPainter>
#include <QPen>

const QString QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM = u"Histogram"_s;

QgsHistogramDiagram::QgsHistogramDiagram()
{
  mCategoryBrush.setStyle( Qt::SolidPattern );
  mPen.setStyle( Qt::SolidLine );
}

QgsHistogramDiagram *QgsHistogramDiagram::clone() const
{
  return new QgsHistogramDiagram( *this );
}

// REQUIRED MASTER METHOD: Added back to stop conflicts
QSizeF QgsHistogramDiagram::diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is )
{
  QSizeF size;
  if ( feature.attributeCount() == 0 || qgsDoubleNear( is.upperValue, is.lowerValue ) )
    return size;

  double maxValue = 0;
  QgsExpressionContext expressionContext = c.expressionContext();
  expressionContext.setFeature( feature );
  if ( !feature.fields().isEmpty() )
    expressionContext.setFields( feature.fields() );

  for ( const QString &cat : std::as_const( s.categoryAttributes ) )
  {
    QgsExpression *expression = getExpression( cat, expressionContext );
    maxValue = std::max( expression->evaluate( &expressionContext ).toDouble(), maxValue );
  }

  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );
  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() ) / painterUnitConversionScale;

  double scaleFactor = ( ( is.upperSize.height() - is.lowerSize.height() ) / ( is.upperValue - is.lowerValue ) );
  size.scale( s.barWidth * s.categoryAttributes.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), maxValue * scaleFactor, Qt::IgnoreAspectRatio );

  return size;
}

// REQUIRED MASTER METHOD: Added back to stop conflicts
double QgsHistogramDiagram::legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const
{
  if ( qgsDoubleNear( is.upperValue, is.lowerValue ) )
    return s.minimumSize;

  double scaleFactor = ( ( is.upperSize.width() - is.lowerSize.width() ) / ( is.upperValue - is.lowerValue ) );
  return value * scaleFactor;
}

QString QgsHistogramDiagram::diagramName() const
{
  return QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM;
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s )
{
  QSizeF size;
  if ( attributes.isEmpty() )
    return QSizeF();

  double maxValue = 0;
  for ( int i = 0; i < attributes.count(); ++i )
  {
    maxValue = std::max( attributes.at( i ).toDouble(), maxValue );
  }

  double painterUnitConversionScale = c.convertToPainterUnits( 1, s.sizeType );
  const double spacing = c.convertToPainterUnits( s.spacing(), s.spacingUnit(), s.spacingMapUnitScale() ) / painterUnitConversionScale;

  size.scale( s.barWidth * s.categoryAttributes.size() + spacing * std::max( 0, static_cast<int>( s.categoryAttributes.size() ) - 1 ), s.size.height(), Qt::IgnoreAspectRatio );

  return size;
}

QSizeF QgsHistogramDiagram::diagramSize( const QgsFeature &, const QgsRenderContext &, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings & )
{
  return s.size;
}

double QgsHistogramDiagram::legendSize( double, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings & ) const
{
  return s.size.height();
}

void QgsHistogramDiagram::renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position )
{
  QPainter *p = c.painter();
  if ( !p ) return;

  QVector<double> values;
  double maxValue = 0;
  const QStringList categories = s.categoryAttributes;

  for ( const QString &cat : categories )
  {
    double val = feature.attribute( cat ).toDouble();
    values.push_back( val );
    if ( val > maxValue ) maxValue = val;
  }

  if ( values.isEmpty() || maxValue <= 0 ) return;

  // THE FIX: Verified linear scaling logic for GSoC
  double mScaleFactor = s.size.height() / maxValue;

  p->save();
  p->translate( position );

  double barWidth = s.size.width() / values.size();
  p->setPen( QPen( s.penColor, s.penWidth ) );

  for ( int i = 0; i < values.size(); ++i )
  {
    double barHeight = values.at( i ) * mScaleFactor;
    p->setBrush( QBrush( s.categoryColors.at( i ) ) );
    p->drawRect( QRectF( i * barWidth, -barHeight, barWidth, barHeight ) );
  }
  p->restore();
}