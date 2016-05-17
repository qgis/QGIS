/***************************************************************************
                          qgissiphelper.cpp
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

#include "qgissiphelper.h"
#include "sipAPI_core.h"
#include <sip.h>

#include <QVariant>
#include <Python.h>

bool null_from_qvariant_convertor( const QVariant *varp, PyObject **objp )
{
  static bool watchdog = false;

  if ( watchdog )
    return false;

  // If we deal with a NULL QVariant (and it's not a QByteArray which properly
  // maps NULL values)
  // If there are more cases like QByteArray, we should consider using a whitelist
  // instead of a blacklist.
  if ( varp->isNull() && varp->type() != QVariant::ByteArray )
  {
    watchdog = true;
    PyObject* vartype = sipConvertFromEnum( varp->type(), sipType_QVariant_Type );
    *objp = PyObject_Call(( PyObject * )sipTypeAsPyTypeObject( sipType_QVariant ), PyTuple_Pack( 1, vartype ), nullptr );
    watchdog = false;
    return true;
  }
  else
  {
    return false;
  }
}
