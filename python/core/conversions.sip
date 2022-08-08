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
- NULL QVariant which is missing in PyQt5 with sip.enableautoconversion
*/

// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<QgsMapLayerType>
        /TypeHintIn="Iterable[QgsMapLayerType]",
        TypeHintOut="List[QgsMapLayerType]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_QgsMapLayerType);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<QgsMapLayerType> *ql = new QList<QgsMapLayerType>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_QgsMapLayerType);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'QgsMapLayerType' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<QgsMapLayerType>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};



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

  const sipTypeDef *qvector_type = sipFindType("QVector<TYPE>");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<TYPE> *t = new QVector<TYPE>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromNewType(t, qvector_type, NULL)) == NULL)
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
  const sipTypeDef *qvector_type = sipFindType("QVector<TYPE>");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QVector< QVector<TYPE> > *ql = new QVector< QVector<TYPE> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<TYPE> *t = reinterpret_cast< QVector<TYPE> * >(sipConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, qvector_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseType(t, qvector_type, state);
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

  const sipTypeDef *qvector_type = sipFindType("QVector<QVector<TYPE> >");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<QVector<TYPE> > *t = new QVector<QVector<TYPE> >(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromNewType(t, qvector_type, NULL)) == NULL)
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

  const sipTypeDef *qvector_type = sipFindType("QVector<QVector<TYPE> >");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QVector< QVector< QVector<TYPE> > > *ql = new QVector< QVector< QVector<TYPE> > >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<QVector<TYPE> > *t = reinterpret_cast< QVector< QVector<TYPE> > * >(sipConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, qvector_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseType(t, qvector_type, state);
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

  const sipTypeDef *qlist_type = sipFindType("QList<TYPE>");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QList<TYPE> *t = new QList<TYPE>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromNewType(t, qlist_type, NULL)) == NULL)
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
  const sipTypeDef *qlist_type = sipFindType("QList<TYPE>");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), qlist_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QList< QList<TYPE> > *ql = new QList< QList<TYPE> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QList<TYPE> *t = reinterpret_cast< QList<TYPE> * >(sipConvertToType(PyList_GET_ITEM(sipPy, i), qlist_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, qlist_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseType(t, qlist_type, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};


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

    if ((tobj = PyLong_FromLong(*it)) == NULL)
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
    qset->insert(PyLong_AsLong(PyList_GET_ITEM(sipPy, i)));
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End
};

%MappedType QList<long>
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
  QList<long>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = PyLong_FromLong(*it)) == NULL)
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

  QList<long> *qlist = new QList<long>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    *qlist << PyLong_AsLong(PyList_GET_ITEM(sipPy, i));
  }

  *sipCppPtr = qlist;
  return sipGetState(sipTransferObj);
%End
};

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

  const sipTypeDef *qmap2 = sipFindType("QMap<int, TYPE>");

  // Set the list elements.
  for (QMap<qint64, QMap<int, TYPE> >::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it)
  {
    QMap<int, TYPE> *t = new QMap<int, TYPE>(*it);

    PyObject *kobj = PyLong_FromLongLong(it.key());
    PyObject *tobj = sipConvertFromNewType(t, qmap2, sipTransferObj);

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

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyDict_Check(sipPy))
      return 0;

    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
      if (!PyDict_Check(tobj))
        return 0;

      Py_ssize_t j = 0;
      while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
      {
        if (!sipCanConvertToType(tobj2, sipType_TYPE, SIP_NOT_NONE))
          return 0;
      }
    }
    return 1;
  }

  QMap<qint64, QMap<int, TYPE> > *qm = new QMap<qint64, QMap<int, TYPE> >;

  while (PyDict_Next(sipPy, &i, &kobj, &tobj))
  {
    qint64 k = PyLong_AsLongLong(kobj);

    // using sipConvertToType to convert directly to QMap<int, TYPE> doesn't work
    // and ends with a segfault

    QMap<int, TYPE> qm2;

    Py_ssize_t j = 0;
    while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
    {
      int k2 = PyLong_AsLong(kobj2);
      int state;

      TYPE *t2 = reinterpret_cast<TYPE*>(sipConvertToType(tobj2, sipType_TYPE, sipTransferObj,SIP_NOT_NONE,&state,sipIsErr));

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
%MappedType QMap<qint64, TYPE>
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

template<TYPE>
%MappedType QList<QMap<int, TYPE>>
{
%TypeHeaderCode
#include <qmap.h>
#include <qlist.h>
%End

%ConvertFromTypeCode

    // Create the list
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return NULL;

    // Set the list elements.
    QList<QMap<int, TYPE>>::const_iterator it = sipCpp->constBegin();
    int i = 0;
    while (it != sipCpp->constEnd())
    {
        // Create the dictionary.
        PyObject *d = PyDict_New();
        if (!d)
        {
            Py_DECREF(l);
            return NULL;
        }

        QMap<int, TYPE>::const_iterator mapIt = it->constBegin();
        while (mapIt != it->constEnd())
        {
            TYPE *t = new TYPE(mapIt.value());

            PyObject *kobj = PyLong_FromLong(mapIt.key());
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

                Py_DECREF(l);
                return NULL;
            }

            Py_DECREF(kobj);
            Py_DECREF(tobj);

            ++mapIt;
        }

        PyList_SET_ITEM(l, i, d);
        ++i;
        ++it;
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *kobj, *tobj;
    SIP_SSIZE_T i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyList_Check(sipPy))
          return 0;

        for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
        {
            if (!PyDict_Check(PyList_GET_ITEM(sipPy, i)))
                return 0;

            Py_ssize_t j = 0;
            while (PyDict_Next(PyList_GET_ITEM(sipPy, i), &j, &kobj, &tobj))
                if (!sipCanConvertToType(tobj, sipType_TYPE, SIP_NOT_NONE))
                    return 0;
        }

        return 1;
    }

    QList< QMap<int, TYPE> >* ql = new QList< QMap<int, TYPE> >;
    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
        Py_ssize_t j = 0;
        QMap<int, TYPE> qm;
        while (PyDict_Next(PyList_GET_ITEM(sipPy, i), &j, &kobj, &tobj))
        {
            int state;
            int k = PyLong_AsLong(kobj);
            TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(tobj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

            if (*sipIsErr)
            {
                sipReleaseType(t, sipType_TYPE, state);

                delete ql;
                return 0;
            }

            qm.insert(k, *t);
            sipReleaseType(t, sipType_TYPE, state);
        }
        ql->append( qm );
    }

    *sipCppPtr = ql;

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
        PyObject *t2obj = PyLong_FromLong( (long) i.value() );

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
    Py_ssize_t i = 0;

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
        int t2 = PyLong_AsLong(t1obj);

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
    Py_ssize_t i = 0;

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
        TYPE *t2 = &i.value();
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
    Py_ssize_t i = 0;

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


%MappedType QHash<double, double>
{
%TypeHeaderCode
#include <QHash>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QHash<double, double>::iterator i;

    for (i = sipCpp->begin(); i != sipCpp->end(); ++i)
    {
        PyObject *t1obj = PyFloat_FromDouble(i.key());
        PyObject *t2obj = PyFloat_FromDouble(i.value());

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
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!PyFloat_Check(t1obj))
                return 0;

           if (!PyFloat_Check(t2obj))
                return 0;
        }

        return 1;
    }

    QHash<double, double> *qm = new QHash<double, double>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        double k = PyFloat_AsDouble(t1obj);
        double v = PyFloat_AsDouble(t2obj);

        qm->insert(k, v);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QMap<double, double>
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
    QMap<double, double>::iterator i;

    for (i = sipCpp->begin(); i != sipCpp->end(); ++i)
    {
        PyObject *t1obj = PyFloat_FromDouble(i.key());
        PyObject *t2obj = PyFloat_FromDouble(i.value());

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
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!PyFloat_Check(t1obj))
                return 0;

           if (!PyFloat_Check(t2obj))
                return 0;
        }

        return 1;
    }

    QMap<double, double> *qm = new QMap<double, double>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        double k = PyFloat_AsDouble(t1obj);
        double v = PyFloat_AsDouble(t2obj);

        qm->insert(k, v);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

template<TYPE2>
%MappedType QMap<QString, QList<TYPE2> >
{
%TypeHeaderCode
#include <QMap>
#include <QList>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<QString, QList< TYPE2 > >::const_iterator i;

    for (i = sipCpp->constBegin(); i != sipCpp->constEnd(); ++i)
    {
        QString *t1 = new QString(i.key());
        PyObject *t1obj = sipConvertFromNewType(t1, sipType_QString, sipTransferObj);

        // build list for dictionary value
        QList< TYPE2 > sourceList = i.value();
        PyObject *t2list = PyList_New( sourceList.size() );
        if ( t2list )
        {
            for ( int j = 0; j < sourceList.size(); j++ )
            {
                TYPE2 *t = new TYPE2(sourceList.at(j));
                PyObject *lobj = sipConvertFromNewType(t, sipType_TYPE2, sipTransferObj);
                PyList_SetItem( t2list, j, lobj );
            }
        }

        if (t1obj == NULL || t2list == NULL || PyDict_SetItem(d, t1obj, t2list) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);

            if (t2list)
                Py_DECREF(t2list);

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2list);
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;


    const sipTypeDef *qlist_type = sipFindType("QList<TYPE2>");


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

    QMap<QString, QList< TYPE2 > > *qm = new QMap<QString, QList< TYPE2 > >;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        QString *t1 = reinterpret_cast<QString *>(sipConvertToType(t1obj, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    QList<TYPE2> *t2 = reinterpret_cast< QList<TYPE2> * >(sipConvertToType(t2obj, qlist_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
        if (*sipIsErr)
        {
            sipReleaseType(t2, sipType_TYPE2, state);
            delete qm;
            return 0;
        }

        if ( t2 )
          qm->insert(*t1, *t2);
        else
          qm->insert(*t1, QList<TYPE2>() );

        sipReleaseType(t1, sipType_QString, state);

        sipReleaseType(t2, sipType_TYPE2, state);
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
      TYPE2 *t2 = &i.value();
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
    Py_ssize_t i = 0;

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
      TYPE2 *t2 = i.value();
      PyObject *t1obj = PyLong_FromSize_t(t1);
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
    Py_ssize_t i = 0;

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
       int k = (int) PyLong_AsLong(t1obj);
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


template <TYPE>
%MappedType QMap< QPair< QString, QString>, TYPE >
{
%TypeHeaderCode
#include <QPair>
#include <QMap>
%End

%ConvertFromTypeCode
//convert map to a python dictionary
  PyObject *d;

  if ( ( d = PyDict_New() ) == NULL )
    return NULL;

  for ( auto it = sipCpp->constBegin(); it != sipCpp->constEnd(); ++ it )
  {
    PyObject *keyobj;
    if ( ( keyobj = PyTuple_New( 2 ) ) == NULL )
    {
      Py_DECREF( d );
      return NULL;
    }

    TYPE *t = new TYPE(it.value());
    PyObject *tobj = sipConvertFromNewType(t, sipType_TYPE, sipTransferObj);
    if ( tobj == NULL )
    {
        Py_DECREF(d);
        delete t;
        return NULL;
    }

    // build key
    PyObject *k1obj = sipConvertFromNewType( new QString( it.key().first ), sipType_QString, sipTransferObj );
    PyTuple_SetItem( keyobj, 0, k1obj );
    PyObject *k2obj = sipConvertFromNewType( new QString( it.key().second ), sipType_QString, sipTransferObj );
    PyTuple_SetItem( keyobj, 1, k2obj );

    if(keyobj == NULL || tobj == NULL || PyDict_SetItem(d, keyobj, tobj) < 0)
    {
        Py_DECREF(d);

        if (keyobj)
        {
            Py_DECREF(keyobj);
        }

        if (tobj)
        {
            Py_DECREF(tobj);
        }
        return NULL;
      }

    Py_DECREF(keyobj);
    Py_DECREF(tobj);
  }

  return d;
%End

%ConvertToTypeCode
  Py_ssize_t i = 0;
  PyObject *kobj, *tobj;

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

  PyObject *t1obj, *t2obj;
  QMap< QPair< QString, QString>, TYPE > *qm = new QMap< QPair< QString, QString>, TYPE >;

  int state;
  while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
  {
      PyObject *sipKeyFirst  = PyTuple_GetItem( t1obj, 0 );
      PyObject *sipKeySecond = PyTuple_GetItem( t1obj, 1 );
      QString *k1 = reinterpret_cast<QString *>(sipConvertToType(sipKeyFirst, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      if (*sipIsErr)
      {
          sipReleaseType(k1, sipType_QString, state);
          delete qm;
          return 0;
      }

      QString *k2 = reinterpret_cast<QString *>(sipConvertToType(sipKeySecond, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      if (*sipIsErr)
      {
          sipReleaseType(k1, sipType_QString, state);
          sipReleaseType(k2, sipType_QString, state);
          delete qm;
          return 0;
      }

      TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(t2obj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      if (*sipIsErr)
      {
          sipReleaseType(t, sipType_TYPE, state);

          delete qm;
          return 0;
      }

      qm->insert( qMakePair( *k1,*k2 ), *t );
      sipReleaseType(k1, sipType_QString, state);
      sipReleaseType(k2, sipType_QString, state);
      sipReleaseType(t, sipType_TYPE, state);
  }

  *sipCppPtr = qm;

  return sipGetState( sipTransferObj );
%End
};

%If (VECTOR_MAPPED_TYPE)
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
            Py_DECREF(tobj);
            Py_DECREF(l);
            return NULL;
        }
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
%End


%MappedType QPointer< QgsMapLayer >
{
%TypeHeaderCode
#include <QPointer>
%End

%ConvertFromTypeCode
    QgsMapLayer *t = sipCpp->data();
    PyObject *tobj = sipConvertFromType(t, sipType_QgsMapLayer, sipTransferObj);
    if (tobj == NULL)
    {
        Py_DECREF(tobj);
        return NULL;
    }
    return tobj;
%End

%ConvertToTypeCode
    int state;

    QgsMapLayer *t = reinterpret_cast<QgsMapLayer *>(sipConvertToType(sipPy, sipType_QgsMapLayer, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, sipType_QgsMapLayer, state);
      return 0;
    }
    QPointer<QgsMapLayer> *v = new QPointer<QgsMapLayer>( t );
    sipReleaseType(t, sipType_QgsMapLayer, state);

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
        QgsFeature *oobj = new QgsFeature(*it.value());

        PyObject *keyobj = PyLong_FromLong(it.key());
        PyObject *pyOobj = sipConvertFromType(oobj, sipType_QgsFeature, sipTransferObj);
        if(pyOobj == NULL || keyobj == NULL || PyDict_SetItem(d, keyobj, pyOobj) < 0)
        {
            Py_DECREF(d);

            if (pyOobj)
            {
                Py_DECREF(pyOobj);
            }
            else
            {
                delete oobj;
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
    Py_ssize_t i = 0;

    QMap<qint64, QgsFeature*> *qm = new QMap<qint64, QgsFeature*>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;
        qint64 t1 = PyLong_AsLongLong(t1obj);
        QgsFeature *t2 = reinterpret_cast<QgsFeature*>(sipConvertToType(t2obj, sipType_QgsFeature, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

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

// QList<QgsField> is implemented as a Python list of QgsField.
%MappedType QList<QgsField>
{
%TypeHeaderCode
#include <qgsfield.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        QgsField *t = new QgsField(sipCpp->at(i));
        PyObject *tobj = sipConvertFromNewType(t, sipType_QgsField, sipTransferObj);

        if (!tobj)
        {
            delete t;
            Py_DECREF(l);

            return 0;
        }

        PyList_SET_ITEM(l, i, tobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        Py_XDECREF(iter);

        return (iter
#if PY_MAJOR_VERSION < 3
                && !PyString_Check(sipPy)
#endif
                && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<QgsField> *ql = new QList<QgsField>;

    for (SIP_SSIZE_T i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int state;
        QgsField *t = reinterpret_cast<QgsField *>( sipForceConvertToType(itm, sipType_QgsField, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            PyErr_Format(PyExc_TypeError,
                    "index %ld has type '%s' but 'QgsField' is expected",
                    (long) i, Py_TYPE(itm)->tp_name);

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);

            return 0;
        }

        ql->append(*t);

        sipReleaseType(t, sipType_QgsField, state);
        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

%ModuleCode
bool null_from_qvariant_converter( const QVariant *varp, PyObject **objp )
{
  static bool sWatchDog = false;

  if ( sWatchDog )
    return false;

  // If we deal with a NULL QVariant (and it's not a QByteArray which properly
  // maps NULL values)
  // If there are more cases like QByteArray, we should consider using a allowlist
  // instead of a blocklist.
  if ( varp->isNull()
       && varp->type() != QVariant::ByteArray
       && varp->type() != QMetaType::VoidStar
       && varp->type() != QMetaType::Nullptr
       && varp->type() != QMetaType::QObjectStar )
  {
    if ( varp->type() == QVariant::UserType
         && varp->userType() == QMetaType::type("QgsLayoutItem*") )
    {
      return false;
    }

    sWatchDog = true;
    PyObject *vartype = sipConvertFromEnum( varp->type(), sipType_QVariant_Type );
    PyObject *args = PyTuple_Pack( 1, vartype );
    PyTypeObject *typeObj = sipTypeAsPyTypeObject( sipType_QVariant );
    *objp = PyObject_Call(( PyObject * )typeObj, args, nullptr );
    Py_DECREF(args);
    Py_DECREF(vartype);
    sWatchDog = false;
    return true;
  }
  else
  {
    return false;
  }
}
%End

// Mapped type for QList<QgsWkbTypes::GeometryType>.
%MappedType QList<QgsWkbTypes::GeometryType>
{
%TypeHeaderCode
#include <qgswkbtypes.h>
%End

%ConvertFromTypeCode
    // Create the list.
    PyObject *l;

    if ((l = PyList_New(sipCpp->size())) == NULL)
        return NULL;

    // Set the list elements.
    for (int i = 0; i < sipCpp->size(); ++i)
    {
       QgsWkbTypes::GeometryType e = sipCpp->at(i);
        PyObject *eobj;

        if ((eobj = sipConvertFromEnum(e, sipType_QgsWkbTypes_GeometryType)) == NULL)
        {
            Py_DECREF(l);

            return NULL;
        }

        PyList_SET_ITEM(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyList_Check(sipPy))
            return 0;

        for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
            if (PyObject_TypeCheck(PyList_GET_ITEM(sipPy, i), sipTypeAsPyTypeObject(sipType_QgsWkbTypes_GeometryType)))
                return 0;

        return 1;
    }

    QList<QgsWkbTypes::GeometryType> *ql = new QList<QgsWkbTypes::GeometryType>;

    for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
        long l = SIPLong_AsLong(PyList_GET_ITEM(sipPy, i));
        ql->append(static_cast<QgsWkbTypes::GeometryType>(l));
    }

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

%PostInitialisationCode  //#spellok

// Import the Chimera helper registration functions.
typedef bool ( *FromQVariantConverterFn )( const QVariant *, PyObject ** );
void (*register_from_qvariant_converter)(FromQVariantConverterFn);
register_from_qvariant_converter = (void (*)(FromQVariantConverterFn))sipImportSymbol("pyqt5_register_from_qvariant_convertor");  //#spellok
register_from_qvariant_converter(null_from_qvariant_converter);
%End
