/***************************************************************************
                         qgsserver.sip
  QGIS Server main class.
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

// See if we are just being asked to check the type of the Python
// object.
if (!sipIsErr)
{
    // Checking whether or not None has been passed instead of a list
    // has already been done.
    if (!PyTuple_Check(sipPy) || PyTuple_Size(sipPy) != 2)
        return 0;

    // Check the type of each element.  We specify SIP_NOT_NONE to
    // disallow None because it is a list of QPoint, not of a pointer
    // to a QPoint, so None isn't appropriate.
    for (int i = 0; i < PyTuple_Size(sipPy); ++i)
        if (!sipCanConvertToType(PyTuple_GET_ITEM(sipPy, i),
                                    sipType_QByteArray, SIP_NOT_NONE))
            return 0;

    // The type is valid.
    return 1;
}

// Create the instance on the heap.
QPair<QByteArray, QByteArray> *qp = new QPair<QByteArray, QByteArray>;

QByteArray *qba1;
int state;

// Get the address of the element's C++ instance.  Note that, in
// this case, we don't apply any ownership changes to the list
// elements, only to the list itself.
qba1 = reinterpret_cast<QByteArray *>(sipConvertToType(
                                        PyTuple_GET_ITEM(sipPy, 0),
                                        sipType_QByteArray, 0,
                                        SIP_NOT_NONE,
                                        &state, sipIsErr));

// Deal with any errors.
if (*sipIsErr)
{
    sipReleaseType(qba1, sipType_QByteArray, state);

    // Tidy up.
    delete qp;

    // There is no temporary instance.
    return 0;
}

qp->first = *qba1;

// A copy of the QByteArray was assigned to the pair so we no longer
// need it.  It may be a temporary instance that should be
// destroyed, or a wrapped instance that should not be destroyed.
// sipReleaseType() will do the right thing.
sipReleaseType(qba1, sipType_QByteArray, state);

/////////////////////////////////////////////
// Second item

QByteArray *qba2;

// Get the address of the element's C++ instance.  Note that, in
// this case, we don't apply any ownership changes to the list
// elements, only to the list itself.
qba2 = reinterpret_cast<QByteArray *>(sipConvertToType(
                                        PyTuple_GET_ITEM(sipPy, 1),
                                        sipType_QByteArray, 0,
                                        SIP_NOT_NONE,
                                        &state, sipIsErr));

// Deal with any errors.
if (*sipIsErr)
{
    sipReleaseType(qba1, sipType_QByteArray, state);
    sipReleaseType(qba2, sipType_QByteArray, state);

    // Tidy up.
    delete qp;

    // There is no temporary instance.
    return 0;
}

qp->second = *qba2;


// A copy of the QByteArray was assigned to the pair so we no longer
// need it.  It may be a temporary instance that should be
// destroyed, or a wrapped instance that should not be destroyed.
// sipReleaseType() will do the right thing.
sipReleaseType(qba2, sipType_QByteArray, state);


// Return the instance.
*sipCppPtr = qp;

// The instance should be regarded as temporary (and be destroyed as
// soon as it has been used) unless it has been transferred from
// Python.  sipGetState() is a convenience function that implements
// this common transfer behaviour.
return sipGetState(sipTransferObj);

%End
};

/** \ingroup server
 * The QgsServer class provides OGC web services.
 */
class QgsServer
{
%TypeHeaderCode
#include "qgsserver.h"
%End

  public:
    QgsServer();
    ~QgsServer();
    /** Server initialization: intialise QGIS ang QT core application.
     * This method is automatically called by handleRequest if it wasn't
     * explicitly called before
     * @note Not available in Python bindings
     */
    //void init( int argc, char ** argv );
    //! The following is mainly for python bindings, that do not pass argc/argv
    void init();

    /** Set environment variable
     * @param var environment variable name
     * @param val value
     * @note added in 2.14
     */
    void putenv( const QString &var, const QString &val );

    /** Handles the request. The output is normally printed trough FCGI printf
     * by the request handler or, in case the server has been invoked from python
     * bindings, a flag is set that captures all the output headers and body, instead
     * of printing it returns the output as a QPair of QByteArray.
     * The query string is normally read from environment
     * but can be also passed in args and in this case overrides the environment
     * variable
     *
     * @param queryString optional QString containing the query string
     * @return the response headers and body QPair of QByteArray if called from python bindings, empty otherwise
     */
    QPair<QByteArray, QByteArray> handleRequest( const QString& queryString = QString() );
    /*
    // The following code was used to test type conversion in python bindings
    QPair<QByteArray, QByteArray> testQPair( QPair<QByteArray, QByteArray> pair );
    */

    /** Returns a pointer to the server interface */
%If (HAVE_SERVER_PYTHON_PLUGINS)
     QgsServerInterface* serverInterface();
%End

  private:
    QgsServer( const QgsServer& );
    QgsServer & operator=( const QgsServer& );
};
