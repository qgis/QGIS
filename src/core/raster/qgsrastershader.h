/* **************************************************************************
                qgsrastershader.h -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSRASTERSHADER_H
#define QGSRASTERSHADER_H

#include "qgsrastershaderfunction.h"

class CORE_EXPORT QgsRasterShader {

public:
  QgsRasterShader(double theMinimumValue=0.0, double theMaximumValue=255.0);
  ~QgsRasterShader();
    
  /*
   *
   * Non-Static Inline methods
   *
   */
  /** \brief Return the maximum value for the raster shader */
  double getMaximumValue() { return mMaximumValue; }
     
  /** \brief Return the minimum value for the raster shader */
  double getMinimumValue() { return mMinimumValue; }
  
  QgsRasterShaderFunction* getRasterShaderFunction() { return mRasterShaderFunction; }
  
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
  
private:
  /** \brief User defineable minimum value for the raster shader */
  double mMinimumValue;
  /** \brief user defineable maximum value for the raster shader */
  double mMaximumValue;
    
  QgsRasterShaderFunction* mRasterShaderFunction;
};
#endif
/* **************************************************************************
                qgsrastershader.h -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSRASTERSHADER_H
#define QGSRASTERSHADER_H

#include "qgsrastershaderfunction.h"

class CORE_EXPORT QgsRasterShader {

public:
  QgsRasterShader(double theMinimumValue=0.0, double theMaximumValue=255.0);
  ~QgsRasterShader();
    
  /*
   *
   * Non-Static Inline methods
   *
   */
  /** \brief Return the maximum value for the raster shader */
  double getMaximumValue() { return mMaximumValue; }
     
  /** \brief Return the minimum value for the raster shader */
  double getMinimumValue() { return mMinimumValue; }
  
  QgsRasterShaderFunction* getRasterShaderFunction() { return mRasterShaderFunction; }
  
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
  
private:
  /** \brief User defineable minimum value for the raster shader */
  double mMinimumValue;
  /** \brief user defineable maximum value for the raster shader */
  double mMaximumValue;
    
  QgsRasterShaderFunction* mRasterShaderFunction;
};
#endif
/* **************************************************************************
                qgsrastershader.h -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSRASTERSHADER_H
#define QGSRASTERSHADER_H

#include "qgsrastershaderfunction.h"

class CORE_EXPORT QgsRasterShader {

public:
  QgsRasterShader(double theMinimumValue=0.0, double theMaximumValue=255.0);
  ~QgsRasterShader();
    
  /*
   *
   * Non-Static Inline methods
   *
   */
  /** \brief Return the maximum value for the raster shader */
  double getMaximumValue() { return mMaximumValue; }
     
  /** \brief Return the minimum value for the raster shader */
  double getMinimumValue() { return mMinimumValue; }
  
  QgsRasterShaderFunction* getRasterShaderFunction() { return mRasterShaderFunction; }
  
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
  
private:
  /** \brief User defineable minimum value for the raster shader */
  double mMinimumValue;
  /** \brief user defineable maximum value for the raster shader */
  double mMaximumValue;
    
  QgsRasterShaderFunction* mRasterShaderFunction;
};
#endif
