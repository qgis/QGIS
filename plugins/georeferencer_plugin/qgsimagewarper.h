#ifndef QGSIMAGEWARPER_H
#define QGSIMAGEWARPER_H

#include <qstring.h>


class QgsImageWarper {
public:
  
  QgsImageWarper(double angle) : mAngle(angle) { }
  
  void warp(const QString& input, const QString& output, 
	    double& xOffset, double& yOffset);
  
private:
  
  struct TransformParameters {
    double angle;
    double x0;
    double y0;
  };

  
  static int transform(void *pTransformerArg, int bDstToSrc, int nPointCount, 
		       double *x, double *y, double *z, int *panSuccess);
  
  double mAngle;
  
};


#endif
