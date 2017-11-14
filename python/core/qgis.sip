/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgis.h                                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





%ModuleHeaderCode
#include <qgis.h>
%End

%ModuleCode
int QgisEvent = QEvent::User + 1;
%End


class Qgis
{
%Docstring
 The Qgis class provides global constants for use throughout the application.
%End

%TypeHeaderCode
#include "qgis.h"
%End
  public:
    static const QString QGIS_VERSION;
%Docstring
Version string
%End
    static const int QGIS_VERSION_INT;
%Docstring
Version number used for comparing versions using the "Check QGIS Version" function
%End
    static const QString QGIS_RELEASE_NAME;
%Docstring
Release name
%End
    static const char *QGIS_DEV_VERSION;
%Docstring
The development version
%End


    enum DataType
    {
      UnknownDataType,
      Byte,
      UInt16,
      Int16,
      UInt32,
      Int32,
      Float32,
      Float64,
      CInt16,
      CInt32,
      CFloat32,
      CFloat64,
      ARGB32,
      ARGB32_Premultiplied
    };

    static const double DEFAULT_SEARCH_RADIUS_MM;
%Docstring
 Identify search radius in mm
.. versionadded:: 2.3
%End

    static const float DEFAULT_MAPTOPIXEL_THRESHOLD;
%Docstring
Default threshold between map coordinates and device coordinates for map2pixel simplification
%End

    static const QColor DEFAULT_HIGHLIGHT_COLOR;
%Docstring
 Default highlight color.  The transparency is expected to only be applied to polygon
  fill. Lines and outlines are rendered opaque.
.. versionadded:: 2.3
%End

    static const double DEFAULT_HIGHLIGHT_BUFFER_MM;
%Docstring
 Default highlight buffer in mm.
.. versionadded:: 2.3
%End

    static const double DEFAULT_HIGHLIGHT_MIN_WIDTH_MM;
%Docstring
 Default highlight line/stroke minimum width in mm.
.. versionadded:: 2.3
%End

    static const double SCALE_PRECISION;
%Docstring
 Fudge factor used to compare two scales. The code is often going from scale to scale
  denominator. So it looses precision and, when a limit is inclusive, can lead to errors.
  To avoid that, use this factor instead of using <= or >=.
.. versionadded:: 2.15
%End

    static const double DEFAULT_Z_COORDINATE;
%Docstring
 Default Z coordinate value for 2.5d geometry
  This value have to be assigned to the Z coordinate for the new 2.5d geometry vertex.
.. versionadded:: 3.0
%End

    static const double UI_SCALE_FACTOR;
%Docstring
 UI scaling factor. This should be applied to all widget sizes obtained from font metrics,
 to account for differences in the default font sizes across different platforms.
.. versionadded:: 3.0
%End

};





uint qHash( const QVariant &variant );
%Docstring
Hash for QVariant
 :rtype: uint
%End

QString qgsDoubleToString( double a, int precision = 17 );
%Docstring
 Returns a string representation of a double
 \param a double value
 \param precision number of decimal places to retain
 :rtype: str
%End

bool qgsDoubleNear( double a, double b, double epsilon = 4 * DBL_EPSILON );
%Docstring
 Compare two doubles (but allow some difference)
 \param a first double
 \param b second double
 \param epsilon maximum difference allowable between doubles
 :rtype: bool
%End

bool qgsFloatNear( float a, float b, float epsilon = 4 * FLT_EPSILON );
%Docstring
 Compare two floats (but allow some difference)
 \param a first float
 \param b second float
 \param epsilon maximum difference allowable between floats
 :rtype: bool
%End

bool qgsDoubleNearSig( double a, double b, int significantDigits = 10 );
%Docstring
Compare two doubles using specified number of significant digits
 :rtype: bool
%End

double qgsRound( double number, double places );
%Docstring
 Returns a double ``number``, rounded (as close as possible) to the specified number of ``places``.

.. versionadded:: 3.0
 :rtype: float
%End



double qgsPermissiveToDouble( QString string, bool &ok );
%Docstring
 Converts a string to a double in a permissive way, e.g., allowing for incorrect
 numbers of digits between thousand separators
 \param string string to convert
 \param ok will be set to true if conversion was successful
 :return: string converted to double if possible
.. versionadded:: 2.9
.. seealso:: permissiveToInt
 :rtype: float
%End

int qgsPermissiveToInt( QString string, bool &ok );
%Docstring
 Converts a string to an integer in a permissive way, e.g., allowing for incorrect
 numbers of digits between thousand separators
 \param string string to convert
 \param ok will be set to true if conversion was successful
 :return: string converted to int if possible
.. versionadded:: 2.9
.. seealso:: permissiveToDouble
 :rtype: int
%End

bool qgsVariantLessThan( const QVariant &lhs, const QVariant &rhs );
%Docstring
 Compares two QVariant values and returns whether the first is less than the second.
 Useful for sorting lists of variants, correctly handling sorting of the various
 QVariant data types (such as strings, numeric values, dates and times)
.. seealso:: qgsVariantGreaterThan()
 :rtype: bool
%End

bool qgsVariantGreaterThan( const QVariant &lhs, const QVariant &rhs );
%Docstring
 Compares two QVariant values and returns whether the first is greater than the second.
 Useful for sorting lists of variants, correctly handling sorting of the various
 QVariant data types (such as strings, numeric values, dates and times)
.. seealso:: qgsVariantLessThan()
 :rtype: bool
%End

QString qgsVsiPrefix( const QString &path );
%Docstring
 :rtype: str
%End




const QString GEOWKT;
const QString PROJECT_SCALES;

const QString GEOPROJ4;
const long GEOSRID;
const long GEOCRS_ID;
const long GEO_EPSG_CRS_ID;
const QString GEO_EPSG_CRS_AUTHID;

const int USER_CRS_START_ID;

const QString GEO_NONE;


const double DEFAULT_POINT_SIZE;
const double DEFAULT_LINE_WIDTH;

const double DEFAULT_SEGMENT_EPSILON;


typedef unsigned long long qgssize;





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgis.h                                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
