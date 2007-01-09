

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

    int i = 0;
    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
      if (!PyDict_Check(tobj))
        return 0;
        
      int j = 0;
      while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
      {
        if (!sipCanConvertToInstance(tobj2, sipClass_TYPE, SIP_NOT_NONE))
          return 0;
      }
      
    }
    return 1;
  }
  
  QMap<int, QMap<int, TYPE> > *qm = new QMap<int, QMap<int, TYPE> >;
  

  int i = 0;
  while (PyDict_Next(sipPy, &i, &kobj, &tobj))
  {
    int k = PyInt_AsLong(kobj);
    
    // using sipConvertToMappedType to convert directly to QMap<int, TYPE> doesn't work
    // and ends with a segfault
    
    QMap<int, TYPE> qm2;
    
    int j = 0;
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




class QgsVectorDataProvider : QgsDataProvider
{
%TypeHeaderCode
#include <qgsvectordataprovider.h>
%End

    public:

      // If you add to this, please also add to capabilitiesString()
      /**
       * enumeration with capabilities that providers might implement
       */
      enum Capability
      {
        NoCapabilities =               0,
        AddFeatures =                  1,
        DeleteFeatures =               2,
        ChangeAttributeValues =        4,
        AddAttributes =                8,
        DeleteAttributes =             16,
        SaveAsShapefile =              32,
        CreateSpatialIndex =           64,
        SelectAtId =                   128,
        ChangeGeometries =             256,
        SelectGeometryAtId =           512,
        RandomSelectGeometryAtId =     1024,
        SequentialSelectGeometryAtId = 2048
      };

      /**
       * Constructor of the vector provider
       * @param uri  uniform resource locator (URI) for a dataset
       */
      QgsVectorDataProvider(QString uri = QString());

      /**
       * Destructor
       */
      virtual ~QgsVectorDataProvider();

      /**
       * Returns the permanent storage type for this layer as a friendly name.
       */
      virtual QString storageType() const;

      /**
       * Select features based on a bounding rectangle. Features can be retrieved 
       * with calls to getFirstFeature and getNextFeature. Request for features 
       * for use in drawing the map canvas should set useIntersect to false.
       * @param mbr QgsRect containing the extent to use in selecting features
       * @param useIntersect If true, use the intersects function to select features
       * rather than the PostGIS && operator that selects based on bounding box
       * overlap.
       *
       */
      virtual void select(QgsRect mbr, bool useIntersect = false) = 0;

      /**
       * Update the feature count based on current spatial filter. If not
       * overridden in the data provider this function returns -1
       */
      virtual long updateFeatureCount();

      /** 
       * Gets the feature at the given feature ID.
       * @param featureId id of the feature
       * @param feature feature which will receive the data
       * @param fetchGeoemtry if true, geometry will be fetched from the provider
       * @param fetchAttributes a list containing the indexes of the attribute fields to copy
       * @return True when feature was found, otherwise false
       */
      virtual bool getFeatureAtId(int featureId,
                                  QgsFeature& feature,
                                  bool fetchGeometry = true,
                                  QList<int> fetchAttributes = QList<int>());

      /**
       * Get the next feature resulting from a select operation.
       * @param feature feature which will receive data from the provider
       * @param fetchGeoemtry if true, geometry will be fetched from the provider
       * @param fetchAttributes a list containing the indexes of the attribute fields to copy
       * @param featureQueueSize  a hint to the provider as to how many features are likely to be retrieved in a batch
       * @return true when there was a feature to fetch, false when end was hit
       */
      virtual bool getNextFeature(QgsFeature& feature,
                                  bool fetchGeometry = true,
                                  QList<int> fetchAttributes = QList<int>(),
                                  uint featureQueueSize = 1) = 0;

      /**
       * Get feature type.
       * @return int representing the feature type
       */
      virtual QGis::WKBTYPE geometryType() const = 0;


      /**
       * Number of features in the layer
       * @return long containing number of features
       */
      virtual long featureCount() const = 0;

      /**
       * Get the attributes associated with a feature
       * TODO: Get rid of "row" and set up provider-internal caching instead
       */
      virtual void getFeatureAttributes(int key, int& row, QgsFeature *f);

      /**
       * Fetch geometry for a particular feature with id "key",
       * modifies "f" in-place.
       *
       * This function is enabled if capabilities() returns "SelectGeometryAtId".
       */
      virtual void getFeatureGeometry(int key, QgsFeature *f);

      /**
       * Number of attribute fields for a feature in the layer
       */
      virtual uint fieldCount() const = 0;

      /**
       * Return a map of indexes with field names for this layer
       * @return map of fields
       */
      virtual const QMap<int, QgsField> & fields() const = 0;

      /** 
       * Reset the layer to clear any spatial filtering or other contstraints that
       * would prevent the entire record set from being traversed by call to 
       * getNextFeature(). Some data stores may not require any special action to
       * reset the layer. In this case, the provider should simply implement an empty
       * function body.
       */
      virtual void reset() = 0;

      /**
       * Returns the minimum value of an attributs
       * @param position the number of the attribute
       */
      virtual QString minValue(uint position) = 0;

      /**
       * Returns the maximum value of an attributs
       * @param position the number of the attribute
       */
      virtual QString maxValue(uint position) = 0;

      /**
       * Adds a list of features
       * @return true in case of success and false in case of failure
       */
      virtual bool addFeatures(QList<QgsFeature> & flist);

      /** 
       * Deletes a feature
       * @param id list containing feature ids to delete
       * @return true in case of success and false in case of failure
       */
      virtual bool deleteFeatures(const QSet<int> & id);

      /**
       * Adds new attributes
       * @param attributes map with attribute name as key and type as value
       * @return true in case of success and false in case of failure
       */
      virtual bool addAttributes(const QMap<QString, QString> & attributes);

      /**
       * Deletes existing attributes
       * @param attributes a set containing indexes of attributes
       * @return true in case of success and false in case of failure
       */
      virtual bool deleteAttributes(const QSet<int> & attributes);

      /**
       * Changes attribute values of existing features.
       * @param attr_map a map containing changed attributes
       * @return true in case of success and false in case of failure 
       */
      virtual bool changeAttributeValues(const QMap<int, QMap<int, QgsFeatureAttribute> > & attr_map);

      /**
       * Returns the default value for attribute @c attr for feature @c f.
       */
      virtual QString getDefaultValue(const QString & attr, QgsFeature* f);

      /**
       * Changes geometries of existing features
       * @param geometry_map   A std::map containing the feature IDs to change the geometries of. 
       *                       the second map parameter being the new geometries themselves
       * @return               true in case of success and false in case of failure
       */
      virtual bool changeGeometryValues(QMap<int, QgsGeometry> & geometry_map);

      /**
       * Creates a spatial index on the datasource (if supported by the provider type).
       * @return true in case of success
       */
      virtual bool createSpatialIndex();

      /** Returns a bitmask containing the supported capabilities
          Note, some capabilities may change depending on whether
          a spatial filter is active on this provider, so it may
          be prudent to check this value per intended operation.
       */
      virtual int capabilities() const;

      /**
       *  Returns the above in friendly format.
       */
      QString capabilitiesString() const;

      /**
       * Set encoding used for accessing data from layer
       */
      virtual void setEncoding(const QString& e);
      
      /**
       * Get encoding which is used for accessing data
       */
      QString encoding() const;
      
      /**
       * Returns the index of a field name or -1 if the field does not exist
       */
      int indexFromFieldName(const QString& fieldName) const;
      
      /**
       * Return list of indexes to fetch all attributes in getNextFeature()
       */
      QList<int> allAttributesList();

      /**
       * Set whether provider should return also features that don't have
       * associated geometry. FALSE by default
       */
      void setFetchFeaturesWithoutGeom(bool fetch);

};
