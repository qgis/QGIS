#include "qgspoint.h"
QgsPoint::QgsPoint(){
}

QgsPoint::QgsPoint(double x, double y) : m_x(x), m_y(y){
 
}

QgsPoint::~QgsPoint(){
}

double QgsPoint::x() const {
  return m_x;
}

double QgsPoint::y() const {
  return m_y;
}
int QgsPoint::xToInt() {
  return (int)m_x;
}
int QgsPoint::yToInt() {
  return (int)m_y;
}
bool QgsPoint::operator==(const QgsPoint &other){
  if((m_x == other.x()) && (m_y == other.y()))
    return true;
  else
    return false;
}
bool QgsPoint::operator!=(const QgsPoint &other){
  if((m_x == other.x()) && (m_y == other.y()))
    return false;
  else
    return true;
}

QgsPoint & QgsPoint::operator=(const QgsPoint &other){
  if(&other != this){
    m_x = other.x();
    m_y = other.y();
  }
  
    return *this;
}
