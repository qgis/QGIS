#include <qstring.h>
#include "qgsrect.h"
QgsRect::QgsRect(double minX, double minY, double maxX, double maxY) :
  xmin(minX), ymin(minY), xmax(maxX), ymax(maxY){
}
QgsRect::~QgsRect(){
}
void QgsRect::setXmin(double x){
  xmin = x;
}
void QgsRect::setXmax(double x){
  xmax = x;
}
void QgsRect::setYmin(double y){
  ymin = y;
}
void QgsRect::setYmax(double y){
  ymax = y;
}
double QgsRect::xMax() const {
  return xmax;
}
double QgsRect::xMin() const {
  return xmin;
}
double QgsRect::yMax() const {
  return ymax;
}
double QgsRect::yMin() const {
  return ymin;
}
double QgsRect::width() const{
  return xmax - xmin;
}
double QgsRect::height() const {
  return ymax - ymin;
}
void QgsRect::normalize(){
  double temp;
  if( xmin > xmax){
    temp = xmin;
    xmin = xmax;
    xmax = temp;
  }
  if(ymin > ymax){
    temp = ymin;
    ymin = ymax;
    ymax = temp;
  }
}
QString QgsRect::stringRep() const{
  QString tmp;
  QString rep = tmp.setNum(xmin);;
  rep += " ";
  rep += tmp.setNum(ymin);
  rep += ",";
  rep += tmp.setNum(xmax);
  rep += " ";
  rep += tmp.setNum(ymax);
  return rep;
}
bool QgsRect::operator==(const QgsRect &r1){
  return (r1.xMax() == this->xMax() && r1.xMin() == this->xMin() &&
	  r1.yMax() == this->yMax() && r1.yMin() == this->yMin());
}

QgsRect & QgsRect::operator=(const QgsRect &r){
  if(&r != this){
    xmax = r.xMax();
    xmin = r.xMin();
    ymax = r.yMax();
    ymin = r.yMin();
  }
    return *this;
  
  
}
