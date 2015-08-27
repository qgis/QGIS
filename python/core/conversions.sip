/*
This file contains code for conversion between various (often nested) mapped types
which are not wrapped by PyQt:
- QVector< QVector<TYPE> >
- QVector< QVector< QVector<TYPE> > >
- QList< QList<TYPE> >
- QList<qint64>
- QSet<int>
- QSet<qint64>
- QSet<TYPE>
- QMap<qint64, QMap<int, TYPE> >
- QMap<QString, QVariant::Type>
- QMap<QString, int>
- QMap<TYPE1, TYPE2*>
- QMap<double, TYPE>
- QMultiMap<double, TYPE2>
- QMultiMap<int, TYPE2>
- QMap<qint64, TYPE>
- QList< QPair< QString, QList<QString> > >
- QVector<TYPE*>
- QMap<qint64, QgsFeature*>
*/

%Feature QSETINT_CONVERSION
%Feature QSETTYPE_CONVERSION
%Feature QLISTCONSTPTR_CONVERSION
%Feature QVECTORINT_CONVERSION

%ModuleHeaderCode

// From Python 2.5, some functions use Py_ssize_t instead of int
// thus this typedef is for maintaining backward compatibility
// for older versions of Python
#if (PY_VERSION_HEX < 0x02050000)
typedef int Py_ssize_t;
#endif

%End


template <TYPE>
%MappedType QVector< QVector<TYPE> >
{
%TypeHeaderCode
#include <QVector>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QgsPoint>");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<TYPE>* t = new QVector<TYPE>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromMappedType(t, qvector_qgspoint, sipTransferObj)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QgsPoint>");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QVector< QVector<TYPE> > *ql = new QVector< QVector<TYPE> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<TYPE> * t = reinterpret_cast< QVector<TYPE> * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseMappedType(t, qvector_qgspoint, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseMappedType(t, qvector_qgspoint, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};



template <TYPE>
%MappedType QVector< QVector< QVector<TYPE> > >
{
%TypeHeaderCode
#include <QVector>

%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QVector<QgsPoint> >");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<QVector<TYPE> >* t = new QVector<QVector<TYPE> >(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromMappedType(t, qvector_qgspoint, sipTransferObj)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }
  return l;
%End

%ConvertToTypeCode

  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QVector<QgsPoint> >");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QVector< QVector< QVector<TYPE> > > *ql = new QVector< QVector< QVector<TYPE> > >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<QVector<TYPE> > * t = reinterpret_cast< QVector< QVector<TYPE> > * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseMappedType(t, qvector_qgspoint, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseMappedType(t, qvector_qgspoint, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};



template <TYPE>
%MappedType QList< QList<TYPE> >
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  const sipMappedType* qlist_type = sipFindMappedType("QList<TYPE>");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QList<TYPE>* t = new QList<TYPE>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromMappedType(t, qlist_type, sipTransferObj)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  const sipMappedType* qlist_type = sipFindMappedType("QList<TYPE>");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToMappedType(PyList_GET_ITEM(sipPy, i), qlist_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QList< QList<TYPE> > *ql = new QList< QList<TYPE> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QList<TYPE> * t = reinterpret_cast< QList<TYPE> * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qlist_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseMappedType(t, qlist_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseMappedType(t, qlist_type, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};


%MappedType QList< QList<QgsPointV2> >
{
%TypeHeaderCode
#include <QList>
#include "qgspointv2.h"
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  const sipMappedType* qlist_type = sipFindMappedType("QList<QgsPointV2>");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QList<QgsPointV2>* t = new QList<QgsPointV2>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromMappedType(t, qlist_type, sipTransferObj)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  const sipMappedType* qlist_type = sipFindMappedType("QList<QgsPointV2>");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToMappedType(PyList_GET_ITEM(sipPy, i), qlist_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QList< QList<QgsPointV2> > *ql = new QList< QList<QgsPointV2> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QList<QgsPointV2> * t = reinterpret_cast< QList<QgsPointV2> * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qlist_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseMappedType(t, qlist_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseMappedType(t, qlist_type, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};

%MappedType QList< QList< QList<QgsPointV2> > >
{
%TypeHeaderCode
#include <QList>
#include "qgspointv2.h"
%End

%ConvertFromTypeCode
   PyObject *l;
   if ((l = PyList_New(sipCpp->size())) == NULL)
   {
      return NULL;
   }
   const sipMappedType* qlist_type = sipFindMappedType("QList<QList<QgsPointV2> >");
   if(!qlist_type )
   {
       Py_DECREF(l); return NULL;
   }

   for (int i = 0; i < sipCpp->size(); ++i)
   {
      QList< QList<QgsPointV2> >* t = new QList<QList<QgsPointV2> >(sipCpp->at(i));
      PyObject *tobj;
      if ((tobj = sipConvertFromMappedType(t, qlist_type, sipTransferObj)) == NULL)
      {
        Py_DECREF(l);
        delete t;
        return NULL;
      }
      PyList_SET_ITEM(l, i, tobj);
   }
   return l;
%End
%ConvertToTypeCode
    const sipMappedType* qlist_type = sipFindMappedType("QList<QList<QgsPointV2> >");
   if (sipIsErr == NULL)
   {
        return 0;
   }
   QList< QList< QList<QgsPointV2> > >* ql = new QList< QList< QList<QgsPointV2> > >();
   for( int i = 0;i < PyList_GET_SIZE(sipPy); ++i)
   {
       int state;
       QList<QList<QgsPointV2> > *t = reinterpret_cast< QList< QList<QgsPointV2> > * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qlist_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
       if( *sipIsErr )
       {
           sipReleaseMappedType(t, qlist_type, state);
           delete ql;
           return 0;
       }
       ql->append( *t );
       sipReleaseMappedType( t, qlist_type, state );
   }
   *sipCppPtr = ql;
   return sipGetState(sipTransferObj);


%End
};


%If (QSETINT_CONVERSION)
%MappedType QSet<int>
{
%TypeHeaderCode
#include <QSet>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QSet<int>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = PyInt_FromLong(*it)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QSet<int> *qset = new QSet<int>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    qset->insert(PyInt_AsLong(PyList_GET_ITEM(sipPy, i)));
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End
};
%End



%MappedType QList<qint64>
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QList<qint64>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = PyLong_FromLongLong(*it)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QList<qint64> *qlist = new QList<qint64>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    *qlist << PyLong_AsLongLong(PyList_GET_ITEM(sipPy, i));
  }

  *sipCppPtr = qlist;
  return sipGetState(sipTransferObj);
%End
};



%MappedType QSet<qint64>
{
%TypeHeaderCode
#include <QSet>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QSet<qint64>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = PyLong_FromLongLong(*it)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QSet<qint64> *qset = new QSet<qint64>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    qset->insert(PyLong_AsLongLong(PyList_GET_ITEM(sipPy, i)));
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End
};



%If (QSETTYPE_CONVERSION)
template <TYPE>
%MappedType QSet<TYPE>
{
%TypeHeaderCode
#include <QSet>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  int i=0;
  for (QSet<TYPE>::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it, ++i)
  {
    TYPE *t = new TYPE(*it);
    PyObject *tobj;

    if ((tobj = sipConvertFromNewType(t, sipType_TYPE, sipTransferObj)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, SIP_NOT_NONE))
        return 0;

    return 1;
  }

  QSet<TYPE> *qset = new QSet<TYPE>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    TYPE* t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, sipType_TYPE, state);
      delete qset;
      return 0;
    }
    qset->insert(*t);
    sipReleaseType(t, sipType_TYPE, state);
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End
};
%End



template<TYPE>
%MappedType QMap<qint64, QMap<int, TYPE> >
{
%TypeHeaderCode
#include <QtGlobal>
#include <QMap>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *d;

  if ((d = PyDict_New()) == NULL)
    return NULL;

  const sipMappedType* qmap2 = sipFindMappedType("QMap<int, TYPE>");

  // Set the list elements.
  for (QMap<qint64, QMap<int, TYPE> >::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it)
  {
    QMap<int, TYPE>* t = new QMap<int, TYPE>(*it);

    PyObject *kobj = PyLong_FromLongLong(it.key());
    PyObject *tobj = sipConvertFromMappedType(t, qmap2, sipTransferObj);

    if (kobj == NULL || tobj == NULL || PyDict_SetItem(d, kobj, tobj) < 0)
    {
      Py_DECREF(d);

      if (kobj)
      {
        Py_DECREF(kobj);
      }

      if (tobj)
      {
        Py_DECREF(tobj);
      }
      else
      {
        delete t;
      }

      return NULL;
    }

    Py_DECREF(kobj);
    Py_DECREF(tobj);
  }

  return d;
%End

%ConvertToTypeCode
  PyObject *kobj, *tobj, *kobj2, *tobj2;
  Py_ssize_t i = 0;

  //TODO: it works using SIP
#if (SIP_VERSION >= 0x041200)
  const sipMappedType* qmap2 = sipFindMappedType("QMap<int, TYPE>");
#endif

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyDict_Check(sipPy))
      return 0;

    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
      if (!PyDict_Check(tobj))
        return 0;

#if (SIP_VERSION >= 0x041200)
      if (!sipCanConvertToMappedType(tobj, qmap2, SIP_NOT_NONE))
        return 0;
#else
      Py_ssize_t j = 0;
      while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
      {
        if (!sipCanConvertToType(tobj2, sipType_TYPE, SIP_NOT_NONE))
          return 0;
      }
#endif
    }
    return 1;
  }

  QMap<qint64, QMap<int, TYPE> > *qm = new QMap<qint64, QMap<int, TYPE> >;

  while (PyDict_Next(sipPy, &i, &kobj, &tobj))
  {
    qint64 k = PyLong_AsLongLong(kobj);

#if (SIP_VERSION >= 0x041200)
    // TODO: search for the minimum SIP version this code works on, it works
    // on SIP 4.13.3 (GS). See #else to know why the version check is needed.

    int state;

    TYPE* t = reinterpret_cast<TYPE*>(sipConvertToMappedType(tobj, qmap2, sipTransferObj,SIP_NOT_NONE,&state,sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseMappedType(t, qmap2, state);
      delete qm;
      return 0;
    }

    qm.insert(k, *t);
    sipReleaseMappedType(t, qmap2, state);
#else
    // using sipConvertToMappedType to convert directly to QMap<int, TYPE> doesn't work
    // and ends with a segfault

    QMap<int, TYPE> qm2;

    Py_ssize_t j = 0;
    while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
    {
      int k2 = PyInt_AsLong(kobj2);
      int state;

      TYPE* t2 = reinterpret_cast<TYPE*>(sipConvertToType(tobj2, sipType_TYPE, sipTransferObj,SIP_NOT_NONE,&state,sipIsErr));

      if (*sipIsErr)
      {
        sipReleaseType(t2, sipType_TYPE, state);
        delete qm;
        return 0;
      }

      qm2.insert(k2, *t2);
      sipReleaseType(t2, sipType_TYPE, state);
    }
    qm->insert(k, qm2);
#endif
  }

  *sipCppPtr = qm;
  return sipGetState(sipTransferObj);
%End
};


//
// copied from PyQt4 QMap<int, TYPE> and adapted to qint64
//

// QMap<qint64, TYPE> is implemented as a Python dictionary.
template<TYPE>
%MappedType QMap<qint64, TYPE> /DocType="dict-of-qint64-TYPE"/
{
%TypeHeaderCode
#include <qmap.h>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<qint64, TYPE>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        TYPE *t = new TYPE(i.value());

        PyObject *kobj = PyLong_FromLongLong(i.key());
        //PyObject *kobj = SIPLong_FromLong(i.key());
        PyObject *tobj = sipConvertFromNewType(t, sipType_TYPE, sipTransferObj);

        if (kobj == NULL || tobj == NULL || PyDict_SetItem(d, kobj, tobj) < 0)
        {
            Py_DECREF(d);

            if (kobj)
            {
                Py_DECREF(kobj);
            }

            if (tobj)
            {
                Py_DECREF(tobj);
            }
            else
            {
                delete t;
            }

            return NULL;
        }

        Py_DECREF(kobj);
        Py_DECREF(tobj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *kobj, *tobj;
    SIP_SSIZE_T i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &kobj, &tobj))
            if (!sipCanConvertToType(tobj, sipType_TYPE, SIP_NOT_NONE))
                return 0;

        return 1;
    }

    QMap<qint64, TYPE> *qm = new QMap<qint64, TYPE>;

    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
        int state;
        //, k = SIPLong_AsLong(kobj);
        qint64 k = PyLong_AsLongLong(kobj);
        TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(tobj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t, sipType_TYPE, state);

            delete qm;
            return 0;
        }

        qm->insert(k, *t);

        sipReleaseType(t, sipType_TYPE, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};







%MappedType QMap<QString, QVariant::Type>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<QString, QVariant::Type>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        QString *t1 = new QString(i.key());

        PyObject *t1obj = sipConvertFromNewType(t1, sipType_QString, sipTransferObj);
        PyObject *t2obj = PyInt_FromLong( (long) i.value() );

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

	    if (t1obj) {
	      Py_DECREF(t1obj);
	    } else {
	      delete t1;
	    }

	    if (t2obj) {
	      Py_DECREF(t2obj);
	    }

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif


    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!sipCanConvertToType(t1obj, sipType_QString, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<QString, QVariant::Type> *qm = new QMap<QString, QVariant::Type>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        QString *t1 = reinterpret_cast<QString *>(sipConvertToType(t1obj, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
        QVariant::Type t2 = (QVariant::Type) PyInt_AsLong(t1obj);

        if (*sipIsErr)
        {
            sipReleaseType(t1, sipType_QString, state);
            delete qm;
            return 0;
        }

        qm->insert(*t1, t2);

        sipReleaseType(t1, sipType_QString, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};



%MappedType QMap<QString, int>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<QString, int>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        QString *t1 = new QString(i.key());

        PyObject *t1obj = sipConvertFromNewType(t1, sipType_QString, sipTransferObj);
        PyObject *t2obj = PyInt_FromLong( (long) i.value() );

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

	    if (t1obj) {
	      Py_DECREF(t1obj);
	    } else {
	      delete t1;
	    }

	    if (t2obj) {
	      Py_DECREF(t2obj);
	    }

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif


    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!sipCanConvertToType(t1obj, sipType_QString, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<QString, int> *qm = new QMap<QString, int>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        QString *t1 = reinterpret_cast<QString *>(sipConvertToType(t1obj, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
        int t2 = PyInt_AsLong(t1obj);

        if (*sipIsErr)
        {
            sipReleaseType(t1, sipType_QString, state);
            delete qm;
            return 0;
        }

        qm->insert(*t1, t2);

        sipReleaseType(t1, sipType_QString, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};



template<TYPE1, TYPE2>
%MappedType QMap<TYPE1, TYPE2*>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<TYPE1, TYPE2*>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        TYPE1 *t1 = new TYPE1(i.key());
        TYPE2 *t2 = i.value();

        PyObject *t1obj = sipConvertFromNewType(t1, sipType_TYPE1, sipTransferObj);
        PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE2, sipTransferObj);

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);
            else
                delete t1;

            if (t2obj)
                Py_DECREF(t2obj);
            else
                delete t2;

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!sipCanConvertToType(t1obj, sipType_TYPE1, SIP_NOT_NONE))
                return 0;

            if (!sipCanConvertToType(t2obj, sipType_TYPE2, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<TYPE1, TYPE2*> *qm = new QMap<TYPE1, TYPE2*>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state1, state2;

        TYPE1 *t1 = reinterpret_cast<TYPE1 *>(sipConvertToType(t1obj, sipType_TYPE1, sipTransferObj, SIP_NOT_NONE, &state1, sipIsErr));
        TYPE2 *t2 = reinterpret_cast<TYPE2 *>(sipConvertToType(t2obj, sipType_TYPE2, sipTransferObj, SIP_NOT_NONE, &state2, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t1, sipType_TYPE1, state1);
            sipReleaseType(t2, sipType_TYPE2, state2);

            delete qm;
            return 0;
        }

        qm->insert(*t1, t2);

        sipReleaseType(t1, sipType_TYPE1, state1);
        sipReleaseType(t2, sipType_TYPE2, state2);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};



template<double, TYPE>
%MappedType QMap<double, TYPE>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<double, TYPE>::iterator i;

    for (i = sipCpp->begin(); i != sipCpp->end(); ++i)
    {
        PyObject *t1obj = PyFloat_FromDouble(i.key());
        TYPE* t2 = &i.value();
        PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE, sipTransferObj);

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);

            if (t2obj)
                Py_DECREF(t2obj);

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);
   }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!PyFloat_Check(t1obj))
                return 0;

            if (!sipCanConvertToType(t2obj, sipType_TYPE, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<double, TYPE> *qm = new QMap<double, TYPE>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        double k = PyFloat_AsDouble(t1obj);
        TYPE *t2 = reinterpret_cast<TYPE *>(sipConvertToType(t2obj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t2, sipType_TYPE, state);
            delete qm;
            return 0;
        }

        qm->insert(k, *t2);

        sipReleaseType(t2, sipType_TYPE, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};



template<double, TYPE2>
%MappedType QMultiMap<double, TYPE2>
{
%TypeHeaderCode
#include <QMultiMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMultiMap<double, TYPE2>::iterator i = sipCpp->begin();

    while (i != sipCpp->end())
    {

      const double t1 = i.key();
      TYPE2 * t2 = &i.value();
      PyObject *t1obj = PyFloat_FromDouble(t1);
      PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE2, sipTransferObj);
      if (PyDict_GetItem(d, t1obj) == NULL) {
       PyObject *lst = PyList_New(0);
       PyDict_SetItem(d, t1obj, lst);
       if (lst)
       {
         Py_DECREF(lst);
       }
      }

      if (t1obj == NULL || t2obj == NULL ||
         PyList_Append(PyDict_GetItem(d, t1obj), t2obj) < 0)
        {
         Py_DECREF(d);
         if (t1obj)
         {
           Py_DECREF(t1obj);
         }

         if (t2obj)
         {
           Py_DECREF(t2obj);
         }

         return NULL;
        }
        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
         for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i) {
           if (!sipCanConvertToType(PyList_GET_ITEM(t2obj, i),
                                        sipType_TYPE2, SIP_NOT_NONE))
             return 0;
         }
        }

        return 1;
    }

    QMultiMap<double, TYPE2> *qm = new QMultiMap<double, TYPE2>;
    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state2;
       double k = PyFloat_AsDouble(t1obj);
       for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i) {
         TYPE2 *t2 =
           reinterpret_cast<TYPE2 *>(sipConvertToType(PyList_GET_ITEM(t2obj, i),
                                                          sipType_TYPE2,
                                                          sipTransferObj,
                                                          SIP_NOT_NONE,
                                                          &state2,
                                                          sipIsErr));

         if (*sipIsErr)
           {
             sipReleaseType(t2, sipType_TYPE2, state2);

             delete qm;
             return 0;
           }

         qm->insert(k, *t2);

         sipReleaseType(t2, sipType_TYPE2, state2);
       }
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


template<int, TYPE2*>
%MappedType QMultiMap<int, TYPE2*>
{
%TypeHeaderCode
#include <QMultiMap>
%End

%ConvertFromTypeCode
    // Convert to Python: create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
    {
        return NULL;
    }

    // Set the dictionary elements.
    QMultiMap<int, TYPE2*>::iterator i = sipCpp->begin();

    while (i != sipCpp->end())
    {

      const int t1 = i.key();
      TYPE2 * t2 = i.value();
      PyObject *t1obj = PyInt_FromSize_t(t1);
      PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE2, sipTransferObj);
      if (PyDict_GetItem(d, t1obj) == NULL)
      {
        PyObject *lst = PyList_New(0);
        PyDict_SetItem(d, t1obj, lst);
        if (lst)
        {
           Py_DECREF(lst);
        }
      }

      if (t1obj == NULL || t2obj == NULL ||
         PyList_Append(PyDict_GetItem(d, t1obj), t2obj) < 0)
      {
         Py_DECREF(d);
         if (t1obj)
         {
           Py_DECREF(t1obj);
         }

         if (t2obj)
         {
           Py_DECREF(t2obj);
         }

         return NULL;
       }
       Py_DECREF(t1obj);
       Py_DECREF(t2obj);

       ++i;
    }

    return d;
%End

%ConvertToTypeCode
    // Convert from Python:
    PyObject *t1obj, *t2obj;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
         for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i) {
           if (!sipCanConvertToType(PyList_GET_ITEM(t2obj, i),
                                        sipType_TYPE2, SIP_NOT_NONE))
             return 0;
         }
        }

        return 1;
    }

    QMultiMap<int, TYPE2*> *qm = new QMultiMap<int, TYPE2*>;
    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
       int state2;
       int k = (int) PyInt_AsLong(t1obj);
       for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i)
       {
           TYPE2 *t2 =
           reinterpret_cast<TYPE2 *>(sipConvertToType(PyList_GET_ITEM(t2obj, i),
                                                          sipType_TYPE2,
                                                          sipTransferObj,
                                                          SIP_NOT_NONE,
                                                          &state2,
                                                          sipIsErr));

         if (*sipIsErr)
         {
             sipReleaseType(t2, sipType_TYPE2, state2);

             delete qm;
             return 0;
         }

         qm->insert(k, t2);

         sipReleaseType(t2, sipType_TYPE2, state2);
       }
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


%MappedType QList < QPair< QString, QList<QString> > >
{
%TypeHeaderCode
#include <QPair>
#include <QList>
%End

%ConvertFromTypeCode
//convert map to a python dictionary
    PyObject *d;

    if ((d = PyList_New( sipCpp->size() )) == NULL)
      return NULL;

    for ( int i = 0; i<sipCpp->size(); i++ )
    {
      PyObject *p;
      if ((p = PyList_New(2) ) == NULL)
      {
	Py_DECREF(d);
	return NULL;
      }

      PyObject *l;
      if ((l = PyList_New( sipCpp->at(i).second.size() )) == NULL)
      {
	Py_DECREF(d);
	Py_DECREF(p);
	return NULL;
      }

      for( int j = 0; j<sipCpp->at(i).second.size(); j++ )
      {
        PyObject *t1obj = sipConvertFromNewType(new QString(sipCpp->at(i).second.at(j)), sipType_QString, sipTransferObj);
        PyList_SetItem( l, j, t1obj);
      }

      PyObject *t1obj = sipConvertFromNewType(new QString(sipCpp->at(i).first), sipType_QString, sipTransferObj);
      PyList_SetItem( p, 0, t1obj );
      PyList_SetItem( p, 1, l);

      PyList_SetItem( d, i, p );
    }

    return d;
%End
%ConvertToTypeCode
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif
    QList < QPair< QString, QList<QString> > > *qm = new QList < QPair< QString, QList<QString> > >;

    for ( i = 0; i < PyList_GET_SIZE(sipPy); i++ )
    {
      PyObject *sipPair = PyList_GetItem( sipPy, i );
      PyObject *sipKey  = PyList_GetItem( sipPair, 0 );
      PyObject *sipList = PyList_GetItem( sipPair, 1 );

      QList< QString > l;
      int state;
      int j;
      for ( j = 0; j < PyList_GET_SIZE( sipList ); j++ )
      {
        PyObject *sipString = PyList_GetItem( sipList, j );
        QString *t1 = reinterpret_cast<QString *>(sipConvertToType(sipString, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
	l << *t1;
        sipReleaseType(t1, sipType_QString, state);
      }

      QString *t1 = reinterpret_cast<QString *>(sipConvertToType(sipKey, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      qm->append( qMakePair( *t1, l ) );
      sipReleaseType(t1, sipType_QString, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QList < QPair< QgsVectorLayer *, int > >
{
%TypeHeaderCode
#include <QPair>
#include <QList>
%End

%ConvertFromTypeCode
//convert map to a python dictionary
    PyObject *d;

    if ((d = PyList_New( sipCpp->size() )) == NULL)
      return NULL;

    for ( int i = 0; i<sipCpp->size(); i++ )
    {
      PyObject *p;
      if ((p = PyList_New(2) ) == NULL)
      {
	Py_DECREF(d);
	return NULL;
      }

      PyObject *t1obj = sipConvertFromNewType(sipCpp->at(i).first, sipType_QgsVectorLayer, sipTransferObj);
      PyObject *t2obj = PyInt_FromLong( (long) sipCpp->at(i).second );
      PyList_SetItem( p, 0, t1obj );
      PyList_SetItem( p, 1, t2obj );

      PyList_SetItem( d, i, p );
    }

    return d;
%End
%ConvertToTypeCode
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif
    QList < QPair< QgsVectorLayer *, int > > *qm = new QList< QPair< QgsVectorLayer *, int > >;

    for ( i = 0; i < PyList_GET_SIZE(sipPy); i++ )
    {
	  int state;

      PyObject *sipPair = PyList_GetItem( sipPy, i );
      PyObject *sipLayer = PyList_GetItem( sipPair, 0 );
      PyObject *sipIdx = PyList_GetItem( sipPair, 1 );

      QgsVectorLayer *t1 = reinterpret_cast<QgsVectorLayer *>(sipConvertToType(sipLayer, sipType_QgsVectorLayer, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      int idx = PyLong_AsLongLong(sipIdx);
      qm->append( qMakePair<QgsVectorLayer *, int>( t1, idx ) );
      sipReleaseType(t1, sipType_QgsVectorLayer, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


template <TYPE>
%MappedType QVector< TYPE* >
{
%TypeHeaderCode
#include <QVector>
%End

%ConvertFromTypeCode
    // Create the list
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return NULL;

    // Set the dictionary elements.
    for( int i = 0; i < sipCpp->size(); i++ )
    {
        TYPE *t = sipCpp->at(i);
        PyObject *tobj = sipConvertFromType(t, sipType_TYPE, sipTransferObj);

        if (tobj == NULL || PyList_SetItem(l, i, tobj) < 0)
        {
            Py_DECREF(l);

            if (tobj)
                Py_DECREF(tobj);
            else
                delete t;

            return NULL;
        }

        Py_DECREF(tobj);
    }

    return l;
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
      if (!PyList_Check(sipPy))
        return 0;

      for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      {
        if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, SIP_NOT_NONE))
          return 0;
      }

      return 1;
    }

    QVector<TYPE*> *v = new QVector<TYPE*>( PyList_GET_SIZE(sipPy) );

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
      int state;

      TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

      if (*sipIsErr)
      {
        sipReleaseType(t, sipType_TYPE, state);
        delete v;
        return 0;
      }

      v->replace( i, t );

      sipReleaseType(t, sipType_TYPE, state);
    }

    *sipCppPtr = v;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QMap<qint64, QgsFeature*>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode

//convert map to a python dictionary
    PyObject *d;

    if ((d = PyDict_New()) == NULL)
      return NULL;

    for (QMap<qint64, QgsFeature*>::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it)
    {
        QgsFeature* oobj = new QgsFeature(*it.value());

        PyObject* keyobj = PyInt_FromLong(it.key());
        PyObject* pyOobj = sipConvertFromType(oobj, sipType_QgsFeature, sipTransferObj);
        PyDict_SetItem(d, keyobj, pyOobj);

        if(pyOobj == NULL || keyobj == NULL || PyDict_SetItem(d, keyobj, pyOobj) < 0)
        {
            Py_DECREF(d);

            if (pyOobj)
            {
                Py_DECREF(pyOobj);
            }

            if (keyobj)
            {
                Py_DECREF(keyobj);
            }
            return NULL;
          }
        Py_DECREF(pyOobj);
        Py_DECREF(keyobj);
        }
    return d;

%End
%ConvertToTypeCode
 PyObject *t1obj, *t2obj;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t i = 0;
#else
    int i = 0;
#endif

    QMap<qint64, QgsFeature*> *qm = new QMap<qint64, QgsFeature*>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;
        qint64 t1 = PyLong_AsLongLong(t1obj);
        QgsFeature* t2 = reinterpret_cast<QgsFeature*>(sipConvertToType(t2obj, sipType_QgsFeature, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t2, sipType_QgsFeature, state);
            delete qm;
            return 0;
        }

        qm->insert(t1, t2);

        sipReleaseType(t2, sipType_QgsFeature, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

%If (QVECTORINT_CONVERSION)
// QVector<int> is implemented as a Python list of integers.
%MappedType QVector<int> /DocType="list-of-int"/
{
%TypeHeaderCode
#include <qvector.h>
%End

%ConvertFromTypeCode
    // Create the list.
    PyObject *l;

    if ((l = PyList_New(sipCpp->size())) == NULL)
        return NULL;

    // Set the list elements.
    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *pobj;

        // Convert to a Python long to make sure it doesn't get interpreted as
        // a signed value.
        if ((pobj = SIPLong_FromLong(sipCpp->value(i))) == NULL)
        {
            Py_DECREF(l);

            return NULL;
        }

        PyList_SET_ITEM(l, i, pobj);
    }

    return l;
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
        return PyList_Check(sipPy);

    QVector<int> *qv = new QVector<int>;

    for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
        qv->append(SIPLong_AsLong(PyList_GET_ITEM(sipPy, i)));

    *sipCppPtr = qv;

    return sipGetState(sipTransferObj);
%End
};
%End
