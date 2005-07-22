/***************************************************************************
                          qgsvectorlayer.h  -  description
                             -------------------
    begin                : Oct 29, 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSVECTORLAYER_H
#define QGSVECTORLAYER_H

class QPainter;
class QgsRect;
class QLibrary;
class QgsMapToPixel;
class OGRLayer;
class OGRDataSource;

class QgisApp;

class QgsData;
class QgsRenderer;
class QgsLegendItem;
class QgsDlgVectorLayerProperties;
class QgsGeometry;
class QgsIdentifyResults;
class QgsLabel;

#include <map>
#include <vector>

#include "qvaluevector.h"

#include "qgsmaplayer.h"
#include "qgsattributeaction.h"
#include "qgsattributetabledisplay.h"
#include "qgsgeometry.h"
#include "qgsgeometryvertexindex.h"
#include "qgsvectordataprovider.h"

/*! \class QgsVectorLayer
 * \brief Vector layer backed by a data source provider
 */

class QgsVectorLayer : public QgsMapLayer
{
  Q_OBJECT;

public:

  //! Constructor
  QgsVectorLayer(QString baseName = 0, QString path = 0, QString providerLib = 0);

  //! Destructor
  virtual ~QgsVectorLayer();

  /**
   *   Returns the permanent storage type for this layer as a friendly name.
   */
  QString storageType() const;

  /**
   *   Capabilities for this layer in a friendly format.
   */
  QString capabilitiesString() const;

  //! Identify feature found within the search rectangle
  void identify(QgsRect *);

  //! Select features found within the search rectangle
  void select(QgsRect * rect, bool lock);

  //!Select not selected features and deselect selected ones
  void invertSelection();

  //! Display the attribute table
  void table();

  //! Resize and fill the attribute table
  void fillTable(QgsAttributeTable* t);

  //! Set the primary display field to be used in the identify results dialog
  void setDisplayField(QString fldName=0);

  //! Returns the primary display field name used in the identify results dialog
const QString displayField() const { return fieldIndex; }

  //! Initialize the context menu
  void initContextMenu(QgisApp * app);

  enum SHAPETYPE
  {
    Point,
    Line,
    Polygon
  };

  /** bind layer to a specific data provider

     @param provider should be "postgres", "ogr", or ??

     @todo XXX should this return bool?  Throw exceptions?
  */
  bool setDataProvider( QString const & provider );
  
  //! Setup the coordinate system tranformation for the layer
  void setCoordinateSystem();

  QgsVectorDataProvider * getDataProvider();

  /** \brief Query data provider to find out the WKT projection string for this layer. This implements the virtual method of the same name defined in QgsMapLayer*/
  QString getProjectionWKT();
  /*!
   * Gets the SRID of the layer by querying the underlying data provider
   * @return The SRID if the provider is able to provide it, otherwise 0
   */
  int getProjectionSrid();
  QgsLabel *label();

  QgsAttributeAction* actions() { return &mActions; }

  /**
      Get a copy of the user-selected features
   */  
  virtual std::vector<QgsFeature>* selectedFeatures();

  /**
      Insert a copy of the given features into the layer
   */
  bool addFeatures(std::vector<QgsFeature*>* features, bool makeSelected = TRUE);

  /**Fill the pixmaps and labels of the renderers into the treeview legend*/
  void refreshLegend();

  
signals:
  /**This signal is emitted when the layer leaves editing mode.
     The purpose is to tell QgsMapCanvas to remove the lines of
     (unfinished) features
  @param norepaint true: there is no repaint at all. False: QgsMapCanvas
  decides, if a repaint is necessary or not*/
  void editingStopped(bool norepaint);
  
  
public slots:

  void inOverview( bool );

  /**Sets the 'tabledisplay' to 0 again*/
  void invalidateTableDisplay();
  void select(int number);
  void removeSelection();
  void triggerRepaint();
  /**Shows the properties dialog*/
  virtual void showLayerProperties();
  /**Returns a pointer to the renderer*/
  QgsRenderer *renderer();
  /**Sets the renderer. If a renderer is already present, it is deleted*/
  void setRenderer(QgsRenderer * r);
  /**Sets m_propertiesDialog*/
  void setLayerProperties(QgsDlgVectorLayerProperties * properties);
  /**Returns point, line or polygon*/
  QGis::VectorType vectorType();
  /**Returns a pointer to the properties dialog*/
  QgsDlgVectorLayerProperties *propertiesDialog();
  /** Return the context menu for the layer */
  QPopupMenu *contextMenu();
  /**Returns the bounding box of the selected features. If there is no selection, QgsRect(0,0,0,0) is returned*/
  virtual QgsRect bBoxOfSelected();
  //! Return the provider type for this layer
  QString providerType();
  //! Return the validity of the layer
  inline bool isValid()
  {
    return valid;
  }

  /** reads vector layer specific state from project file DOM node.

      @note

      Called by QgsMapLayer::readXML().

  */
  /* virtual */ bool readXML_( QDomNode & layer_node );


  /** write vector layer specific state to project file DOM node.

      @note

      Called by QgsMapLayer::writeXML().

  */
  /* virtual */ bool writeXML_( QDomNode & layer_node, QDomDocument & doc );


  /**
  * Get the first feature resulting from a select operation
  * @return QgsFeature
  */
  virtual QgsFeature * getFirstFeature(bool fetchAttributes=false) const;

  /**
  * Get the next feature resulting from a select operation
  * @return QgsFeature
  */
  virtual QgsFeature * getNextFeature(bool fetchAttributes=false) const;

  /**
   * Get the next feature using new method
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

  /**
   * Update the feature count 
   * @return long containing the number of features in the datasource
   */
  virtual long updateFeatureCount() const;

  /**
   * Update the extents for the layer. This is necessary if features are
   * added/deleted or the layer has been subsetted.
   */
  virtual void updateExtents();

  /**
   * Set the string (typically sql) used to define a subset of the layer
   * @param subset The subset string. This may be the where clause of a sql statement
   * or other defintion string specific to the underlying dataprovider and data
   * store.
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
  virtual std::vector<QgsField> const & fields() const;

  /** Adds a feature
      @param lastFeatureInBatch  If True, will also go to the effort of e.g. updating the extents.
      @return                    Irue in case of success and False in case of error
   */
  bool addFeature(QgsFeature* f, bool alsoUpdateExtent = TRUE);
  
  
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

  /**Deletes the selected features
     @return true in case of success and false otherwise*/
  bool deleteSelectedFeatures();

  /**Returns the default value for the attribute @c attr for the feature
     @c f. */
  QString getDefaultValue(const QString& attr, QgsFeature* f);

  /**Set labels on */
  void setLabelOn( bool on );

  /**Label is on */
  bool labelOn( void );

  /**
   * Minimum scale at which the layer is rendered
   * @return Scale as integer 
   */
  int minimumScale();
  /**
   * Maximum scale at which the layer is rendered
   * @return Scale as integer 
   */
  int maximumScale();
  /**
   * Is scale dependent rendering in effect?
   * @return true if so
   */
  bool scaleDependentRender();

  /**Returns true if the provider is in editing mode*/
  virtual bool isEditable() const {return (mEditable&&dataProvider);}

  /**Returns true if the provider has been modified since the last commit*/
  virtual bool isModified() const {return mModified;}

  //! Save as shapefile
  virtual void saveAsShapefile();

  /**Snaps a point to the closest vertex if there is one within the snapping tolerance
     @param point       The point which is set to the position of a vertex if there is one within the snapping tolerance.
     If there is no point within this tolerance, point is left unchanged.
     @param tolerance   The snapping tolerance
     @return true if the position of point has been changed, and false otherwise */
  bool snapPoint(QgsPoint& point, double tolerance);

  /**Snaps a point to the closest vertex if there is one within the snapping tolerance
     @param atVertex          Set to a vertex index of the snapped-to vertex
     @param snappedFeatureId  Set to the feature ID that where the snapped-to vertex belongs to.
     @param snappedGeometry   Set to the geometry that the snapped-to vertex belongs to.
     @param tolerance         The snapping tolerance
     @return true if the position of the points have been changed, and false otherwise (or not implemented by the provider) 
     
     TODO: Handle returning multiple verticies if they are coincident
   */
  bool snapVertexWithContext(QgsPoint& point,
                             QgsGeometryVertexIndex& atVertex,
                             int& snappedFeatureId,
                             QgsGeometry& snappedGeometry,
                             double tolerance);

  /**Snaps a point to the closest line segment if there is one within the snapping tolerance (mSnappingTolerance)
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

  /**Commits edited attributes. Depending on the feature id,
     the changes are written to not commited features or redirected to
     the data provider*/
  bool commitAttributeChanges(const std::set<QString>& deleted,
            const std::map<QString,QString>& added,
            std::map<int,std::map<QString,QString> >& changed);

  /** \brief Draws the layer using coordinate transformation
   *  \param widthScale line width scale
   *  \param symbolScale symbol scale
   */
  void draw(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * cXf,  QPaintDevice * dst, double widthScale, double symbolScale, int oversampling = 1);

  /** \brief Draws the layer labels using coordinate transformation
   *  \param scale size scale, applied to all values in pixels
   */
  void drawLabels(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * cXf,  QPaintDevice * dst, double scale);

  
 
protected:
  /**Pointer to the table display object if there is one, else a pointer to 0*/
  QgsAttributeTableDisplay * tabledisplay;
  
  /** cache of the committed geometries retrieved for the current display */
  std::map<int, QgsGeometry*> mCachedGeometries;
  
  /** Set holding the feature IDs that are activated */
  std::set<int> mSelected;
  
  /** Set holding the feature IDs that are in "set A" for a future geometry algebra operation
      TODO: BM: Do something useful with this.
   */
  std::set<int> mSubjected;
  
  /** Deleted feature IDs which are not commited */
  std::set<int> mDeleted;
  
  /** New features which are not commited */
  std::vector<QgsFeature*> mAddedFeatures;
  
  /** Changed attributes which are not commited */
  std::map<int,std::map<QString,QString> > mChangedAttributes;
  
  /** Changed geometries which are not commited. */
  std::map<int, QgsGeometry> mChangedGeometries;
  
  /**Renderer object which holds the information about how to display the features*/
  QgsRenderer *m_renderer;
  /**Label */
  QgsLabel *mLabel;
  /**Display labels */
  bool mLabelOn;
  /**Dialog to set the properties*/
  QgsDlgVectorLayerProperties *m_propertiesDialog;
  /**Goes through all features and finds a free id (e.g. to give it temporarily to a not-commited feature)*/
  int findFreeId();
  /**Writes the changes to disk*/
  bool commitChanges();
  /**Discards the edits*/
  bool rollBack();


 
protected slots:
  void toggleEditing();
  
  void startEditing();
  
  void stopEditing();

  void drawFeature(QPainter* p, QgsFeature* fet, QgsMapToPixel * cXf, QPicture* marker, double markerScaleFactor, bool projectionsEnabledFlag );

private:                       // Private attributes

  //! Draws the layer labels using coordinate transformation
  void drawLabels(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * cXf,  QPaintDevice * dst);

  // Convenience function to transform the given point
  void transformPoint(double& x, double& y, 
      QgsMapToPixel* mtp, bool projectionsEnabledFlag);
  
  void transformPoints(std::vector<double>& x, std::vector<double>& y,
      std::vector<double>& z,
      QgsMapToPixel* mtp, bool projectionsEnabledFlag);

  // Inverse projects the given rectangle if coordinate transforms are
  // in effect. Alters the given rectangle
  QgsRect inverseProjectRect(const QgsRect& r) const;

  // Draw the linestring as given in the WKB format. Returns a pointer
  // to the byte after the end of the line string binary data stream
  // (WKB).
  unsigned char* drawLineString(unsigned char* WKBlinestring, QPainter* p,
      QgsMapToPixel* mtp, 
      bool projectionsEnabledFlag);

  // Draw the polygon as given in the WKB format. Returns a pointer to
  // the byte after the end of the polygon binary data stream (WKB).
  unsigned char* drawPolygon(unsigned char* WKBpolygon, QPainter* p, 
      QgsMapToPixel* mtp, bool projectionsEnabledFlag);

  /** tailor the right-click context menu with vector layer only stuff

    @note called by QgsMapLayer::initContextMenu();
   */
  void initContextMenu_(QgisApp *);

  //! Draws the layer using coordinate transformation
  void draw(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * cXf,  QPaintDevice * dst);
  //! Pointer to data provider derived from the abastract base class QgsDataProvider
  QgsVectorDataProvider *dataProvider;
  //! index of the primary label field
  QString fieldIndex;
  //! Data provider key
  QString providerKey;
  //! Flag to indicate if this is a valid layer
  bool valid;
  bool registered;

  /** constants for endian-ness
    XDR is network, or big-endian, byte order
    NDR is little-endian byte order
  */
  typedef enum ENDIAN
  {
    XDR = 0,
    NDR = 1
  }
  endian_t;

  enum WKBTYPE
  {
    WKBPoint = 1,
    WKBLineString,
    WKBPolygon,
    WKBMultiPoint,
    WKBMultiLineString,
    WKBMultiPolygon
  };
private:                       // Private methods
  endian_t endian();
  // pointer for loading the provider library
  QLibrary *myLib;
  //! Pointer to the identify results dialog
  QgsIdentifyResults *ir;
  //! Update threshold for drawing features as they are read. A value of zero indicates
  // that no features will be drawn until all have been read
  int updateThreshold;
  //! Minimum scale factor at which the layer is displayed
  int mMinimumScale;
  //! Maximum scale factor at which the layer is displayed
  int mMaximumScale;
  //! Flag to indicate if scale dependent rendering is in effect
  bool mScaleDependentRender;

  /// vector layers are not copyable
  QgsVectorLayer( QgsVectorLayer const & rhs );

  /// vector layers are not copyable
  QgsVectorLayer & operator=( QgsVectorLayer const & rhs );

  // The user-defined actions that are accessed from the
  // Identify Results dialog box
  QgsAttributeAction mActions;

  /**Flag indicating whether the layer is in editing mode or not*/
  bool mEditable;
  
  /**Flag indicating whether the layer has been modified since the last commit*/
  bool mModified;
  
  /** Position in popup menu that the "Editing" toggle item is placed. */
  int mToggleEditingPopupItem;

};


// Convenience function to transform the given point
inline void QgsVectorLayer::transformPoint(double& x, 
             double& y, 
             QgsMapToPixel* mtp,
             bool projectionsEnabledFlag)
{
  // transform the point
  if (projectionsEnabledFlag)
  {
    double z = 0;
    mCoordinateTransform->transformInPlace(x, y, z);
  }

  // transform from projected coordinate system to pixel 
  // position on map canvas
  mtp->transformInPlace(x, y);
}

inline void QgsVectorLayer::transformPoints(
    std::vector<double>& x, std::vector<double>& y, std::vector<double>& z,
    QgsMapToPixel* mtp, bool projectionsEnabledFlag)
{
  // transform the point
  if (projectionsEnabledFlag)
    mCoordinateTransform->transformInPlace(x, y, z);

  // transform from projected coordinate system to pixel 
  // position on map canvas
  mtp->transformInPlace(x, y);
}
#endif
