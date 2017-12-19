/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsprintlayout.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsPrintLayout : QgsLayout
{
%Docstring
 Print layout, a QgsLayout subclass for static or atlas-based layouts.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprintlayout.h"
%End
  public:

    QgsPrintLayout( QgsProject *project );
%Docstring
Constructor for QgsPrintLayout.
%End

    QgsLayoutAtlas *atlas();
%Docstring
Returns the print layout's atlas.
%End

    virtual QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsprintlayout.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
