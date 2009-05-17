
#include "qgssymbollayerv2utils.h"

#include "qgssymbollayerv2.h"
#include "qgssymbolv2.h"
#include "qgsvectorcolorrampv2.h"

#include <QColor>
#include <QIcon>
#include <QPainter>

QString QgsSymbolLayerV2Utils::encodeColor(QColor color)
{
  return QString("%1,%2,%3").arg(color.red()).arg(color.green()).arg(color.blue());
}

QColor QgsSymbolLayerV2Utils::decodeColor(QString str)
{
  QStringList lst = str.split(",");
  if (lst.count() != 3)
    return QColor();
  return QColor(lst[0].toInt(), lst[1].toInt(), lst[2].toInt());
}

QString QgsSymbolLayerV2Utils::encodePenStyle(Qt::PenStyle style)
{
  switch (style)
  {
    case Qt::SolidLine:      return "solid";
    case Qt::DashLine:       return "dash";
    case Qt::DotLine:        return "dot";
    case Qt::DashDotLine:    return "dash dot";
    case Qt::DashDotDotLine: return "dash dot dot";
    default: return "???";
  }
}

Qt::PenStyle QgsSymbolLayerV2Utils::decodePenStyle(QString str)
{
  if (str == "solid") return Qt::SolidLine;
  if (str == "dash") return Qt::DashLine;
  if (str == "dot") return Qt::DotLine;
  if (str == "dash dot") return Qt::DashDotLine;
  if (str == "dast dot dot") return Qt::DashDotDotLine;
  return Qt::SolidLine;
}

QString QgsSymbolLayerV2Utils::encodeBrushStyle(Qt::BrushStyle style)
{
  switch (style)
  {
    case Qt::SolidPattern : return "solid";
    case Qt::HorPattern : return "horizontal";
    case Qt::VerPattern : return "vertical";
    case Qt::CrossPattern : return "cross";
    case Qt::BDiagPattern : return "b_diagonal";
    case Qt::FDiagPattern : return  "f_diagonal";
    case Qt::DiagCrossPattern : return "diagonal_x";
    case Qt::Dense1Pattern  : return "dense1";
    case Qt::Dense2Pattern  : return "dense2";
    case Qt::Dense3Pattern  : return "dense3";
    case Qt::Dense4Pattern  : return "dense4";
    case Qt::Dense5Pattern  : return "dense5";
    case Qt::Dense6Pattern  : return "dense6";
    case Qt::Dense7Pattern  : return "dense7";
    case Qt::NoBrush : return "no";
    default: return "???";
  }
}

Qt::BrushStyle QgsSymbolLayerV2Utils::decodeBrushStyle(QString str)
{
  if (str == "solid") return Qt::SolidPattern;
  if (str == "horizontal") return Qt::HorPattern;
  if (str == "vertical") return Qt::VerPattern;
  if (str == "cross") return Qt::CrossPattern;
  if (str == "b_diagonal") return Qt::BDiagPattern;
  if (str == "f_diagonal") return Qt::FDiagPattern;
  if (str == "diagonal_x") return Qt::DiagCrossPattern;
  if (str == "dense1") return Qt::Dense1Pattern;
  if (str == "dense2") return Qt::Dense2Pattern;
  if (str == "dense3") return Qt::Dense3Pattern;
  if (str == "dense4") return Qt::Dense4Pattern;
  if (str == "dense5") return Qt::Dense5Pattern;
  if (str == "dense6") return Qt::Dense6Pattern;
  if (str == "dense7") return Qt::Dense7Pattern;
  return Qt::SolidPattern;
}

QString QgsSymbolLayerV2Utils::encodePoint(QPointF point)
{
  return QString("%1,%2").arg(point.x()).arg(point.y());
}

QPointF QgsSymbolLayerV2Utils::decodePoint(QString str)
{
  QStringList lst = str.split(',');
  if (lst.count() != 2)
    return QPointF(0,0);
  return QPointF( lst[0].toDouble(), lst[1].toDouble() );
}

QIcon QgsSymbolLayerV2Utils::symbolPreviewIcon(QgsSymbolV2* symbol, QSize size)
{
  QPixmap pixmap(size);
  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHint( QPainter::Antialiasing );
  painter.eraseRect(QRect(QPoint(0,0),size));
  symbol->drawPreviewIcon(&painter, size);
  painter.end();
  return QIcon(pixmap);
}

QIcon QgsSymbolLayerV2Utils::symbolLayerPreviewIcon(QgsSymbolLayerV2* layer, QSize size)
{
  QPixmap pixmap(size);
  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHint( QPainter::Antialiasing );
  painter.eraseRect(QRect(QPoint(0,0),size));
  layer->drawPreviewIcon(&painter, size);
  painter.end();
  return QIcon(pixmap);
}

QIcon QgsSymbolLayerV2Utils::colorRampPreviewIcon(QgsVectorColorRampV2* ramp, QSize size)
{
  return QIcon(colorRampPreviewPixmap(ramp, size));
}

QPixmap QgsSymbolLayerV2Utils::colorRampPreviewPixmap(QgsVectorColorRampV2* ramp, QSize size)
{
  QPixmap pixmap(size);
  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHint( QPainter::Antialiasing );
  painter.eraseRect(QRect(QPoint(0,0),size));
  for (int i = 0; i < size.width(); i++)
  {
    QPen pen( ramp->color( (double) i / size.width() ) );
    painter.setPen(pen);
    painter.drawLine(i,0,i,size.height()-1);
  }
  painter.end();
  return pixmap;
}


#include <QPolygonF>

#include <cmath>
#include <cfloat>


// calculate line's angle and tangent
static bool lineInfo(QPointF p1, QPointF p2, double& angle, double& t)
{
  double x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();

  if (x1 == x2 && y1 == y2)
    return false;

  // tangent
  t = ( x1 == x2 ? t = DBL_MAX : (y2-y1)/(x2-x1) );

  // angle
  if (t == DBL_MAX)
    angle = ( y2 >= y1 ? M_PI/2 : M_PI*3/2 );  // angle is 90 or 270
  else if (t >= 0)
    angle = ( y2 >= y1 ? atan(t) : M_PI + atan(t) );
  else // t < 0
    angle = ( y2 >= y1 ? M_PI + atan(t) : M_PI*2 + atan(t) );

  return true;
}

// offset a point with an angle and distance
static QPointF offsetPoint(QPointF pt, double angle, double dist)
{
  return QPointF(pt.x() + dist * cos(angle), pt.y() + dist * sin(angle));
}

// calc intersection of two (infinite) lines defined by one point and tangent
static QPointF linesIntersection(QPointF p1, double t1, QPointF p2, double t2)
{
  // parallel lines?
  if ( (t1 == DBL_MAX && t2 == DBL_MAX) || t1 == t2)
    return QPointF();

  double x,y;
  if (t1 == DBL_MAX || t2 == DBL_MAX)
  {
    // in case one line is with angle 90 resp. 270 degrees (tangent undefined)
    // swap them so that line 2 is with undefined tangent
    if (t1 == DBL_MAX)
    {
      QPointF pSwp = p1; p1 = p2; p2 = pSwp;
      double  tSwp = t1; t1 = t2; t2 = tSwp;
    }

    x = p2.x();
  }
  else
  {
    // usual case
    x = ( (p1.y() - p2.y()) + t2*p2.x() - t1*p1.x() ) / (t2 - t1);
  }

  y = p1.y() + t1 * (x - p1.x());
  return QPointF(x,y);
}


QPolygonF offsetLine(QPolygonF polyline, double dist)
{
  QPolygonF newLine;

  if (polyline.count() < 2)
    return newLine;

  double angle, t_new, t_old=0;
  QPointF pt_old, pt_new;
  QPointF p1 = polyline[0], p2;

  for (int i = 1; i < polyline.count(); i++)
  {
    p2 = polyline[i];

    if ( !lineInfo(p1, p2, angle, t_new) )
      continue; // not a line...

    pt_new = offsetPoint(p1, angle + M_PI/2, dist);

    if (i != 1)
    {
      // if it's not the first line segment
      // calc intersection with last line (with offset)
      pt_new = linesIntersection(pt_old, t_old, pt_new, t_new);
    }

    newLine.append(pt_new);

    pt_old = pt_new;
    t_old = t_new;
    p1 = p2;
  }

  // last line segment:
  pt_new = offsetPoint(p2, angle + M_PI/2, dist);
  newLine.append(pt_new);
  return newLine;
}
