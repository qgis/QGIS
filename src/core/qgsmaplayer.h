/***************************************************************************
                          qgsmaplayer.h  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
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

#ifndef QGSMAPLAYER_H
#define QGSMAPLAYER_H

#include <QDateTime>
#include <QObject>
#include <QUndoStack>
#include <QVariant>
#include <QImage>
#include <QDomNode>
#include <QPainter>

#include "qgis.h"
#include "qgserror.h"
#include "qgsrectangle.h"
#include "qgsmaprenderer.h"

class QgsRenderContext;
class QgsCoordinateReferenceSystem;

class QDomDocument;
class QKeyEvent;
class QPainter;

/** \ingroup core
 * Base class for all map layer types.
 * This is the base class for all map layer types (vector, raster).
 */
class CORE_EXPORT QgsMapLayer : public QObject
{
    Q_OBJECT

  public:
    /** Layers enum defining the types of layers that can be added to a map */
    enum LayerType
    {
      VectorLayer,
      RasterLayer,
      PluginLayer // added in 1.5
    };

    /** Constructor
     * @param type Type of layer as defined in QgsMapLayer::LayerType enum
     * @param lyrname Display Name of the layer
     * @param source datasource of layer
     */
    QgsMapLayer( QgsMapLayer::LayerType type = VectorLayer, QString lyrname = QString::null, QString source = QString::null );

    /** Destructor */
    virtual ~QgsMapLayer();

    /** Get the type of the layer
     * @return Integer matching a value in the QgsMapLayer::LayerType enum
     */
    QgsMapLayer::LayerType type() const;

    /** Get this layer's unique ID, this ID is used to access this layer from map layer registry
     * @note added in 1.7
     */
    QString id() const;

    /** Set the display name of the layer
     * @param name New name for the layer
     */
    void setLayerName( const QString & name );

    /** Get the display name of the layer
     * @return the layer name
     */
    const QString & name() const;

    /** Get the original name of the layer
     * @note added in 1.9
     */
    const QString & originalName() const { return mLayerOrigName; }

    void setTitle( const QString& title ) { mTitle = title; }
    const QString& title() const { return mTitle; }

    void setAbstract( const QString& abstract ) { mAbstract = abstract; }
    const QString& abstract() const { return mAbstract; }

    /* Set the blending mode used for rendering a layer */
    void setBlendMode( const QPainter::CompositionMode blendMode );
    /* Returns the current blending mode for a layer */
    QPainter::CompositionMode blendMode() const;

    /**Synchronises with changes in the datasource
        @note added in version 1.6*/
    virtual void reload() {}

    /** This is the method that does the actual work of
     * drawing the layer onto a paint device.
     * @param rendererContext describes the extents,
     * resolumon etc. that should be used when rendering the
     * layer.
     */
    virtual bool draw( QgsRenderContext& rendererContext );

    /** Draw labels
     * @todo to be removed: used only in vector layers
     */
    virtual void drawLabels( QgsRenderContext& rendererContext );

    /** Return the extent of the layer */
    virtual QgsRectangle extent();

    /*! Return the status of the layer. An invalid layer is one which has a bad datasource
     * or other problem. Child classes set this flag when intialized
     * @return True if the layer is valid and can be accessed
     */
    bool isValid();

    /*! Gets a version of the internal layer definition that has sensitive
      *  bits removed (for example, the password). This function should
      * be used when displaying the source name for general viewing.
     */
    QString publicSource() const;

    /** Returns the source for the layer */
    const QString &source() const;

    /**
     * Returns the sublayers of this layer
     * (Useful for providers that manage their own layers, such as WMS)
     */
    virtual QStringList subLayers() const;

    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top
     * (Useful for providers that manage their own layers, such as WMS)
     */
    virtual void setLayerOrder( const QStringList &layers );

    /** Set the visibility of the given sublayer name */
    virtual void setSubLayerVisibility( QString name, bool vis );

    /** True if the layer can be edited */
    virtual bool isEditable() const;

    /** sets state from Dom document
       @param layer_node is Dom node corresponding to ``maplayer'' tag
       @note

       The Dom node corresponds to a Dom document project file XML element read
       by QgsProject.

       This, in turn, calls readXml(), which is over-rideable by sub-classes so
       that they can read their own specific state from the given Dom node.

       Invoked by QgsProject::read().

       @returns true if successful
     */
    bool readXML( const QDomNode& layer_node );


    /** stores state in Dom node
       @param layer_node is Dom node corresponding to ``projectlayers'' tag
       @param document is Dom document
       @note

       The Dom node corresponds to a Dom document project file XML element to be
       written by QgsProject.

       This, in turn, calls writeXml(), which is over-rideable by sub-classes so
       that they can write their own specific state to the given Dom node.

       Invoked by QgsProject::write().

       @returns true if successful
    */
    bool writeXML( QDomNode & layer_node, QDomDocument & document );

    /** Set a custom property for layer. Properties are stored in a map and saved in project file.
     *  @note Added in v1.4 */
    void setCustomProperty( const QString& key, const QVariant& value );
    /** Read a custom property from layer. Properties are stored in a map and saved in project file.
     *  @note Added in v1.4 */
    QVariant customProperty( const QString& value, const QVariant& defaultValue = QVariant() ) const;
    /** Remove a custom property from layer. Properties are stored in a map and saved in project file.
     *  @note Added in v1.4 */
    void removeCustomProperty( const QString& key );

#if 0
    /** Accessor for transparency level. */
    unsigned int getTransparency();

    /** Mutator for transparency level. Should be between 0 and 255 */
    virtual void setTransparency( unsigned int );
#endif

    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    virtual QString lastErrorTitle();

    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    virtual QString lastError();

    /** Get current status error. This error describes some principal problem
     *  for which layer cannot work and thus is not valid. It is not last error
     *  after accessing data by draw() etc.
     */
    virtual QgsError error() const { return mError; }

    /** Returns layer's spatial reference system
    @note This was introduced in QGIS 1.4
    */
    const QgsCoordinateReferenceSystem& crs() const;

    /** Sets layer's spatial reference system
    @note emitSignal added in 1.4 */
    void setCrs( const QgsCoordinateReferenceSystem& srs, bool emitSignal = true );

    /** A convenience function to (un)capitalise the layer name */
    static QString capitaliseLayerName( const QString& name );

    /** Retrieve the style URI for this layer
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @return a QString with the style file name
     * @see also loadNamedStyle () and saveNamedStyle ();
     * @note This method was added in QGIS 1.8
     */
    virtual QString styleURI( );

    /** Retrieve the default style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param theResultFlag a reference to a flag that will be set to false if
     * we did not manage to load the default style.
     * @return a QString with any status messages
     * @see also loadNamedStyle ();
     */
    virtual QString loadDefaultStyle( bool & theResultFlag );

    /** Retrieve a named style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param theURI - the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and load that, if that fails the qgis.db styles
     * table will be consulted to see if there is a style who's
     * key matches the URI.
     * @param theResultFlag a reference to a flag that will be set to false if
     * we did not manage to load the default style.
     * @return a QString with any status messages
     * @see also loadDefaultStyle ();
     */
    virtual QString loadNamedStyle( const QString theURI, bool & theResultFlag );

    virtual bool loadNamedStyleFromDb( const QString db, const QString theURI, QString &qml );

    //TODO edit infos
    /**
     * Export the properties of this layer as named style in a QDomDocument
     * @param doc the target QDomDocument
     * @param errorMsg this QString will be initialized on error
     * during the execution of writeSymbology
     */
    virtual void exportNamedStyle( QDomDocument &doc, QString &errorMsg );


    /**
     * Export the properties of this layer as SLD style in a QDomDocument
     * @param doc the target QDomDocument
     * @param errorMsg this QString will be initialized on error
     * during the execution of writeSymbology
     */
    virtual void exportSldStyle( QDomDocument &doc, QString &errorMsg );

    /** Save the properties of this layer as the default style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param theResultFlag a reference to a flag that will be set to false if
     * we did not manage to save the default style.
     * @return a QString with any status messages
     * @sa loadNamedStyle() and @see saveNamedStyle()
     */
    virtual QString saveDefaultStyle( bool & theResultFlag );

    /** Save the properties of this layer as a named style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param theURI the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and save to that, if that fails the qgis.db styles
     * table will be used to create a style entry who's
     * key matches the URI.
     * @param theResultFlag a reference to a flag that will be set to false if
     * we did not manage to save the default style.
     * @return a QString with any status messages
     * @sa saveDefaultStyle()
     */
    virtual QString saveNamedStyle( const QString theURI, bool & theResultFlag );

    virtual QString saveSldStyle( const QString theURI, bool & theResultFlag );
    virtual QString loadSldStyle( const QString theURI, bool &theResultFlag );

    virtual bool readSld( const QDomNode& node, QString& errorMessage )
    { Q_UNUSED( node ); errorMessage = QString( "Layer type %1 not supported" ).arg( type() ); return false; }


    /** Read the symbology for the current layer from the Dom node supplied.
     * @param node node that will contain the symbology definition for this layer.
     * @param errorMessage reference to string that will be updated with any error messages
     * @return true in case of success.
    */
    virtual bool readSymbology( const QDomNode& node, QString& errorMessage ) = 0;

    /** Write the symbology for the layer into the docment provided.
     *  @param node the node that will have the style element added to it.
     *  @param doc the document that will have the QDomNode added.
     *  @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     */
    virtual bool writeSymbology( QDomNode &node, QDomDocument& doc, QString& errorMessage ) const = 0;

    /** Return pointer to layer's undo stack */
    QUndoStack* undoStack();

    /** Get the QImage used for caching render operations
     * @note This method was added in QGIS 1.4 **/
    QImage *cacheImage() { return mpCacheImage; }
    /** Set the QImage used for caching render operations
     * @note This method was added in QGIS 1.4 **/
    void setCacheImage( QImage * thepImage );

    /**
     * @brief Is called when the cache image is being deleted. Overwrite and use to clean up.
     * @note added in 2.0
     */
    virtual void onCacheImageDelete() {};

  public slots:

    /** Event handler for when a coordinate transform fails due to bad vertex error */
    virtual void invalidTransformInput();

    /** Accessor and mutator for the minimum scale denominator member */
    void setMinimumScale( float theMinScale );
    float minimumScale();

    /** Accessor and mutator for the maximum scale denominator member */
    void setMaximumScale( float theMaxScale );
    float maximumScale();

    /** Accessor and mutator for the scale based visilibility flag */
    void toggleScaleBasedVisibility( bool theVisibilityFlag );
    bool hasScaleBasedVisibility();

    /** Clear cached image
     * added in 1.5 */
    void clearCacheImage();

    /** \brief Obtain Metadata for this layer */
    virtual QString metadata();

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const { return QDateTime() ; }

  signals:

    /** Emit a signal to notify of a progress event */
    void drawingProgress( int theProgress, int theTotalSteps );

    /** Emit a signal with status (e.g. to be caught by QgisApp and display a msg on status bar) */
    void statusChanged( QString theStatus );

    /** Emit a signal that the layer name has been changed */
    void layerNameChanged();

    /** Emit a signal that layer's CRS has been reset
     added in 1.4
     */
    void layerCrsChanged();

    /** This signal should be connected with the slot QgsMapCanvas::refresh()
     * \todo to be removed - GUI dependency
     */
    void repaintRequested();

    /**The layer emits this signal when a screen update is requested.
     This signal should be connected with the slot QgsMapCanvas::updateMap()*/
    void screenUpdateRequested();

    /** This is used to send a request that any mapcanvas using this layer update its extents */
    void recalculateExtents();

    /** data of layer changed
     * added in 1.5 */
    void dataChanged();

  protected:
    /** Set the extent */
    virtual void setExtent( const QgsRectangle &rect );

    /** set whether layer is valid or not - should be used in constructor.
        \note added in v1.5 */
    void setValid( bool valid );

    /** called by readXML(), used by children to read state specific to them from
        project files.
    */
    virtual bool readXml( const QDomNode& layer_node );

    /** called by writeXML(), used by children to write state specific to them to
        project files.
    */
    virtual bool writeXml( QDomNode & layer_node, QDomDocument & document );


    /** Read custom properties from project file. Added in v1.4
      @param layerNode note to read from
      @param keyStartsWith reads only properties starting with the specified string (or all if the string is empty)*/
    void readCustomProperties( const QDomNode& layerNode, const QString& keyStartsWith = "" );

    /** Write custom properties to project file. Added in v1.4 */
    void writeCustomProperties( QDomNode & layerNode, QDomDocument & doc ) const;

    /** debugging member - invoked when a connect() is made to this object */
    void connectNotify( const char * signal );

    /** Add error message */
    void appendError( const QgsErrorMessage & theMessage ) { mError.append( theMessage );}
    /** Set error message */
    void setError( const QgsError & theError ) { mError = theError;}

    /** Transparency level for this layer should be 0-255 (255 being opaque) */
    unsigned int mTransparencyLevel;

    /** Extent of the layer */
    QgsRectangle mExtent;

    /** Indicates if the layer is valid and can be drawn */
    bool mValid;

    /** data source description string, varies by layer type */
    QString mDataSource;

    /** Name of the layer - used for display */
    QString mLayerName;

    /** Original name of the layer
     *  @note added in 1.9
     */
    QString mLayerOrigName;

    QString mTitle;

    /**Description of the layer*/
    QString mAbstract;

    /** \brief Error */
    QgsError mError;

  private:
    /** layer's spatial reference system.
        private to make sure setCrs must be used and layerCrsChanged() is emitted */
    QgsCoordinateReferenceSystem* mCRS;

    /** private copy constructor - QgsMapLayer not copyable */
    QgsMapLayer( QgsMapLayer const & );

    /** private assign operator - QgsMapLayer not copyable */
    QgsMapLayer & operator=( QgsMapLayer const & );

    /** Unique ID of this layer - used to refer to this layer in map layer registry */
    QString mID;

    /** Type of the layer (eg. vector, raster) */
    QgsMapLayer::LayerType mLayerType;

    /** Blend mode for the layer */
    QPainter::CompositionMode mBlendMode;

    /** Tag for embedding additional information */
    QString mTag;

    /** Minimum scale denominator at which this layer should be displayed */
    float mMinScale;
    /** Maximum scale denominator at which this layer should be displayed */
    float mMaxScale;
    /** A flag that tells us whether to use the above vars to restrict layer visibility */
    bool mScaleBasedVisibility;

    /** Collection of undoable operations for this layer. **/
    QUndoStack mUndoStack;

    QMap<QString, QVariant> mCustomProperties;

    /**QImage for caching of rendering operations
     * @note This property was added in QGIS 1.4 **/
    QImage * mpCacheImage;

};

#endif
