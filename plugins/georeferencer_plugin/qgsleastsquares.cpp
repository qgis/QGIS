#include <cmath>
#include <stdexcept>

#include "qgsleastsquares.h"


void QgsLeastSquares::linear(std::vector<QgsPoint> mapCoords, 
			     std::vector<QgsPoint> pixelCoords,
			     QgsPoint& origin, double& pixelSize) {
  int n = mapCoords.size();
  if (n < 2) {
    throw std::domain_error("Fit to a linear transform requires at "
			    "least 2 points");
  }

  double sumPx(0), sumPy(0), sumPx2(0), sumPy2(0), sumPxMx(0), sumPyMy(0),
    sumMx(0), sumMy(0);
  for (int i = 0; i < n; ++i) {
    sumPx += pixelCoords[i].x();
    sumPy += pixelCoords[i].y();
    sumPx2 += std::pow(pixelCoords[i].x(), 2);
    sumPy2 += std::pow(pixelCoords[i].y(), 2);
    sumPxMx += pixelCoords[i].x() * mapCoords[i].x();
    sumPyMy += pixelCoords[i].y() * mapCoords[i].y();
    sumMx += mapCoords[i].x();
    sumMy += mapCoords[i].y();
  }
  
  double deltaX = n * sumPx2 - std::pow(sumPx, 2);
  double deltaY = n * sumPy2 - std::pow(sumPy, 2);
  
  double aX = (sumPx2 * sumMx - sumPx * sumPxMx) / deltaX;
  double aY = (sumPy2 * sumMy - sumPy * sumPyMy) / deltaY;
  double bX = (n * sumPxMx - sumPx * sumMx) / deltaX;
  double bY = (n * sumPyMy - sumPy * sumMy) / deltaY;
  
  origin.setX(aX);
  origin.setY(aY);
  pixelSize = (std::abs(bX) + std::abs(bY)) / 2;
}

  
void QgsLeastSquares::helmert(std::vector<QgsPoint> mapCoords, 
			      std::vector<QgsPoint> pixelCoords,
			      QgsPoint& origin, double& pixelSize, 
			      double& rotation) {
  throw std::logic_error("The Helmert transform is not implemented");
}
