
#ifndef QGSBRUSHSTYLECOMBOBOX_H
#define QGSBRUSHSTYLECOMBOBOX_H

#include <QComboBox>

class QgsBrushStyleComboBox : public QComboBox
{
  public:
    QgsBrushStyleComboBox(QWidget* parent = NULL);
  
    Qt::BrushStyle brushStyle() const;
  
    void setBrushStyle(Qt::BrushStyle style);
  
  protected:
    QIcon iconForBrush(Qt::BrushStyle style);
    
};

#endif
