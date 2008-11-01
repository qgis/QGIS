
class QgsRasterShaderFunction
{
%TypeHeaderCode
#include <qgsrastershaderfunction.h>
%End

public:
  QgsRasterShaderFunction(double theMinimumValue = 0.0, double theMaximumValue = 255.0);
  virtual ~QgsRasterShaderFunction();
   
  /** \brief generates and new RGB value based on one input value */
  virtual bool shade(double, int*, int*, int*);
  /** \brief generates and new RGB value based on original RGB value */
  virtual bool shade(double, double, double, int*, int*, int*);
  
  /** \brief Set the maximum value */
  virtual void setMaximumValue(double);
  /** \brief Return the minimum value */
  virtual void setMinimumValue(double);
};
