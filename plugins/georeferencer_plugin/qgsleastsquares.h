#ifndef QGSLEASTSQUARES_H
#define QGSLEASTSQUARES_H

#include <vector>
#include <stdexcept>

#include "qgspoint.h"


class QgsLeastSquares {
 public:
  static void linear(std::vector<QgsPoint> mapCoords, 
		     std::vector<QgsPoint> pixelCoords,
		     QgsPoint& origin, double& pixelSize);
  
  static void helmert(std::vector<QgsPoint> mapCoords, 
		      std::vector<QgsPoint> pixelCoords,
		      QgsPoint& origin, double& pixelSize, double& rotation);
};


#endif
