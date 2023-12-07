/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsattributes.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/









typedef QMap<int, QVariant> QgsAttributeMap;

typedef QMap<int, QString> QgsFieldNameMap;

typedef QMap<int, QgsField> QgsFieldMap;


typedef QVector<QVariant> QgsAttributes;

%MappedType QgsAttributes
{
%TypeHeaderCode
#include "qgsfeature.h"
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ( ( l = PyList_New( sipCpp->size() ) ) == NULL )
    return NULL;

  // Set the list elements.
  for ( int i = 0; i < sipCpp->size(); ++i )
  {
    QVariant *v = new QVariant( sipCpp->at( i ) );
    PyObject *tobj;

    if ( ( tobj = sipConvertFromNewType( v, sipType_QVariant, Py_None ) ) == NULL )
    {
      Py_DECREF( l );
      delete v;

      return NULL;
    }

    PyList_SET_ITEM( l, i, tobj );
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if ( sipIsErr == NULL )
  {
    if ( !PyList_Check( sipPy ) )
      return 0;

    for ( SIP_SSIZE_T i = 0; i < PyList_GET_SIZE( sipPy ); ++i )
      if ( !sipCanConvertToType( PyList_GET_ITEM( sipPy, i ), sipType_QVariant, SIP_NOT_NONE ) )
        return 0;

    return 1;
  }

  QgsAttributes *qv = new QgsAttributes;
  SIP_SSIZE_T listSize = PyList_GET_SIZE( sipPy );
  qv->reserve( listSize );

  for ( SIP_SSIZE_T i = 0; i < listSize; ++i )
  {
    PyObject *obj = PyList_GET_ITEM( sipPy, i );
    if ( obj == Py_None )
    {
      qv->append( QVariant( QVariant::Int ) );
    }
    else
    {
      int state;
      QVariant *t = reinterpret_cast<QVariant *>( sipConvertToType( obj, sipType_QVariant, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr ) );

      if ( *sipIsErr )
      {
        sipReleaseType( t, sipType_QVariant, state );

        delete qv;
        return 0;
      }

      qv->append( *t );
      sipReleaseType( t, sipType_QVariant, state );
    }
  }

  *sipCppPtr = qv;

  return sipGetState( sipTransferObj );
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsattributes.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
