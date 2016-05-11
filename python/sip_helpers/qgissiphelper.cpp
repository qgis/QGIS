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
  if ( varp->isNull() )
  {
#if 0
    PyObject *tobj = sipConvertFromType(( void * )varp, sipType_QVariant, sipTransferObj );
    Py_INCREF( Py_None );
    *objp = Py_None;
#endif
    static bool watchdog = false;

    if ( watchdog )
      return false;

    watchdog = true;
    *objp = PyObject_Call(( PyObject * )sipTypeAsPyTypeObject( sipType_QVariant ), PyTuple_New( 0 ), nullptr );
    watchdog = false;
    return true;
  }
  return false;
}
