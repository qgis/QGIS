
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

class QgsPenJoinStyleComboBox : public QComboBox
{
public:
  QgsPenJoinStyleComboBox(QWidget* parent = NULL);

  Qt::PenJoinStyle penJoinStyle() const;

  void setPenJoinStyle(Qt::PenJoinStyle style);
};

class QgsPenCapStyleComboBox : public QComboBox
{
public:
  QgsPenCapStyleComboBox(QWidget* parent = NULL);

  Qt::PenCapStyle penCapStyle() const;

  void setPenCapStyle(Qt::PenCapStyle style);
};

#endif
