#ifndef QGSIMAGEWARPER_H
#define QGSIMAGEWARPER_H

#include <gdalwarper.h>
#include <qstring.h>


class QgsImageWarper {
public:
  
  enum ResamplingMethod {
    NearestNeighbour = GRA_NearestNeighbour,
    Bilinear = GRA_Bilinear,
    Cubic = GRA_Cubic
  };
  
  
  QgsImageWarper(double angle) : mAngle(angle) { }
  
  void warp(const QString& input, const QString& output, 
	    double& xOffset, double& yOffset, 
	    ResamplingMethod resampling = Bilinear, bool useZeroAsTrans = true);
  
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
