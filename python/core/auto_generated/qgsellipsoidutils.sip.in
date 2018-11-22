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

      double semiMajor;
      double semiMinor;

      bool useCustomParameters;

      double inverseFlattening;

      QgsCoordinateReferenceSystem crs;
    };

    struct EllipsoidDefinition
    {
      QString acronym;
      QString description;
      QgsEllipsoidUtils::EllipsoidParameters parameters;
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

.. seealso:: :py:func:`acronyms`
%End

    static QStringList acronyms();
%Docstring
Returns a list of all known ellipsoid acronyms from the internal
ellipsoid database.

.. seealso:: :py:func:`definitions`
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsellipsoidutils.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
