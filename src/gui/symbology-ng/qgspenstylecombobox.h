
#ifndef QGSPENSTYLECOMBOBOX_H
#define QGSPENSTYLECOMBOBOX_H

#include <QComboBox>

class QgsPenStyleComboBox : public QComboBox
{
public:
  QgsPenStyleComboBox(QWidget* parent = NULL);
  
  Qt::PenStyle penStyle() const;
  
  void setPenStyle(Qt::PenStyle style);
  
protected:
  QIcon iconForPen(Qt::PenStyle style);
    
};

#endif
