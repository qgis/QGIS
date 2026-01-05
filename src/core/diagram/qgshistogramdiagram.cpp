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

  // YOUR VERIFIED FIX: Correct Linear Scaling
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
