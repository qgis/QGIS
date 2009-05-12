
#ifndef QGSVECTORCOLORRAMPV2_H
#define QGSVECTORCOLORRAMPV2_H

#include <QColor>

#include "qgssymbollayerv2.h" // for QgsStringMap

class QgsVectorColorRampV2
{
public:
  virtual ~QgsVectorColorRampV2() {}
  
  virtual QColor color(double value) const = 0;
  
  virtual QString type() const = 0;
  
  virtual QgsVectorColorRampV2* clone() const = 0;
  
  virtual QgsStringMap properties() const = 0;
  
};

#define DEFAULT_GRADIENT_COLOR1 QColor(0,0,255)
#define DEFAULT_GRADIENT_COLOR2 QColor(0,255,0)

class QgsVectorGradientColorRampV2 : public QgsVectorColorRampV2
{
public:
  QgsVectorGradientColorRampV2(QColor color1 = DEFAULT_GRADIENT_COLOR1,
                               QColor color2 = DEFAULT_GRADIENT_COLOR2);
  
  static QgsVectorColorRampV2* create(const QgsStringMap& properties = QgsStringMap());
    
  virtual QColor color(double value) const;
  
  virtual QString type() const { return "gradient"; }
  
  virtual QgsVectorColorRampV2* clone() const;
  
  virtual QgsStringMap properties() const;
  
  QColor color1() const { return mColor1; }
  QColor color2() const { return mColor2; }
  
  void setColor1(QColor color) { mColor1 = color; }
  void setColor2(QColor color) { mColor2 = color; }
  
protected:
  QColor mColor1, mColor2;
};

#endif
