#include "qgspoint.h"
#include "qgscoordinatetransform.h"

QgsCoordinateTransform::QgsCoordinateTransform(double mupp=0, double ymax = 0, double ymin=0, double xmin = 0) : 
  mapUnitsPerPixel(mupp), yMax(ymax), yMin(ymin), xMin(xmin){
}
QgsCoordinateTransform::~QgsCoordinateTransform(){
}
QgsPoint QgsCoordinateTransform::transform(QgsPoint p){
}

QgsPoint QgsCoordinateTransform::transform(double x, double y){
}
