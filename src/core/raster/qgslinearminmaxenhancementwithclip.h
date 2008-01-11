/* **************************************************************************
              qgslinearminmaxenhancementwithclip.h -  description
                       -------------------
begin                : Fri Nov 16 2007
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

#ifndef QGSLINEARMINMAXENHANCEMENTWITHCLIP_H
#define QGSLINEARMINMAXENHANCEMENTWITHCLIP_H

#include "qgscontrastenhancementfunction.h"

class CORE_EXPORT QgsLinearMinMaxEnhancementWithClip : public QgsContrastEnhancementFunction
{

  public:
    QgsLinearMinMaxEnhancementWithClip(QgsContrastEnhancement::QgsRasterDataType, double, double);
    int enhanceValue(double);
    bool isValueInDisplayableRange(double);
};

#endif
/* **************************************************************************
              qgslinearminmaxenhancementwithclip.h -  description
                       -------------------
begin                : Fri Nov 16 2007
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

#ifndef QGSLINEARMINMAXENHANCEMENTWITHCLIP_H
#define QGSLINEARMINMAXENHANCEMENTWITHCLIP_H

#include "qgscontrastenhancementfunction.h"

class CORE_EXPORT QgsLinearMinMaxEnhancementWithClip : public QgsContrastEnhancementFunction
{

  public:
    QgsLinearMinMaxEnhancementWithClip(QgsContrastEnhancement::QgsRasterDataType, double, double);
    int enhanceValue(double);
    bool isValueInDisplayableRange(double);
};

#endif
/* **************************************************************************
              qgslinearminmaxenhancementwithclip.h -  description
                       -------------------
begin                : Fri Nov 16 2007
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

#ifndef QGSLINEARMINMAXENHANCEMENTWITHCLIP_H
#define QGSLINEARMINMAXENHANCEMENTWITHCLIP_H

#include "qgscontrastenhancementfunction.h"

class CORE_EXPORT QgsLinearMinMaxEnhancementWithClip : public QgsContrastEnhancementFunction
{

  public:
    QgsLinearMinMaxEnhancementWithClip(QgsContrastEnhancement::QgsRasterDataType, double, double);
    int enhanceValue(double);
    bool isValueInDisplayableRange(double);
};

#endif
