
#include "qgsbrushstylecombobox.h"

#include <QList>
#include <QPair>

#include <QBrush>
#include <QPainter>
#include <QPen>

QgsBrushStyleComboBox::QgsBrushStyleComboBox(QWidget* parent)
  : QComboBox(parent)
{
  QList < QPair<Qt::BrushStyle, QString> > styles;
  styles << qMakePair(Qt::SolidPattern, QString("Solid"))
      << qMakePair(Qt::HorPattern, QString("Horizontal"))
      << qMakePair(Qt::VerPattern, QString("Vertical"))
      << qMakePair(Qt::CrossPattern, QString("Cross"))
      << qMakePair(Qt::BDiagPattern, QString("BDiagonal"))
      << qMakePair(Qt::FDiagPattern, QString("FDiagonal"))
      << qMakePair(Qt::DiagCrossPattern, QString("Diagonal X"))
      << qMakePair(Qt::Dense1Pattern, QString("Dense 1"))
      << qMakePair(Qt::Dense2Pattern, QString("Dense 2"))
      << qMakePair(Qt::Dense3Pattern, QString("Dense 3"))
      << qMakePair(Qt::Dense4Pattern, QString("Dense 4"))
      << qMakePair(Qt::Dense5Pattern, QString("Dense 5"))
      << qMakePair(Qt::Dense6Pattern, QString("Dense 6"))
      << qMakePair(Qt::Dense7Pattern, QString("Dense 7"))
      << qMakePair(Qt::NoBrush, QString("No Brush"));
  
  setIconSize(QSize(32,16));
  
  for (int i = 0; i < styles.count(); i++)
  {
    Qt::BrushStyle style = styles.at(i).first;
    QString name = styles.at(i).second;
    addItem(iconForBrush(style), name, QVariant(style));
  }  
  
 setCurrentIndex(1);

}


Qt::BrushStyle QgsBrushStyleComboBox::brushStyle() const
{
  return (Qt::BrushStyle) itemData(currentIndex()).toInt();
}
  
void QgsBrushStyleComboBox::setBrushStyle(Qt::BrushStyle style)
{
  int idx = findData(QVariant(style));
  setCurrentIndex( idx == -1 ? 0 : idx );
}

QIcon QgsBrushStyleComboBox::iconForBrush(Qt::BrushStyle style)
{
  QPixmap pix(iconSize());
  QPainter p;
  pix.fill(Qt::transparent);
  
  p.begin(&pix);
  QBrush brush(QColor(100,100,100), style);
  p.setBrush(brush);
  QPen pen(Qt::NoPen);
  p.setPen(pen);
  p.drawRect(QRect(QPoint(0,0),iconSize()));
  p.end();
  
  return QIcon(pix);
}
