
class QgsVectorDataProvider : QgsDataProvider
{
%TypeHeaderCode
#include <qgsvectordataprovider.h>
%End

    public:

  QgsVectorDataProvider* __iter__();
%MethodCode
    sipRes = sipCpp;
%End

  SIP_PYOBJECT __next__();
%MethodCode
    QgsFeature* f = new QgsFeature;
    if (sipCpp->nextFeature(*f))
      sipRes = sipConvertFromInstance(f, sipClass_QgsFeature, Py_None);
    else
    {
      delete f;
      PyErr_SetString(PyExc_StopIteration,"");
    }
%End

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

      /** bitmask of all provider's editing capabilities */
      static const int EditingCapabilities;

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

      /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
       * @param fetchAttributes list of attributes which should be fetched
       * @param rect spatial filter
       * @param fetchGeometry true if the feature geometry should be fetched
       * @param useIntersect true if an accurate intersection test should be used,
       *                     false if a test based on bounding box is sufficient
       */
      virtual void select(QList<int> fetchAttributes = QList<int>(),
                          QgsRectangle rect = QgsRectangle(),
                          bool fetchGeometry = true,
                          bool useIntersect = false) = 0;

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
      virtual bool featureAtId(int featureId,
                                  QgsFeature& feature,
                                  bool fetchGeometry = true,
                                  QList<int> fetchAttributes = QList<int>());

      /**
       * Get the next feature resulting from a select operation.
       * @param feature feature which will receive data from the provider
       * @return true when there was a feature to fetch, false when end was hit
       */
      virtual bool nextFeature(QgsFeature& feature) = 0;

      /**
       * Get feature type.
       * @return int representing the feature type
       */
      virtual QGis::WkbType geometryType() const = 0;

      /**
       * Number of features in the layer
       * @return long containing number of features
       */
      virtual long featureCount() const = 0;

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
       * Return a short comment for the data that this provider is
       * providing access to (e.g. the comment for postgres table).
       */
      virtual QString dataComment() const;
      
      /** Restart reading features from previous select operation */
      virtual void rewind() = 0;

      /**
       * Returns the minimum value of an attributs
       * @param index the index of the attribute
       *
       * Default implementation walks all numeric attributes and caches minimal
       * and maximal values. If provider has facilities to retrieve minimal
       * value directly, override this function.
       */
      virtual QVariant minimumValue(int index);

      /**
       * Returns the maximum value of an attributs
       * @param index the index of the attribute
       *
       * Default implementation walks all numeric attributes and caches minimal
       * and maximal values. If provider has facilities to retrieve maximal
       * value directly, override this function.
       */
      virtual QVariant maximumValue(int index);

     /**
       * Return unique values of an attribute
       * @param index the index of the attribute
       * @param values reference to the list to fill
       * @param limit maximum number of values to return (added in 1.4)
       *
       * Default implementation simply iterates the features 
       */
      virtual void uniqueValues(int index, QList<QVariant> &uniqueValues /Out/, int limit = -1);

      /**Returns the possible enum values of an attribute. Returns an empty stringlist if a provider does not support enum types
      or if the given attribute is not an enum type.
      * @param index the index of the attribute
      * @param enumList reference to the list to fill
      @note: added in version 1.2*/
      virtual void enumValues( int index, QStringList& enumList /Out/);

      /**
       * Adds a list of features
       * @return true in case of success and false in case of failure
       */
      virtual bool addFeatures(QList<QgsFeature> & flist /In,Out/);

      /** 
       * Deletes a feature
       * @param id list containing feature ids to delete
       * @return true in case of success and false in case of failure
       */
      virtual bool deleteFeatures(const QSet<int> & id);

      /**
       * Adds new attributes
       * @param attributes list of new attribute fields
       * @return true in case of success and false in case of failure
       * @note changed in 1.2
       */
      virtual bool addAttributes(const QList<QgsField> &attributes);

      /**
       * Adds new attributes
       * @param attributes map with attribute name as key and type as value
       * @return true in case of success and false in case of failure
       * @deprecated
       */
      virtual bool addAttributes(const QMap<QString, QString> &attributes);

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
      virtual bool changeAttributeValues(const QMap<int, QMap<int, QVariant> > & attr_map);

      /**
       * Returns the default value for field specified by @c fieldId
       */
      virtual QVariant defaultValue(int fieldId);

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
      int fieldNameIndex(const QString& fieldName) const;
      
      /**
       * Return list of indexes to fetch all attributes in nextFeature()
       */
      QList<int> attributeIndexes();

      /**
       * check if provider supports type of field
       * @note added in 1.2
       */
      bool supportedType( const QgsField &field ) const;

      struct NativeType
      {
	NativeType( QString typeDesc, QString typeName, QVariant::Type type, int minLen = 0, int maxLen = 0, int minPrec = 0, int maxPrec = 0 );
      };

      /**
       * Returns the names of the supported types
       * @note added in 1.2
       */
      const QList< QgsVectorDataProvider::NativeType > &nativeTypes() const;

      /**Returns the names of the numerical types
         @deprecated */
      const QMap<QString,QVariant::Type> &supportedNativeTypes() const; 

      /**
       * Set whether provider should return also features that don't have
       * associated geometry. FALSE by default
       */
      void enableGeometrylessFeatures(bool fetch);

      /** Returns true if the provider is strict about the type of inserted features
          (e.g. no multipolygon in a polygon layer)
          @note: added in version 1.4*/
      bool doesStrictFeatureTypeCheck() const;

      /** Returns a list of available encodings */
      static const QStringList &availableEncodings();

      /* provider has errors to report
       * @note added in 1.7
       */
      bool hasErrors();

      /* clear recorded errors 
       * @note added in 1.7
       */
      void clearErrors();

      /* get recorded errors
       * @note added in 1.7
       */
      QStringList errors();
};
