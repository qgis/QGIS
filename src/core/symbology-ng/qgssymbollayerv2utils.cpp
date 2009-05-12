
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
