/*
This file contains code for conversion between various (often nested) mapped types
which are not wrapped by PyQt:
- QVector< QVector<TYPE> >
- QVector< QVector< QVector<TYPE> > >
- QSet<int>
- QSet<TYPE>
- QMap<int, QMap<int, TYPE> >

*/

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
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<TYPE> * t = reinterpret_cast< QVector<TYPE> * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseInstance(t, sipClass_TYPE, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseInstance(t, sipClass_TYPE, state);
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
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<QVector<TYPE> > * t = reinterpret_cast< QVector< QVector<TYPE> > * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseInstance(t, sipClass_TYPE, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseInstance(t, sipClass_TYPE, state);
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

    if ((tobj = sipConvertFromNewInstance(t, sipClass_TYPE, sipTransferObj)) == NULL)
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
      if (!sipCanConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, SIP_NOT_NONE))
        return 0;

    return 1;
  }

  QSet<TYPE> *qset = new QSet<TYPE>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    TYPE* t = reinterpret_cast<TYPE *>(sipConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseInstance(t, sipClass_TYPE, state);
      delete qset;
      return 0;
    }
    qset->insert(*t);
    sipReleaseInstance(t, sipClass_TYPE, state);
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End

};



template<TYPE>
%MappedType QMap<int, QMap<int, TYPE> >
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *d;

  if ((d = PyDict_New()) == NULL)
    return NULL;
      
  const sipMappedType* qmap2 = sipFindMappedType("QMap<int, TYPE>");
      
  // Set the list elements.
  for (QMap<int, QMap<int, TYPE> >::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it)
  {
    QMap<int, TYPE>* t = new QMap<int, TYPE>(*it);
    
    PyObject *kobj = PyInt_FromLong(it.key());
    PyObject *tobj = sipConvertFromMappedType(t, qmap2, sipTransferObj);

    if (kobj == NULL || tobj == NULL || PyDict_SetItem(d, kobj, tobj) < 0)
    {
      Py_DECREF(d);
      
      if (kobj)
        Py_DECREF(kobj);

      if (tobj)
        Py_DECREF(tobj);
      else
        delete t;

      return NULL;
    }
  
    Py_DECREF(kobj);
    Py_DECREF(tobj);
  }
 
  return d;
%End

%ConvertToTypeCode
  PyObject *kobj, *tobj, *kobj2, *tobj2;

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyDict_Check(sipPy))
      return 0;

    Py_ssize_t i = 0;
    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
      if (!PyDict_Check(tobj))
        return 0;
        
      Py_ssize_t j = 0;
      while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
      {
        if (!sipCanConvertToInstance(tobj2, sipClass_TYPE, SIP_NOT_NONE))
          return 0;
      }
      
    }
    return 1;
  }
  
  QMap<int, QMap<int, TYPE> > *qm = new QMap<int, QMap<int, TYPE> >;
  

  Py_ssize_t i = 0;
  while (PyDict_Next(sipPy, &i, &kobj, &tobj))
  {
    int k = PyInt_AsLong(kobj);
    
    // using sipConvertToMappedType to convert directly to QMap<int, TYPE> doesn't work
    // and ends with a segfault
    
    QMap<int, TYPE> qm2;
    
    Py_ssize_t j = 0;
    while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
    {
      int k2 = PyInt_AsLong(kobj2);
      int state;
      
      TYPE* fa = reinterpret_cast<TYPE*>(sipConvertToInstance(tobj2, sipClass_TYPE, sipTransferObj,SIP_NOT_NONE,&state,sipIsErr));
      
      if (*sipIsErr)
      {
        sipReleaseInstance(tobj2, sipClass_TYPE, state);
        delete qm;
        return 0;
      }
      
      qm2.insert(k2, *fa);
      sipReleaseInstance(tobj2, sipClass_TYPE, state);
    }
    qm->insert(k, qm2);
  }

  *sipCppPtr = qm;
  return sipGetState(sipTransferObj);
%End

};

