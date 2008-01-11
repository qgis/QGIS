
class QgsRasterShader
{
%TypeHeaderCode
#include <qgsrastershader.h>
#include <qgsrastershaderfunction.h>
%End

public:
  QgsRasterShader(double theMinimumValue=0.0, double theMaximumValue=255.0);
  ~QgsRasterShader();
  /*
   *
   * Non-Static Inline methods
   *
   */
  /** \brief Return the maximum value for the raster shader */
  double getMaximumValue();
     
  /** \brief Return the minimum value for the raster shader */
  double getMinimumValue();
  
  QgsRasterShaderFunction* getRasterShaderFunction();
  
  /*
   *
   * Non-Static methods
   *
   */  
  /** \brief generates and new RGB value based on one input value */
  bool generateShadedValue(double, int*, int*, int*);
  /** \brief generates and new RGB value based on original RGB value */
  bool generateShadedValue(double, double, double, int*, int*, int*);
  /** \brief A public method that allows the user to set their own shader function */
  void setRasterShaderFunction(QgsRasterShaderFunction*);
  /** \brief Set the maximum value */
  void setMaximumValue(double);
  /** \brief Return the minimum value */
  void setMinimumValue(double);
};