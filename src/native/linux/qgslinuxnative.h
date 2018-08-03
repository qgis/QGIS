/***************************************************************************
    qgslinuxnative.h
                             -------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINUXNATIVE_H
#define QGSLINUXNATIVE_H

#include "qgsnative.h"

class NATIVE_EXPORT QgsLinuxNative : public QgsNative
{
  public:
    void openFileExplorerAndSelectFile( const QString &path ) override;
};

#endif // QGSLINUXNATIVE_H
