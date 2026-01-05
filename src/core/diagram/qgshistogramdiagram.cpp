#include "qgshistogramdiagram.h"
#include "qgsdiagramrenderer.h"
#include "qgsrendercontext.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

// This definition is required to fix the Linker Error
const QString QgsHistogramDiagram::DIAGRAM_NAME_HISTOGRAM = QStringLiteral( "Histogram" );

QgsHistogramDiagram::QgsHistogramDiagram() {}
QgsHistogramDiagram *QgsHistogramDiagram::clone() const { return new QgsHistogramDiagram( *this ); }
QSizeF QgsHistogramDiagram::diagramSize( const QgsAttributes &, const QgsRenderContext &, const QgsDiagramSettings &s ) { return s.size; }
QSizeF QgsHistogramDiagram::diagramSize( const QgsFeature &, const QgsRenderContext &, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings & ) { return s.size; }
double QgsHistogramDiagram::legendSize( double, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings & ) const { return s.size.height(); }

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

  if ( s.size.height() > 0 )
  {
     // THE FIX: Correct Linear Scaling
     double mScaleFactor = s.size.height() / maxValue; 

     p->save();
     p->translate( position );

     double barWidth = s.size.width() / values.size();
     // penColor and penWidth match your specific compiler branch
     p->setPen( QPen( s.penColor, s.penWidth ) );

     for ( int i = 0; i < values.size(); ++i )
     {
       double barHeight = values.at( i ) * mScaleFactor;
       p->setBrush( QBrush( s.categoryColors.at( i ) ) );
       p->drawRect( QRectF( i * barWidth, -barHeight, barWidth, barHeight ) );
     }
     p->restore();
  }
}

QString QgsHistogramDiagram::diagramName() const
{
  return DIAGRAM_NAME_HISTOGRAM;
}
