/***************************************************************************
                          qgsserverinterface.sip

  Class defining the interface made available to QGIS Server plugins.
  -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
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

/**
 * \class QgsServerInterface
 * \brief Class defining the interface made available to server plugins.
 *
 * This class provides methods to access the request handler and
 * the capabilties cache. A method to read the environment
 * variables set in the main FCGI loop is also available.
 * Plugins can add listeners (instances of QgsServerFilter) with
 * a certain priority through the registerFilter( QgsServerFilter* , int) method.
 */


typedef QMultiMap<int, QgsServerFilter*> QgsServerFiltersMap;



template<int, TYPE2*>
%MappedType QMultiMap<int, TYPE2*>
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
    QMultiMap<int, TYPE2*>::iterator i = sipCpp->begin();

    while (i != sipCpp->end())
    {

      const int t1 = i.key();
      TYPE2 * t2 = i.value();
      PyObject *t1obj = PyInt_FromSize_t(t1);
      PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE2, sipTransferObj);
      ///////PyObject *t2obj = sipConvertFromNewType(t2, sipType_TYPE2, sipTransferObj);
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

    QMultiMap<int, TYPE2*> *qm = new QMultiMap<int, TYPE2*>;
    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state2;
       int k = (int) PyInt_AsLong(t1obj);
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

         qm->insert(k, t2);

         sipReleaseType(t2, sipType_TYPE2, state2);
       }
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};




class QgsServerInterface
{
%TypeHeaderCode
  #include "qgsserverinterface.h"
%End

  public:
    /** Returns the current request handler*/
    virtual QgsRequestHandler* requestHandler( ) = 0 /KeepReference/;
    /** Returns the capabilities cache*/
    virtual QgsCapabilitiesCache* capabiblitiesCache() = 0 /KeepReference/;
    // Tansfer ownership to avoid garbage collector to call dtor
    /** Register a filter with the given priority. The filter's requestReady()
     * and responseReady() methods will be called from the loop*/
    virtual void registerFilter( QgsServerFilter* filter /Transfer/, int priority = 0 ) = 0;
    /** Return an environment variable set by FCGI*/
    virtual QString getEnv(const QString& name ) const = 0;
    // Commented because of problems with typedef QgsServerFiltersMap, provided
    // methods to alter the filters map into QgsRequestHandler API
    virtual QgsServerFiltersMap filters( ) = 0;
    /** Returns the configFilePath as seen by the server, this value is only
     * available after requestReady has been called.*/
    virtual QString configFilePath( ) = 0;
    /** Set the config file path */
    virtual void setConfigFilePath( QString configFilePath) = 0;

private:
    /** Constructor */
    QgsServerInterface( );

};

