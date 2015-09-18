
/** \ingroup core
* Class for doing transforms between two map coordinate systems.
*
* This class can convert map coordinates to a different coordinate reference system.
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
    /** Default constructor. Make sure you use initialised() manually if you use this one! */
    QgsCoordinateTransform();

    /** Constructs a QgsCoordinateTransform using QgsCoordinateReferenceSystem objects.
    * @param theSource CRS, typically of the layer's coordinate system
    * @param theDest CRS, typically of the map canvas coordinate system
    */
    QgsCoordinateTransform( const QgsCoordinateReferenceSystem& theSource,
                            const QgsCoordinateReferenceSystem& theDest );

    /** Constructs a QgsCoordinateTransform using CRS ID of source and destination CRS */
    QgsCoordinateTransform( long theSourceSrsId, long theDestSrsId );

    /*!
     * Constructs a QgsCoordinateTransform using the Well Known Text representation
     * of the layer and map canvas coordinate systems
     * @param theSourceWkt Wkt, typically of the layer's coordinate system
     * @param theDestWkt Wkt, typically of the map canvas coordinate system
     */
    QgsCoordinateTransform( QString theSourceWkt, QString theDestWkt );

    /*!
     * Constructs a QgsCoordinateTransform using a Spatial Reference Id
     * of the layer and map canvas coordinate system as Wkt
     * @param theSourceSrid Spatial Ref Id of the layer's coordinate system
     * @param theDestWkt Wkt of the map canvas coordinate system
     * @param theSourceCRSType On of the enum members defined in QgsCoordinateReferenceSystem::CrsType
     */
    QgsCoordinateTransform( long theSourceSrid,
                            QString theDestWkt,
                            QgsCoordinateReferenceSystem::CrsType theSourceCRSType = QgsCoordinateReferenceSystem::PostgisCrsId );

    //! destructor
    ~QgsCoordinateTransform();

    QgsCoordinateTransform* clone() const /Factory/;

    //! Enum used to indicate the direction (forward or inverse) of the transform
    enum TransformDirection
    {
      ForwardTransform,     /*!< Transform from source to destination CRS. */
      ReverseTransform      /*!< Transform from destination to source CRS. */
    };

    /*!
     * Set the source (layer) QgsCoordinateReferenceSystem
     * @param theCRS QgsCoordinateReferenceSystem representation of the layer's coordinate system
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem& theCRS );

    /*!
     * Mutator for dest QgsCoordinateReferenceSystem
     * @param theCRS of the destination coordinate system
     */
    void setDestCRS( const QgsCoordinateReferenceSystem& theCRS );

    /*!
     * Get the QgsCoordinateReferenceSystem representation of the layer's coordinate system
     * @return QgsCoordinateReferenceSystem of the layer's coordinate system
     */
    const QgsCoordinateReferenceSystem& sourceCrs() const;

    /*!
     * Get the QgsCoordinateReferenceSystem representation of the map canvas coordinate system
     * @return QgsCoordinateReferenceSystem of the map canvas coordinate system
     */
    const QgsCoordinateReferenceSystem& destCRS() const;

    /** Transform the point from Source Coordinate System to Destination Coordinate System
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param p Point to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsPoint in Destination Coordinate System
     */
    QgsPoint transform( const QgsPoint &p, TransformDirection direction = ForwardTransform ) const throw (QgsCsException);

    /** Transform the point specified by x,y from Source Coordinate System to Destination Coordinate System
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param x x cordinate of point to transform
     * @param y y coordinate of point to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsPoint in Destination Coordinate System
     */
    QgsPoint transform( const double x, const double y, TransformDirection direction = ForwardTransform ) const throw (QgsCsException);

    /** Transform a QgsRectangle to the dest Coordinate system
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * It assumes that rect is a bounding box, and creates a bounding box
     * in the proejcted CS, so that all points in source rectangle is within
     * returned rectangle.
     * @param theRect rect to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @param handle180Crossover set to true if destination crs is geographic and handling of extents crossing the 180 degree
     * longitude line is required
     * @return QgsRectangle in Destination Coordinate System
     */
    QgsRectangle transformBoundingBox( const QgsRectangle &theRect, TransformDirection direction = ForwardTransform, const bool handle180Crossover = false ) const throw (QgsCsException);

    // Same as for the other transform() functions, but alters the x
    // and y variables in place. The second one works with good old-fashioned
    // C style arrays.
    void transformInPlace( double& x, double& y, double &z, TransformDirection direction = ForwardTransform ) const throw (QgsCsException);

    //! @note not available in python bindings
    // void transformInPlace( QVector<double>& x, QVector<double>& y, QVector<double>& z,
    //                        TransformDirection direction = ForwardTransform ) const;

    void transformPolygon( QPolygonF& poly, TransformDirection direction = ForwardTransform ) const;

    // TODO: argument not supported
    // void transformInPlace( QVector<double>& x, QVector<double>& y, QVector<double>& z,
    //                        TransformDirection direction = ForwardTransform ) const;

    /** Transform a QgsRectangle to the dest Coordinate system
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param theRect rect to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsRectangle in Destination Coordinate System
     */
    QgsRectangle transform( const QgsRectangle &theRect, TransformDirection direction = ForwardTransform ) const throw (QgsCsException);

    /** Transform an array of coordinates to a different Coordinate System
     * If the direction is ForwardTransform then coordinates are transformed from layer CS --> map canvas CS,
     * otherwise points are transformed from map canvas CS to layerCS.
     * @param numPoint number of coordinates in arrays
     * @param x array of x coordinates to transform
     * @param y array of y coordinates to transform
     * @param z array of z coordinates to transform
     * @param direction TransformDirection (defaults to ForwardTransform)
     * @return QgsRectangle in Destination Coordinate System
     */
    void transformCoords( const int &numPoint, double *x, double *y, double *z, TransformDirection direction = ForwardTransform ) const throw (QgsCsException);

    /*!
     * Flag to indicate whether the coordinate systems have been initialised
     * @return true if initialised, otherwise false
     */
    bool isInitialised() const;

    /** See if the transform short circuits because src and dest are equivalent
     * @return bool True if it short circuits
     */
    bool isShortCircuited();

    /** Change the destination coordinate system by passing it a qgis srsid
    * A QGIS srsid is a unique key value to an entry on the tbl_srs in the
    * srs.db sqlite database.
    * @note This slot will usually be called if the
    * project properties change and a different coordinate system is
    * selected.
    * @note This coord transform will be reinitialised when this slot is called
    * to check if short circuiting is needed or not etc.
    * @param theCRSID -  A long representing the srsid of the srs to be used */
    void setDestCRSID( long theCRSID );

    /** Returns list of datum transformations for the given src and dest CRS
     * @note not available in python bindings
     */
    // static QList< QList< int > > datumTransformations( const QgsCoordinateReferenceSystem& srcCRS, const QgsCoordinateReferenceSystem& destCRS );
    static QString datumTransformString( int datumTransform );
    /** Gets name of source and dest geographical CRS (to show in a tooltip)
        @return epsgNr epsg code of the transformation (or 0 if not in epsg db)*/
    static bool datumTransformCrsInfo( int datumTransform, int& epsgNr, QString& srcProjection, QString& dstProjection, QString &remarks, QString &scope, bool &preferred, bool &deprecated );

    int sourceDatumTransform() const;
    void setSourceDatumTransform( int dt );
    int destinationDatumTransform() const;
    void setDestinationDatumTransform( int dt );

  public slots:
    //!initialise is used to actually create the Transformer instance
    void initialise();

    /** Restores state from the given Dom node.
    * @param theNode The node from which state will be restored
    * @return bool True on success, False on failure
    */
    bool readXML( QDomNode & theNode );

    /** Stores state to the given Dom node in the given document
    * @param theNode The node in which state will be restored
    * @param theDoc The document in which state will be stored
    * @return bool True on success, False on failure
    */
    bool writeXML( QDomNode & theNode, QDomDocument & theDoc );

  signals:
    /** Signal when an invalid pj_transform() has occured */
    void  invalidTransformInput() const;
};
