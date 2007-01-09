
/*! \class QgsCoordinateTransform
* \brief Class for doing transforms between two map coordinate systems.
*
* This class can convert map coordinates to a different spatial reference system.
* It is normally associated with a map layer and is used to transform between the
* layer's coordinate system and the coordinate system of the map canvas, although
* it can be used in a more general sense to transform coordinates.
*
* All references to source and destination coordinate systems refer to 
* layer and map canvas respectively. All operations are from the perspective 
* of the layer. For example, a forward transformation transforms coordinates from the
* layers coordinate system to the map canvas.
*/
class QgsCoordinateTransform : QObject
{
%TypeHeaderCode
#include <qgscoordinatetransform.h>
%End
  
 public:
    /*! Default constructor. Make sure you use initialised() manually if you use this one! */
    QgsCoordinateTransform() ;

    /** Constructs a QgsCoordinateTransform using QgsSpatialRefSys objects.
    * @param theSource SRS, typically of the layer's coordinate system
    * @param theDest SRS, typically of the map canvas coordinate system
    */
    QgsCoordinateTransform(const QgsSpatialRefSys& theSource, 
                          const QgsSpatialRefSys& theDest);
  
    /** Constructs a QgsCoordinateTransform using SRS ID of source and destination SRS */
    QgsCoordinateTransform(long theSourceSrsId, long theDestSrsId);

    /*!
     * Constructs a QgsCoordinateTransform using the Well Known Text representation
     * of the layer and map canvas coordinate systems
     * @param theSourceWKT WKT, typically of the layer's coordinate system
     * @param theDestWKT WKT, typically of the map canvas coordinate system
     */
    QgsCoordinateTransform(QString theSourceWKT, QString theDestWKT  );

    /*!
     * Constructs a QgsCoordinateTransform using a Spatial Reference Id
     * of the layer and map canvas coordinate system as Wkt
     * @param theSourceSrid Spatial Ref Id of the layer's coordinate system
     * @param theSourceWKT WKT of the map canvas coordinate system
     * @param theSourceSRSType On of the enum members defined in QgsSpatialRefSys::SRS_TYPE
     */
    QgsCoordinateTransform(long theSourceSrid,  
                           QString theDestWKT,
                           QgsSpatialRefSys::SRS_TYPE theSourceSRSType = QgsSpatialRefSys::POSTGIS_SRID  );

     //! destructor
    ~QgsCoordinateTransform();

    //! Enum used to indicate the direction (forward or inverse) of the transform
    enum TransformDirection{
      FORWARD,     /*!< Transform from source to destination SRS. */
      INVERSE      /*!< Transform from destination to source SRS. */
    };
    
    /*! 
     * Set the source (layer) QgsSpatialRefSys
     * @param theSRS QgsSpatialRefSys representation of the layer's coordinate system
     */
    void setSourceSRS(const QgsSpatialRefSys& theSRS);

    /*! 
     * Mutator for dest QgsSpatialRefSys 
     * @param theSRS of the destination coordinate system
     */
    void setDestSRS(const QgsSpatialRefSys& theSRS);

    /*!
     * Get the QgsSpatialRefSys representation of the layer's coordinate system
     * @return QgsSpatialRefSys of the layer's coordinate system
     */
    QgsSpatialRefSys& sourceSRS();

    /*! 
     * Get the QgsSpatialRefSys representation of the map canvas coordinate system
     * @return QgsSpatialRefSys of the map canvas coordinate system
     */
    QgsSpatialRefSys& destSRS();

    /*! Transform the point from Source Coordinate System to Destination Coordinate System
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param p Point to transform
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsPoint in Destination Coordinate System
     */    
   QgsPoint transform(const QgsPoint p,TransformDirection direction=FORWARD);

    /*! Transform the point specified by x,y from Source Coordinate System to Destination Coordinate System
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param x x cordinate of point to transform
    * @param y y coordinate of point to transform
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsPoint in Destination Coordinate System
     */
   QgsPoint transform(const double x, const double y,TransformDirection direction=FORWARD);

    /*! Transform a QgsRect to the dest Coordinate system 
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * It assumes that rect is a bounding box, and creates a bounding box
    * in the proejcted CS, so that all points in source rectangle is within
    * returned rectangle.
    * @param QgsRect rect to transform
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsRect in Destination Coordinate System
     */        
   QgsRect transformBoundingBox(const QgsRect theRect,TransformDirection direction=FORWARD);

   // Same as for the other transform() functions, but alters the x
   // and y variables in place. The second one works with good old-fashioned
   // C style arrays.
   void transformInPlace(double& x, double& y, double &z, TransformDirection direction = FORWARD);

   // TODO: argument not supported
   //void transformInPlace(std::vector<double>& x, std::vector<double>& y, std::vector<double>& z, 
   //    TransformDirection direction = FORWARD);

    /*! Transform a QgsRect to the dest Coordinate system 
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param QgsRect rect to transform
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsRect in Destination Coordinate System
     */        
   QgsRect transform(const QgsRect theRect,TransformDirection direction=FORWARD);
    
    /*! Transform an array of coordinates to a different Coordinate System
    * If the direction is FORWARD then coordinates are transformed from layer CS --> map canvas CS,
    * otherwise points are transformed from map canvas CS to layerCS.
    * @param x x cordinate of point to transform
    * @param y y coordinate of point to transform     
    * @param direction TransformDirection (defaults to FORWARD)
    * @return QgsRect in Destination Coordinate System
     */        
   void transformCoords( const int &numPoint, double *x, double *y, double *z,TransformDirection direction=FORWARD);

  /*! 
   * Flag to indicate whether the coordinate systems have been initialised
   * @return true if initialised, otherwise false
   */
   bool isInitialised();

   /*! See if the transform short circuits because src and dest are equivalent
    * @return bool True if it short circuits
    */
    bool isShortCircuited();

    /*! Change the destination coordinate system by passing it a qgis srsid
    * A QGIS srsid is a unique key value to an entry on the tbl_srs in the
    * srs.db sqlite database.
    * @note This slot will usually be called if the
    * project properties change and a different coordinate system is 
    * selected. 
    * @note This coord transform will be reinitialised when this slot is called
    * to check if short circuiting is needed or not etc.
    * @param theSRSID -  A long representing the srsid of the srs to be used */
    void setDestSRSID (long theSRSID);

  public slots:
    //!initialise is used to actually create the Transformer instance
    void initialise();

    /*! Restores state from the given DOM node.
    * @param theNode The node from which state will be restored
    * @return bool True on success, False on failure
    */
    bool readXML( QDomNode & theNode );

    /*! Stores state to the given DOM node in the given document
    * @param theNode The node in which state will be restored
    * @param theDom The document in which state will be stored
    * @return bool True on success, False on failure
    */
    bool writeXML( QDomNode & theNode, QDomDocument & theDoc );

 signals:
    /** Signal when an invalid pj_transform() has occured */
    void  invalidTransformInput();

};

