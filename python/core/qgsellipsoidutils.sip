/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsellipsoidutils.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsEllipsoidUtils
{
%Docstring
Contains utility functions for working with ellipsoids and querying the ellipsoid database.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsellipsoidutils.h"
%End
  public:

    struct EllipsoidParameters
    {
      bool valid;
%Docstring
Whether ellipsoid parameters are valid
%End

      double semiMajor;
%Docstring
Semi-major axis
%End
      double semiMinor;
%Docstring
Semi-minor axis
%End

      bool useCustomParameters;
%Docstring
Whether custom parameters alone should be used (semiMajor/semiMinor only)
%End

      double inverseFlattening;
%Docstring
Inverse flattening
%End

      QgsCoordinateReferenceSystem crs;
%Docstring
Associated coordinate reference system
%End
    };

    struct EllipsoidDefinition
    {
      QString acronym;
%Docstring
Acronym for ellipsoid
%End
      QString description;
%Docstring
Description of ellipsoid
%End
      QgsEllipsoidUtils::EllipsoidParameters parameters;
%Docstring
Ellipsoid parameters
%End
    };

    static EllipsoidParameters ellipsoidParameters( const QString &ellipsoid );
%Docstring
Returns the parameters for the specified ``ellipsoid``.
Results are cached to allow for fast retrieval of parameters.
%End

    static QList< QgsEllipsoidUtils::EllipsoidDefinition > definitions();
%Docstring
Returns a list of the definitions for all known ellipsoids from the
internal ellipsoid database.

.. seealso:: :py:func:`acronyms()`
%End

    static QStringList acronyms();
%Docstring
Returns a list of all known ellipsoid acronyms from the internal
ellipsoid database.

.. seealso:: :py:func:`definitions()`
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsellipsoidutils.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
