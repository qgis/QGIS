/***************************************************************************
                          qgsserver.sip
 Qgis Mapserver
                          -------------------
  begin                : 2015-05-21
  copyright            : (C) 2015 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


%MappedType QPair<QByteArray, QByteArray>
{
%TypeHeaderCode
#include <QPair>
#include <QByteArray>
%End


%TypeCode
// Convenience function for converting a QByteArray to a Python str object. (from QtCore/qbytearray.sip)
static PyObject *QByteArrayToPyStr(QByteArray *ba)
{
  char *data = ba->data();

  if (data)
    // QByteArrays may have embedded '\0's so set the size explicitly.
    return SIPBytes_FromStringAndSize(data, ba->size());
  return SIPBytes_FromString("");
}
%End


%ConvertFromTypeCode
    // Create the tuple.
    return Py_BuildValue((char *)"OO", QByteArrayToPyStr( &sipCpp->first ), QByteArrayToPyStr( &sipCpp->second ) );
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
        return (PyTuple_Size(sipPy) == 2);

    QPair<QByteArray, QByteArray> *qp = new QPair<QByteArray, QByteArray>;

    qp->first = SIPLong_AsLong(PyTuple_GET_ITEM(sipPy, 0));
    qp->second = SIPLong_AsLong(PyTuple_GET_ITEM(sipPy, 1));

    *sipCppPtr = qp;

    return sipGetState(sipTransferObj);
%End
};

class QgsServer
{
%TypeHeaderCode
#include "qgsserver.h"
%End

  public:
    QgsServer();
    ~QgsServer();
    // Original init for the fcgi application:
    //void init( int argc, char* argv[] );
    // init for python bindings:
    void init( );
    QPair<QByteArray, QByteArray> handleRequest( const QString queryString = QString( ) );
%If (HAVE_SERVER_PYTHON_PLUGINS)
     QgsServerInterface* serverInterface( );
%End

  // The following is needed because otherwise SIP fails trying to create copy
  // ctor
  private:
    QgsServer( const QgsServer& );
    QgsServer & operator=( const QgsServer& );
};
