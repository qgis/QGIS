/***************************************************************************
                          qgissiphelper.h
                             -------------------
    begin                : Wed, May 11 2016
    copyright            : (C) 2016 by Matthias Kuhn
    email                : matthias@opengis.ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <Python.h>

class QVariant;

typedef bool ( *FromQVariantConvertorFn )( const QVariant *, PyObject ** );

bool null_from_qvariant_convertor( const QVariant *, PyObject ** );
