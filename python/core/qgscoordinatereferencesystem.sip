/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatereferencesystem.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/









class QgsCoordinateReferenceSystem
{
%Docstring
This class represents a coordinate reference system (CRS).

Coordinate reference system object defines a specific map projection, as well as transformations
between different coordinate reference systems. There are various ways how a CRS can be defined:
using well-known text (WKT), PROJ.4 string or combination of authority and code (e.g. EPSG:4326).
QGIS comes with its internal database of coordinate reference systems (stored in SQLite) that
allows lookups of CRS and seamless conversions between the various definitions.

Most commonly one comes across two types of coordinate systems:

1. **Geographic coordinate systems** - based on a geodetic datum, normally with coordinates being
latitude/longitude in degrees. The most common one is World Geodetic System 84 (WGS84).
2. **Projected coordinate systems** - based on a geodetic datum with coordinates projected to a plane,
typically using meters or feet as units. Common projected coordinate systems are Universal
Transverse Mercator or Albers Equal Area.

Internally QGIS uses proj4 library for all the math behind coordinate transformations, so in case
of any troubles with projections it is best to examine the PROJ.4 representation within the object,
as that is the representation that will be ultimately used.

Methods that allow inspection of CRS instances include isValid(), authid(), description(),
toWkt(), toProj4(), mapUnits() and others.
Creation of CRS instances is further described in \ref crs_construct_and_copy section below.
Transformations between coordinate reference systems are done using QgsCoordinateTransform class.

For example, the following code will create and inspect "British national grid" CRS:

.. code-block:: python

     crs = QgsCoordinateReferenceSystem("EPSG:27700")
     if crs.isValid():
         print("CRS Description: {}".format(crs.description()))
         print("CRS PROJ.4 text: {}".format(crs.toProj4()))
     else:
         print("Invalid CRS!")

This will produce the following output:

.. code-block::

     CRS Description: OSGB 1936 / British National Grid
     CRS PROJ.4 text: +proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 [output trimmed]

CRS Definition Formats
======================

This section gives an overview of various supported CRS definition formats:

1. **Authority and Code.** Also referred to as OGC WMS format within QGIS as they have been widely
used in OGC standards. These are encoded as `<auth>:<code>`, for example `EPSG:4326` refers
to WGS84 system. EPSG is the most commonly used authority that covers a wide range
of coordinate systems around the world.

An extended variant of this format is OGC URN. Syntax of URN for CRS definition is
`urn:ogc:def:crs:<auth>:[<version>]:<code>`. This class can also parse URNs (versions
are currently ignored). For example, WGS84 may be encoded as `urn:ogc:def:crs:OGC:1.3:CRS84`.

QGIS adds support for "USER" authority that refers to IDs used internally in QGIS. This variant
is best avoided or used with caution as the IDs are not permanent and they refer to different CRS
on different machines or user profiles.

See authid() and createFromOgcWmsCrs() methods.

2. **PROJ.4 string.** This is a string consisting of a series of key/value pairs in the following
format: `+param1=value1 +param2=value2 [...]`. This is the format natively used by the
underlying proj4 library. For example, the definition of WGS84 looks like this:

+proj=longlat +datum=WGS84 +no_defs

See toProj4() and createFromProj4() methods.

3. **Well-known text (WKT).** Defined by Open Geospatial Consortium (OGC), this is another common
format to define CRS. For WGS84 the OGC WKT definition is the following:

GEOGCS["WGS 84",
DATUM["WGS_1984",
SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],
AUTHORITY["EPSG","6326"]],
PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],
UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],
AUTHORITY["EPSG","4326"]]

See toWkt() and createFromWkt() methods.

CRS Database and Custom CRS
===========================

The database of CRS shipped with QGIS is stored in a SQLite database (see QgsApplication.srsDatabaseFilePath())
and it is based on the data files maintained by GDAL project (a variety of .csv and .wkt files).

Sometimes it happens that users need to use a CRS definition that is not well known
or that has been only created with a specific purpose (and thus its definition is not
available in our database of CRS). Whenever a new CRS definition is seen, it will
be added to the local database (in user's home directory, see QgsApplication.qgisUserDatabaseFilePath()).
QGIS also features a GUI for management of local custom CRS definitions.

There are therefore two databases: one for shipped CRS definitions and one for custom CRS definitions.
Custom CRS have internal IDs (accessible with srsid()) greater or equal to \ref USER_CRS_START_ID.
The local CRS databases should never be accessed directly with SQLite functions, instead
you should use QgsCoordinateReferenceSystem API for CRS lookups and for managements of custom CRS.

Validation
==========

In some cases (most prominently when loading a map layer), QGIS will try to ensure
that the given map layer CRS is valid using validate() call. If not, a custom
validation function will be called - such function may for example show a GUI
for manual CRS selection. The validation function is configured using setCustomCrsValidation().
If validation fails or no validation function is set, the default CRS is assigned
(WGS84). QGIS application registers its validation function that will act according
to user's settings (either show CRS selector dialog or use project/custom CRS).

Object Construction and Copying  {#crs_construct_and_copy}
===============================

The easiest way of creating CRS instances is to use QgsCoordinateReferenceSystem(const QString&)
constructor that automatically recognizes definition format from the given string.

Creation of CRS object involves some queries in a local SQLite database, which may
be potentially expensive. Consequently, CRS creation methods use an internal cache to avoid
unnecessary database lookups. If the CRS database is modified, then it is necessary to call
invalidateCache() to ensure that outdated records are not being returned from the cache.

Since QGIS 2.16 QgsCoordinateReferenceSystem objects are implicitly shared.

Caveats
=======

There are two different flavors of WKT: one is defined by OGC, the other is the standard
used by ESRI. They look very similar, but they are not the same. QGIS is able to consume
both flavors.


.. seealso:: :py:class:`QgsCoordinateTransform`
%End

%TypeHeaderCode
#include "qgscoordinatereferencesystem.h"
%End
  public:

    enum CrsType
    {
      InternalCrsId,
      PostgisCrsId,
      EpsgCrsId
    };

    QgsCoordinateReferenceSystem();
%Docstring
Constructs an invalid CRS object
%End

    ~QgsCoordinateReferenceSystem();

    explicit QgsCoordinateReferenceSystem( const QString &definition );
%Docstring
Constructs a CRS object from a string definition using createFromString()

It supports the following formats:
- "EPSG:<code>" - handled with createFromOgcWms()
- "POSTGIS:<srid>" - handled with createFromSrid()
- "INTERNAL:<srsid>" - handled with createFromSrsId()
- "PROJ4:<proj4>" - handled with createFromProj4()
- "WKT:<wkt>" - handled with createFromWkt()

If no prefix is specified, WKT definition is assumed.

:param definition: A String containing a coordinate reference system definition.

.. seealso:: :py:func:`createFromString`
/ // TODO QGIS 3: remove "POSTGIS" and "INTERNAL", allow PROJ4 without the prefix
%End

    explicit QgsCoordinateReferenceSystem( const long id, CrsType type = PostgisCrsId );
%Docstring
Constructor a CRS object using a PostGIS SRID, an EPSG code or an internal QGIS CRS ID.

.. note::

   We encourage you to use EPSG code, WKT or Proj4 to describe CRS's in your code
wherever possible. Internal QGIS CRS IDs are not guaranteed to be permanent / involatile.

:param id: The ID valid for the chosen CRS ID type
:param type: One of the types described in CrsType
/ // TODO QGIS 3: remove type and always use EPSG code
%End

    QgsCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &srs );
%Docstring
Copy constructor
%End


    operator QVariant() const;

    static QList< long > validSrsIds();
%Docstring
Returns a list of all valid SRS IDs present in the CRS database. Any of the
returned values can be safely passed to fromSrsId() to create a new, valid
QgsCoordinateReferenceSystem object.

.. seealso:: :py:func:`fromSrsId`

.. versionadded:: 3.0
%End


    static QgsCoordinateReferenceSystem fromOgcWmsCrs( const QString &ogcCrs );
%Docstring
Creates a CRS from a given OGC WMS-format Coordinate Reference System string.

:param ogcCrs: OGR compliant CRS definition, e.g., "EPSG:4326"

:return: matching CRS, or an invalid CRS if string could not be matched

.. versionadded:: 3.0

.. seealso:: :py:func:`createFromOgcWmsCrs`
%End

    static QgsCoordinateReferenceSystem fromEpsgId( long epsg );
%Docstring
Creates a CRS from a given EPSG ID.

:param epsg: epsg CRS ID

:return: matching CRS, or an invalid CRS if string could not be matched

.. versionadded:: 3.0
%End

    static QgsCoordinateReferenceSystem fromProj4( const QString &proj4 );
%Docstring
Creates a CRS from a proj4 style formatted string.

:param proj4: proj4 format string

:return: matching CRS, or an invalid CRS if string could not be matched

.. versionadded:: 3.0

.. seealso:: :py:func:`createFromProj4`
%End

    static QgsCoordinateReferenceSystem fromWkt( const QString &wkt );
%Docstring
Creates a CRS from a WKT spatial ref sys definition string.

:param wkt: WKT for the desired spatial reference system.

:return: matching CRS, or an invalid CRS if string could not be matched

.. versionadded:: 3.0

.. seealso:: :py:func:`createFromWkt`
%End

    static QgsCoordinateReferenceSystem fromSrsId( long srsId );
%Docstring
Creates a CRS from a specified QGIS SRS ID.

:param srsId: internal QGIS SRS ID

:return: matching CRS, or an invalid CRS if ID could not be found

.. versionadded:: 3.0

.. seealso:: :py:func:`createFromSrsId`

.. seealso:: :py:func:`validSrsIds`
%End


    bool createFromId( const long id, CrsType type = PostgisCrsId );
%Docstring
Sets this CRS by lookup of the given ID in the CRS database.

:return: True on success else false

.. note::

   We encourage you to use EPSG code, WKT or Proj4 to describe CRS's in your code
wherever possible. Internal QGIS CRS IDs are not guaranteed to be permanent / involatile.
/     // TODO QGIS 3: remove type and always use EPSG code, rename to createFromEpsg
%End

    bool createFromOgcWmsCrs( const QString &crs );
%Docstring
Sets this CRS to the given OGC WMS-format Coordinate Reference Systems.

Accepts both "<auth>:<code>" format and OGC URN "urn:ogc:def:crs:<auth>:[<version>]:<code>".
It also recognizes "QGIS", "USER", "CUSTOM" authorities, which all have the same meaning
and refer to QGIS internal CRS IDs.

:return: True on success else false

.. note::

   this method uses an internal cache. Call invalidateCache() to clear the cache.

.. seealso:: :py:func:`fromOgcWmsCrs`
/     // TODO QGIS 3: remove "QGIS" and "CUSTOM", only support "USER" (also returned by authid())
%End

    bool createFromSrid( const long srid );
%Docstring
Sets this CRS by lookup of the given PostGIS SRID in the CRS database.

:param srid: The PostGIS SRID for the desired spatial reference system.

:return: True on success else false
/     // TODO QGIS 3: remove unless really necessary - let's use EPSG codes instead
%End

    bool createFromWkt( const QString &wkt );
%Docstring
Sets this CRS using a WKT definition.

If EPSG code of the WKT definition can be determined, it is extracted
and createFromOgcWmsCrs() is used to initialize the object.
Otherwise the WKT will be converted to a proj4 string and createFromProj4()
set up the object.

:param wkt: The WKT for the desired spatial reference system.

:return: True on success else false

.. note::

   Some members may be left blank if no match can be found in CRS database.

.. note::

   this method uses an internal cache. Call invalidateCache() to clear the cache.

.. seealso:: :py:func:`fromWkt`
%End

    bool createFromSrsId( const long srsId );
%Docstring
Sets this CRS by lookup of internal QGIS CRS ID in the CRS database.

If the srsid is < USER_CRS_START_ID, system CRS database is used, otherwise
user's local CRS database from home directory is used.

:param srsId: The internal QGIS CRS ID for the desired spatial reference system.

:return: True on success else false

.. note::

   this method uses an internal cache. Call invalidateCache() to clear the cache.

.. seealso:: :py:func:`fromSrsId`
%End

    bool createFromProj4( const QString &projString );
%Docstring
Sets this CRS by passing it a PROJ.4 style formatted string.

The string will be parsed and the projection and ellipsoid
members set and the remainder of the proj4 string will be stored
in the parameters member. The reason for this is so that we
can easily present the user with 'natural language' representation
of the projection and ellipsoid by looking them up in the srs.db sqlite
database.

We try to match the proj4 string to internal QGIS CRS ID using the following logic:

- perform a whole text search on proj4 string (if not null)
- if not match is found, split proj4 into individual parameters and try to find
a match where the parameters are in a different order
- if none of the above match, use findMatchingProj()

:param projString: A proj4 format string

:return: True on success else false

.. note::

   Some members may be left blank if no match can be found in CRS database.

.. note::

   this method uses an internal cache. Call invalidateCache() to clear the cache.

.. seealso:: :py:func:`fromProj4`
%End

    bool createFromString( const QString &definition );
%Docstring
Set up this CRS from a string definition.

It supports the following formats:
- "EPSG:<code>" - handled with createFromOgcWms()
- "POSTGIS:<srid>" - handled with createFromSrid()
- "INTERNAL:<srsid>" - handled with createFromSrsId()
- "PROJ4:<proj4>" - handled with createFromProj4()
- "WKT:<wkt>" - handled with createFromWkt()

If no prefix is specified, WKT definition is assumed.

:param definition: A String containing a coordinate reference system definition.

:return: True on success else false
%End

    bool createFromUserInput( const QString &definition );
%Docstring
Set up this CRS from various text formats.

Valid formats: WKT string, "EPSG:n", "EPSGA:n", "AUTO:proj_id,unit_id,lon0,lat0",
"urn:ogc:def:crs:EPSG.n", PROJ.4 string, filename (with WKT, XML or PROJ.4 string),
well known name (such as NAD27, NAD83, WGS84 or WGS72),
ESRI.[WKT string] (directly or in a file), "IGNF:xxx"

For more details on supported formats see OGRSpatialReference.SetFromUserInput()
( http://www.gdal.org/ogr/classOGRSpatialReference.html#aec3c6a49533fe457ddc763d699ff8796 )

:param definition: A String containing a coordinate reference system definition.

:return: True on success else false

.. note::

   this function generates a WKT string using OSRSetFromUserInput() and
passes it to createFromWkt() function.
/    // TODO QGIS3: rename to createFromStringOGR so it is clear it's similar to createFromString, just different backend
%End

    static void setupESRIWktFix();
%Docstring
Make sure that ESRI WKT import is done properly.
This is required for proper shapefile CRS import when using gdal>= 1.9.

.. note::

   This function is called by createFromUserInput() and QgsOgrProvider.crs(), there is usually
no need to call it from elsewhere.

.. note::

   This function sets CPL config option GDAL_FIX_ESRI_WKT to a proper value,
unless it has been set by the user through the commandline or an environment variable.
For more details refer to OGRSpatialReference.morphFromESRI() .
%End

    bool isValid() const;
%Docstring
Returns whether this CRS is correctly initialized and usable
%End

    void validate();
%Docstring
Perform some validation on this CRS. If the CRS doesn't validate the
default behavior settings for layers with unknown CRS will be
consulted and acted on accordingly. By hell or high water this
method will do its best to make sure that this CRS is valid - even
if that involves resorting to a hard coded default of geocs:wgs84.

.. note::

   It is not usually necessary to use this function, unless you
are trying to force this CRS to be valid.

.. seealso:: :py:func:`setCustomCrsValidation`
%End

    long findMatchingProj();
%Docstring
Walks the CRS databases (both system and user database) trying to match
stored PROJ.4 string to a database entry in order to fill in further
pieces of information about CRS.

.. note::

   The ellipsoid and projection acronyms must be set as well as the proj4string!

:return: long the SrsId of the matched CRS, zero if no match was found
/    // TODO QGIS 3: seems completely obsolete now (only compares proj4 - already done in createFromProj4)
%End

    bool operator==( const QgsCoordinateReferenceSystem &srs ) const;

    bool operator!=( const QgsCoordinateReferenceSystem &srs ) const;

    bool readXml( const QDomNode &node );
%Docstring
Restores state from the given DOM node.
If it fails or if the node is empty, a default not empty CRS will be returned.

:param node: The node from which state will be restored

:return: bool True on success, False on failure
%End

    bool writeXml( QDomNode &node, QDomDocument &doc ) const;
%Docstring
Stores state to the given Dom node in the given document.

:param node: The node in which state will be restored
:param doc: The document in which state will be stored

:return: bool True on success, False on failure
%End





    long srsid() const;
%Docstring
Returns the internal CRS ID, if available.

:return: the internal sqlite3 srs.db primary key for this CRS
%End

    long postgisSrid() const;
%Docstring
Returns PostGIS SRID for the CRS.

:return: the PostGIS spatial_ref_sys identifier for this CRS (defaults to 0)
/    // TODO QGIS 3: remove unless really necessary - let's use EPSG codes instead
%End

    QString authid() const;
%Docstring
Returns the authority identifier for the CRS.

The identifier includes both the authority (e.g., EPSG) and the CRS number (e.g., 4326).
This is the best method to use when showing a very short CRS identifier to a user,
e.g., "EPSG:4326".

If CRS object is a custom CRS (not found in database), the method will return
internal QGIS CRS ID with "QGIS" authority, for example "QGIS:100005"

:return: the authority identifier for this CRS

.. seealso:: :py:func:`description`
%End

    QString description() const;
%Docstring
Returns the descriptive name of the CRS, e.g., "WGS 84" or "GDA 94 / Vicgrid94". In most
cases this is the best method to use when showing a friendly identifier for the CRS to a
user.

:return: descriptive name of the CRS

.. note::

   an empty string will be returned if the description is not available for the CRS

.. seealso:: :py:func:`authid`
%End

    QString projectionAcronym() const;
%Docstring
Returns the projection acronym for the projection used by the CRS.

:return: the official proj4 acronym for the projection family

.. note::

   an empty string will be returned if the projectionAcronym is not available for the CRS

.. seealso:: :py:func:`ellipsoidAcronym`
%End

    QString ellipsoidAcronym() const;
%Docstring
Returns the ellipsoid acronym for the ellipsoid used by the CRS.

:return: the official proj4 acronym for the ellipoid

.. note::

   an empty string will be returned if the ellipsoidAcronym is not available for the CRS

.. seealso:: :py:func:`projectionAcronym`
%End

    QString toWkt() const;
%Docstring
Returns a WKT representation of this CRS.

:return: string containing WKT of the CRS

.. seealso:: :py:func:`toProj4`
%End

    QString toProj4() const;
%Docstring
Returns a Proj4 string representation of this CRS.

If proj and ellps keys are found in the parameters,
they will be stripped out and the projection and ellipsoid acronyms will be
overridden with these.

:return: Proj4 format string that defines this CRS.

.. note::

   an empty string will be returned if the CRS could not be represented by a Proj4 string

.. seealso:: :py:func:`toWkt`
%End

    bool isGeographic() const;
%Docstring
Returns whether the CRS is a geographic CRS (using lat/lon coordinates)

:return: true if CRS is geographic, or false if it is a projected CRS
%End

    bool hasAxisInverted() const;
%Docstring
Returns whether axis is inverted (e.g., for WMS 1.3) for the CRS.

:return: true if CRS axis is inverted
%End

    QgsUnitTypes::DistanceUnit mapUnits() const;
%Docstring
Returns the units for the projection used by the CRS.
%End

    QgsRectangle bounds() const;
%Docstring
Returns the approximate bounds for the region the CRS is usable within.

The returned bounds represent the latitude and longitude extent for the
projection in the WGS 84 CRS.

.. versionadded:: 3.0
%End


    void setValidationHint( const QString &html );
%Docstring
Set user hint for validation
%End

    QString validationHint();
%Docstring
Get user hint for validation
%End

    static int syncDatabase();
%Docstring
Update proj.4 parameters in our database from proj.4

:return: number of updated CRS on success and
negative number of failed updates in case of errors.

.. note::

   This is used internally and should not be necessary to call in client code
%End


    long saveAsUserCrs( const QString &name );
%Docstring
Save the proj4-string as a custom CRS.

Returns the new CRS srsid(), or -1 if the CRS could not be saved.
%End

    QString geographicCrsAuthId() const;
%Docstring
Returns auth id of related geographic CRS
%End

    static QStringList recentProjections();
%Docstring
Returns a list of recently used projections

:return: list of srsid for recently used projections

.. versionadded:: 2.7
%End

    static void invalidateCache();
%Docstring
Clears the internal cache used to initialize QgsCoordinateReferenceSystem objects.
This should be called whenever the srs database has been modified in order to ensure
that outdated CRS objects are not created.

.. versionadded:: 3.0
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatereferencesystem.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
