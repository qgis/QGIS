/***************************************************************************
                          qgsgrasswin.h
                             -------------------
    begin                : October, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRASSWIN_H
#define QGSGRASSWIN_H

// Windows utils
class GRASS_LIB_EXPORT QgsGrassWin
{
  public:
    static void hideWindow( int pid );
};

#endif // QGSGRASSWIN_H

