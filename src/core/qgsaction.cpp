/***************************************************************************
  qgsaction.cpp - QgsAction

 ---------------------
 begin                : 18.4.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaction.h"

bool QgsAction::runable() const
{
  return mType == Generic ||
         mType == GenericPython ||
         mType == OpenUrl ||
#if defined(Q_OS_WIN)
         mType == Windows
#elif defined(Q_OS_MAC)
         mType == Mac
#else
         mType == Unix
#endif
         ;
}
