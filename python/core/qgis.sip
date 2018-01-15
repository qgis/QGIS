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
    static const int QGIS_VERSION_INT;
    static const QString QGIS_RELEASE_NAME;
    static const char *QGIS_DEV_VERSION;


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

    static const float DEFAULT_MAPTOPIXEL_THRESHOLD;

    static const QColor DEFAULT_HIGHLIGHT_COLOR;

    static const double DEFAULT_HIGHLIGHT_BUFFER_MM;

    static const double DEFAULT_HIGHLIGHT_MIN_WIDTH_MM;

    static const double SCALE_PRECISION;

    static const double DEFAULT_Z_COORDINATE;

    static const double UI_SCALE_FACTOR;

};





uint qHash( const QVariant &variant );
%Docstring
Hash for QVariant
%End

QString qgsDoubleToString( double a, int precision = 17 );
%Docstring
Returns a string representation of a double

:param a: double value
:param precision: number of decimal places to retain
%End

bool qgsDoubleNear( double a, double b, double epsilon = 4 * DBL_EPSILON );
%Docstring
Compare two doubles (but allow some difference)

:param a: first double
:param b: second double
:param epsilon: maximum difference allowable between doubles
%End

bool qgsFloatNear( float a, float b, float epsilon = 4 * FLT_EPSILON );
%Docstring
Compare two floats (but allow some difference)

:param a: first float
:param b: second float
:param epsilon: maximum difference allowable between floats
%End

bool qgsDoubleNearSig( double a, double b, int significantDigits = 10 );
%Docstring
Compare two doubles using specified number of significant digits
%End

double qgsRound( double number, double places );
%Docstring
Returns a double ``number``, rounded (as close as possible) to the specified number of ``places``.

.. versionadded:: 3.0
%End



double qgsPermissiveToDouble( QString string, bool &ok );
%Docstring
Converts a string to a double in a permissive way, e.g., allowing for incorrect
numbers of digits between thousand separators

:param string: string to convert
:param ok: will be set to true if conversion was successful

:return: string converted to double if possible

.. versionadded:: 2.9

.. seealso:: :py:func:`permissiveToInt`
%End

int qgsPermissiveToInt( QString string, bool &ok );
%Docstring
Converts a string to an integer in a permissive way, e.g., allowing for incorrect
numbers of digits between thousand separators

:param string: string to convert
:param ok: will be set to true if conversion was successful

:return: string converted to int if possible

.. versionadded:: 2.9

.. seealso:: :py:func:`permissiveToDouble`
%End

bool qgsVariantLessThan( const QVariant &lhs, const QVariant &rhs );
%Docstring
Compares two QVariant values and returns whether the first is less than the second.
Useful for sorting lists of variants, correctly handling sorting of the various
QVariant data types (such as strings, numeric values, dates and times)

.. seealso:: :py:func:`qgsVariantGreaterThan`
%End

bool qgsVariantGreaterThan( const QVariant &lhs, const QVariant &rhs );
%Docstring
Compares two QVariant values and returns whether the first is greater than the second.
Useful for sorting lists of variants, correctly handling sorting of the various
QVariant data types (such as strings, numeric values, dates and times)

.. seealso:: :py:func:`qgsVariantLessThan`
%End

QString qgsVsiPrefix( const QString &path );




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
