/***************************************************************************
    qgswfsguiutils.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSGUIUTILS_H
#define QGSWFSGUIUTILS_H

#include "qgswfscapabilities.h"

class QWidget;

class QgsWfsGuiUtils
{
  public:

    //! Display a message box when a capability error occur.
    static void displayErrorMessageOnFailedCapabilities( QgsWfsCapabilities *capabilities, QWidget *parent );
};

#endif // QGSWFSGUIUTILS_H
