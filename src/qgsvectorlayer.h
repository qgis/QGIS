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
class QgsCoordinateTransform;
class OGRLayer;
class OGRDataSource;
class QgsDataProvider;
class QgsRenderer;
class QgsLegendItem;
class QgsDlgVectorLayerProperties;
class QgisApp;
class QgsIdentifyResults;
class QgsLabel;

#include <map>

#include "qgsmaplayer.h"
#include "qvaluevector.h"
#include "qgsattributetabledisplay.h"
#include "qgsvectordataprovider.h"


/*! \class QgsVectorLayer
 * \brief Vector layer backed by a data source provider
 */

class QgsVectorLayer:public QgsMapLayer
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

  QgsDataProvider * getDataProvider();


  QgsLabel *label();

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
   * Number of features in the layer
   * @return long containing number of features
   */
  virtual long featureCount() const;
  
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
  
  /**Set labels on */
  void setLabelOn( bool on );

  /**Label is on */
  bool labelOn( void );

  /**True if the layer can be edited*/
  bool isEditable();

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
protected:
  /**Pointer to the table display object if there is one, else a pointer to 0*/
    QgsAttributeTableDisplay * tabledisplay;
  /**Vector holding the information which features are activated*/
  std::map < int, bool > selected;
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

  protected slots:
  void startEditing();
  void stopEditing();

private:                       // Private attributes

  //! Draws the layer labels using coordinate transformation
  void drawLabels(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf,  QPaintDevice * dst);

    /** tailor the right-click context menu with vector layer only stuff 

      @note called by QgsMapLayer::initContextMenu();
     */
    void initContextMenu_(QgisApp *);

  //! Draws the layer using coordinate transformation
  void draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf,  QPaintDevice * dst);
  //! Pointer to data provider derived from the abastract base class QgsDataProvider
  QgsVectorDataProvider *dataProvider;
  //! index of the primary label field
  QString fieldIndex;
  //! Data provider key
  QString providerKey;
  //! Flag to indicate if this is a valid layer
  bool valid;
  bool registered;

  enum ENDIAN
  {
    NDR = 1,
    XDR = 0
  };
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
  int endian();
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
  

};

#endif
