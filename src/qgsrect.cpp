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
double QgsRect::xMax(){
  return xmax;
}
double QgsRect::xMin(){
  return xmin;
}
double QgsRect::yMax(){
  return ymax;
}
double QgsRect::yMin(){
  return ymin;
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
