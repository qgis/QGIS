#ifndef QGSLEASTSQUARES_H
#define QGSLEASTSQUARES_H

#include <vector>
#include <cstdarg>
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
  
  // private:
  /*
  class LinearSystem {
  public:
    
    LinearSystem(int n, double** coeff) {
      mCoeff = new double*[n];
      for (int r = 0; r < n; ++r) {
	mCoeff[r] = new double[n+1];
	for (int c = 0; c < n + 1; ++c)
	  mCoeff[r][c] = coeff[r][c];
      }
    }
    
    void solve(std::vector<double>& solution) {
      
    }
      
  private:
    double** mCoeff;
  };
  */
};


#endif
