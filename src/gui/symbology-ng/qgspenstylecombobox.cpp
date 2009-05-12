
#include "qgspenstylecombobox.h"

#include <QList>
#include <QPair>

#include <QPainter>
#include <QPen>

QgsPenStyleComboBox::QgsPenStyleComboBox(QWidget* parent)
  : QComboBox(parent)
{
  QList < QPair<Qt::PenStyle, QString> > styles;
  styles << qMakePair(Qt::SolidLine, QString("Solid Line"))
      << qMakePair(Qt::DashLine, QString("Dash Line"))
      << qMakePair(Qt::DotLine, QString("Dot Line"))
      << qMakePair(Qt::DashDotLine, QString("Dash Dot Line"))
      << qMakePair(Qt::DashDotDotLine, QString("Dash Dot Dot Line"));
  
  setIconSize(QSize(32,12));
  
  for (int i = 0; i < styles.count(); i++)
  {
    Qt::PenStyle style = styles.at(i).first;
    QString name = styles.at(i).second;
    addItem(iconForPen(style), name, QVariant(style));
  }
}
  
Qt::PenStyle QgsPenStyleComboBox::penStyle() const
{
  return (Qt::PenStyle) itemData(currentIndex()).toInt();
}
  
void QgsPenStyleComboBox::setPenStyle(Qt::PenStyle style)
{
  int idx = findData(QVariant(style));
  setCurrentIndex( idx == -1 ? 0 : idx );
}

QIcon QgsPenStyleComboBox::iconForPen(Qt::PenStyle style)
{
  QPixmap pix(iconSize());
  QPainter p;
  pix.fill(Qt::transparent);
  
  p.begin(&pix);
  QPen pen(style);
  pen.setWidth(2);
  p.setPen(pen);
  double mid = iconSize().height() / 2.0;
  p.drawLine(0,mid,iconSize().width(),mid);
  p.end();
  
  return QIcon(pix);
}
