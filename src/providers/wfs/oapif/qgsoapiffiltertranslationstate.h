/***************************************************************************
    qgsoapiffiltertranslationstate.h
    --------------------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFFILTERTRANSLATIONSTATE_H
#define QGSOAPIFFILTERTRANSLATIONSTATE_H

enum class QgsOapifFilterTranslationState
{
  FULLY_CLIENT,
  PARTIAL,
  FULLY_SERVER
};

#endif
