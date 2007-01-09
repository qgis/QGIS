

class QgsVectorLayer : QgsMapLayer
{
%TypeHeaderCode
#include <qgsvectorlayer.h>
%End

public:

  /** Constructor */
  QgsVectorLayer(QString baseName = 0, QString path = 0, QString providerLib = 0);

  /** Destructor */
  virtual ~QgsVectorLayer();

  /** Returns the permanent storage type for this layer as a friendly name. */
  QString storageType() const;

  /** Capabilities for this layer in a friendly format. */
  QString capabilitiesString() const;

  /** Set the primary display field to be used in the identify results dialog */
  void setDisplayField(QString fldName=0);

  /** Returns the primary display field name used in the identify results dialog */
  const QString displayField() const;

  /** bind layer to a specific data provider
     @param provider should be "postgres", "ogr", or ??
     @todo XXX should this return bool?  Throw exceptions?
  */
  bool setDataProvider(const QString & provider );
  
  /** Returns the data provider */
  QgsVectorDataProvider* getDataProvider();

  /** Returns the data provider in a const-correct manner */
  //const QgsVectorDataProvider* getDataProvider() const;

  /** Sets the textencoding of the data provider */
  void setProviderEncoding(const QString& encoding);

  /** Setup the coordinate system tranformation for the layer */
  void setCoordinateSystem();

  QgsLabel *label();

  // TODO: wrap QgsAttributeAction* actions();

  /** The number of features that are selected in this layer */
  int selectedFeatureCount();
  
  /** Select features found within the search rectangle (in layer's coordinates) */
  void select(QgsRect & rect, bool lock);

  /** Select feature by its ID, optionally emit signal selectionChanged() */
  void select(int featureId, bool emitSignal = TRUE);
  
  /** Clear selection */
  void removeSelection(bool emitSignal = TRUE);
  
  /** Select not selected features and deselect selected ones */
  void invertSelection();

  /** Get a copy of the user-selected features */  
  QList<QgsFeature> selectedFeatures();
  
  /** Return reference to identifiers of selected features */
  const QSet<int>& selectedFeaturesIds() const;
  
  /** Change selection to the new set of features */
  void setSelectedFeatures(const QSet<int>& ids);

  /** Returns the bounding box of the selected features. If there is no selection, QgsRect(0,0,0,0) is returned */
  QgsRect boundingBoxOfSelected();

  
  /** Insert a copy of the given features into the layer */
  bool addFeatures(QList<QgsFeature> features, bool makeSelected = TRUE);

  /** Copies the symbology settings from another layer. Returns true in case of success */
  bool copySymbologySettings(const QgsMapLayer& other);

  /** Returns true if this layer can be in the same symbology group with another layer */
  bool isSymbologyCompatible(const QgsMapLayer& other) const;
  
  /** Returns a pointer to the renderer */
  const QgsRenderer* renderer() const;

  /** Sets the renderer. If a renderer is already present, it is deleted */
  void setRenderer(QgsRenderer * r);

  /** Returns point, line or polygon */
  QGis::VectorType vectorType() const;

  /**Returns the WKBType or WKBUnknown in case of error*/
  QGis::WKBTYPE geometryType() const;

  /** Return the provider type for this layer */
  QString providerType() const;

  /** reads vector layer specific state from project file DOM node.
   *  @note Called by QgsMapLayer::readXML().
   */
  virtual bool readXML_( QDomNode & layer_node );

  /** write vector layer specific state to project file DOM node.
   *  @note Called by QgsMapLayer::writeXML().
   */
  virtual bool writeXML_( QDomNode & layer_node, QDomDocument & doc );


  /** Get the next feature resulting from a select operation
  * @param selected selected feeatures only
  * @return QgsFeature
  */
  virtual QgsFeature * getNextFeature(bool fetchAttributes=false, bool selected=false) const;

  /** Get the next feature using new method
   * TODO - make this pure virtual once it works and change existing providers
   *        to use this method of fetching features
   */
  virtual bool getNextFeature(QgsFeature &feature, bool fetchAttributes=false) const;

  /**
   * Number of features in the layer. This is necessary if features are
   * added/deleted or the layer has been subsetted. If the data provider
   * chooses not to support this feature, the total number of features
   * can be returned.
   * @return long containing number of features
   */
  virtual long featureCount() const;

  /** Update the feature count 
   * @return long containing the number of features in the datasource
   */
  virtual long updateFeatureCount() const;

  /**
   * Set the string (typically sql) used to define a subset of the layer
   * @param subset The subset string. This may be the where clause of a sql statement
   *               or other defintion string specific to the underlying dataprovider
   *               and data store.
   */
  virtual void setSubsetString(QString subset);

  /**
   * Get the string (typically sql) used to define a subset of the layer
   * @return The subset string or QString::null if not implemented by the provider
   */
  virtual QString subsetString();

  /**
   * Number of attribute fields for a feature in the layer
   */
  virtual int fieldCount() const;

  /**
    Return a list of field names for this layer
   @return vector of field names
  */
  virtual const QMap<int, QgsField> & fields() const;

  /** Adds a feature
      @param lastFeatureInBatch  If True, will also go to the effort of e.g. updating the extents.
      @return                    Irue in case of success and False in case of error
   */
  bool addFeature(QgsFeature& f, bool alsoUpdateExtent = TRUE);
  
  
  /** Insert a new vertex before the given vertex number,
   *  in the given ring, item (first number is index 0), and feature
   *  Not meaningful for Point geometries
   */
  bool insertVertexBefore(double x, double y, int atFeatureId,
                          QgsGeometryVertexIndex beforeVertex);

  /** Moves the vertex at the given position number,
   *  ring and item (first number is index 0), and feature
   *  to the given coordinates
   */
  bool moveVertexAt(double x, double y, int atFeatureId,
                    QgsGeometryVertexIndex atVertex);

  /** Deletes the vertex at the given position number,
   *  ring and item (first number is index 0), and feature
   */
  bool deleteVertexAt(int atFeatureId,
                      QgsGeometryVertexIndex atVertex);

  /** Deletes the selected features
   *  @return true in case of success and false otherwise
   */
  bool deleteSelectedFeatures();

  /** Returns the default value for the attribute @c attr for the feature @c f. */
  QString getDefaultValue(const QString& attr, QgsFeature* f);

  /** Set labels on */
  void setLabelOn( bool on );

  /** Label is on */
  bool labelOn( );

  /** Returns true if the provider is in editing mode */
  virtual bool isEditable() const;

  /** Returns true if the provider has been modified since the last commit */
  virtual bool isModified() const;

  /**Snaps a point to the closest vertex if there is one within the snapping tolerance
     @param point       The point which is set to the position of a vertex if there is one within the snapping tolerance.
     If there is no point within this tolerance, point is left unchanged.
     @param tolerance   The snapping tolerance
     @return true if the position of point has been changed, and false otherwise */
  bool snapPoint(QgsPoint& point, double tolerance);

  /**Snaps a point to the closest vertex if there is one within the snapping tolerance
     @param atVertex          Set to a vertex index of the snapped-to vertex
     @param beforeVertexIndex Returns the index of the vertex before atVertex (for rubber band purposes). -1 if no vertex is before
     @param afterVertexIndex  Returns the index of the vertex after atVertex (for rubber band purposes). -1 if no vertex is after
     @param snappedFeatureId  Set to the feature ID that where the snapped-to vertex belongs to.
     @param snappedGeometry   Set to the geometry that the snapped-to vertex belongs to.
     @param tolerance         The snapping tolerance
     @return true if the position of the points have been changed, and false otherwise (or not implemented by the provider) 
     
     TODO: Handle returning multiple verticies if they are coincident
   */
  bool snapVertexWithContext(QgsPoint& point,
                             QgsGeometryVertexIndex& atVertex,
			     int& beforeVertexIndex,
			     int& afterVertexIndex,
                             int& snappedFeatureId,
                             QgsGeometry& snappedGeometry,
                             double tolerance);

  /**Snaps a point to the closest line segment if there is one within the snapping tolerance
     @param beforeVertex      Set to a value where the snapped-to segment is before this vertex index
     @param snappedFeatureId  Set to the feature ID that where the snapped-to segment belongs to.
     @param snappedGeometry   Set to the geometry that the snapped-to segment belongs to.
     @param tolerance         The snapping tolerance
     @return true if the position of the points have been changed, and false otherwise (or not implemented by the provider) 
     
     TODO: Handle returning multiple lineFeatures if they are coincident
   */
  bool snapSegmentWithContext(QgsPoint& point,
                              QgsGeometryVertexIndex& beforeVertex,
                              int& snappedFeatureId, 
                              QgsGeometry& snappedGeometry,
                              double tolerance);

  /**
    Commits edited attributes. Depending on the feature id,
    the changes are written to not commited features or redirected to
    the data provider

    The commits (in this version) occur in three distinct stages,
    (delete attributes, add attributes, change attribute values)
    so if a stage fails, it's difficult to roll back cleanly.

    \todo Need to indicate at which stage the failed commit occurred,
          for better cleanup and recovery from the error.

    \param deleted  Set of attribute names (i.e. columns) to delete
    \param added    Map (name, type) of attribute names (i.e. columns) to add
    \param changed  Map (feature ID, Map (attribute name, new value) )
                      of attribute values to change

   */
  bool commitAttributeChanges(const QSet<int>& deleted,
                              const QMap<QString, QString>& added,
                              const QMap<int, QMap<int, QgsFeatureAttribute> >& changed);

  /** Draws the layer using coordinate transformation
   *  @return FALSE if an error occurred during drawing
   */
  bool draw(QPainter * p,
            QgsRect & viewExtent,
            QgsMapToPixel * cXf,
            QgsCoordinateTransform* ct,
            bool drawingToEditingCanvas);

  /** Draws the layer labels using coordinate transformation */
  void drawLabels(QPainter * p, QgsRect & viewExtent, QgsMapToPixel * cXf, QgsCoordinateTransform* ct);

  /** \brief Draws the layer using coordinate transformation
   *  \param widthScale line width scale
   *  \param symbolScale symbol scale
   */
  void draw(QPainter * p,
            QgsRect & viewExtent,
            QgsMapToPixel * cXf,
            QgsCoordinateTransform* ct,
            bool drawingToEditingCanvas,
            double widthScale,
            double symbolScale);

  /** \brief Draws the layer labels using coordinate transformation
   *  \param scale size scale, applied to all values in pixels
   */
  void drawLabels(QPainter * p, QgsRect & viewExtent, QgsMapToPixel * cXf, QgsCoordinateTransform* ct, double scale);

  /** returns array of added features */
  QList<QgsFeature>& addedFeatures();

  /** returns array of deleted feature IDs */
  QSet<int>& deletedFeatureIds();
 
  /** returns array of features with changed attributes */
  QMap<int, QMap<int, QgsFeatureAttribute> >& changedAttributes();

  /** Sets whether some features are modified or not */
  void setModified(bool modified = TRUE, bool onlyGeometryWasModified = FALSE);
  
  /** Save as shapefile */
  QString saveAsShapefile(QString path, QString encoding);

  /** Make layer editable */
  bool startEditing();
  
  /**
    Attempts to commit any changes to disk.  Returns the result of the attempt.
    If a commit fails, the in-memory changes are left alone.

    This allows editing to continue if the commit failed on e.g. a
    disallowed value in a Postgres database - the user can re-edit and try
    again.

    The commits (in this version) occur in four distinct stages,
    (add features, change attributes, change geometries, delete features)
    so if a stage fails, it's difficult to roll back cleanly.
    Therefore any error message also includes which stage failed so 
    that the user has some chance of repairing the damage cleanly.

   */
  bool commitChanges();

  /** Stop editing and discard the edits */
  bool rollBack();

public slots:

  void triggerRepaint();

  /** Update the extents for the layer. This is necessary if features are
   *  added/deleted or the layer has been subsetted.
   */
  virtual void updateExtents();

signals:

  /** This signal is emited when selection was changed */
  void selectionChanged();
  
  /** This signal is emitted when modifications has been done on layer */
  void wasModified(bool onlyGeometry);

  /** This signal is emitted when drawing features to tell current progress */
  void drawingProgress(int current, int total);

private:                       // Private methods

  /** vector layers are not copyable */
  QgsVectorLayer( const QgsVectorLayer & rhs );

};

