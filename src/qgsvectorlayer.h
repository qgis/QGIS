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
class QgsData;
class QgsRenderer;
class QgsLegendItem;
class QgsDlgVectorLayerProperties;
class QgisApp;
class QgsIdentifyResults;
class QgsLabel;

#include <map>
#include <vector>

#include "qgsmaplayer.h"
#include "qvaluevector.h"
#include "qgsattributetabledisplay.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeaction.h"

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

  //! Identify feature found within the search rectangle
  void identify(QgsRect *);

  //! Select features found within the search rectangle
  void select(QgsRect * rect, bool lock);

  //! Display the attribute table
  void table();

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
  void setDataProvider( QString const & provider );

  QgsVectorDataProvider * getDataProvider();

  /** \brief Query gdal to find out the WKT projection string for this layer. This implements the virtual method of the same name defined in QgsMapLayer*/
  QString getProjectionWKT() 
  { 
  //delegate to data provider...
  };
    
  QgsLabel *label();

  QgsAttributeAction* actions() { return &mActions; }

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
  /**Returns a pointer to the renderer dialog*/
  QDialog *rendererDialog();
  /**Sets the renderer. If a renderer is already present, it is deleted*/
  void setRenderer(QgsRenderer * r);
  /**Sets the renderer dialog. If a renderer dialog is already present, it is deleted*/
  void setRendererDialog(QDialog * dialog);
  /**Sets m_propertiesDialog*/
  void setLayerProperties(QgsDlgVectorLayerProperties * properties);
  /**Returns point, line or polygon*/
    QGis::VectorType vectorType();
  /**Returns a pointer to the properties dialog*/
  QgsDlgVectorLayerProperties *propertiesDialog();
  /** Return the context menu for the layer */
  QPopupMenu *contextMenu();
    /**Returns the bounding box of the selected features. If there is no selection, the lower bounds are DBL_MAX and the upper bounds -DBL_MAX*/
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

  /**Adds a feature
   @return true in case of success and false in case of error*/
  bool addFeature(QgsFeature* f);

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
protected:
  /**Pointer to the table display object if there is one, else a pointer to 0*/
    QgsAttributeTableDisplay * tabledisplay;
  /**Vector holding the information which features are activated*/
  std::set<int> mSelected;
  std::set<int> mDeleted;
  std::list<QgsFeature*> mAddedFeatures;
  /**Color to and fill the selected features*/
  QColor selectionColor;
  /**Renderer object which holds the information about how to display the features*/
  QgsRenderer *m_renderer;
  /**Label */
  QgsLabel *mLabel;
  /**Display labels */ 
  bool mLabelOn;
  /**Dialog to set the properties*/
  QgsDlgVectorLayerProperties *m_propertiesDialog;
  /**Widget to set the symbology properties*/
  QDialog *m_rendererDialog;
  /**Goes through all features and finds a free id (e.g. to give it temporarily to a not-commited feature)*/
  int findFreeId();
  /**Writes the changes to disk*/
  bool commitChanges();
  /**Discards the edits*/
  bool rollBack();

  protected slots:
  void startEditing();
  void stopEditing();

  void drawFeature(QPainter* p, QgsFeature* fet, QgsMapToPixel * cXf, QPicture* marker, double markerScaleFactor);

private:                       // Private attributes

  //! Draws the layer labels using coordinate transformation
  void drawLabels(QPainter * p, QgsRect * viewExtent, QgsMapToPixel * cXf,  QPaintDevice * dst);

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
  } endian_t;
  
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

  /**Flag indicating wheter the layer is in editing mode or not*/
  bool mEditable;
  /**Flag indicating wheter the layer has been modified since the last commit*/
  bool mModified;

};

#endif
