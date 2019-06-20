/***************************************************************************
                             qgslayoutapputils.h
                             -------------------
    Date                 : October 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTAPPUTILS_H
#define QGSLAYOUTAPPUTILS_H

/**
 * Utils for layout handling from app.
 */
class QgsLayoutAppUtils
{

  public:

    /**
     * Registers the GUI handlers for known layout item types.
     */
    static void registerGuiForKnownItemTypes();

};



#endif // QGSLAYOUTAPPUTILS_H
