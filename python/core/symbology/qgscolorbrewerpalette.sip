/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgscolorbrewerpalette.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsColorBrewerPalette
{

%TypeHeaderCode
#include "qgscolorbrewerpalette.h"
%End
  public:
    static QList<QColor> listSchemeColors( const QString &schemeName, int colors );
%Docstring
 :rtype: list of QColor
%End

    static QStringList listSchemes();
%Docstring
 :rtype: list of str
%End

    static QList<int> listSchemeVariants( const QString &schemeName );
%Docstring
 :rtype: list of int
%End

    static const char *BREWER_STRING;
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgscolorbrewerpalette.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
