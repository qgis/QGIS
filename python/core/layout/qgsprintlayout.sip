/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsprintlayout.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsPrintLayout : QgsLayout, QgsMasterLayoutInterface
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

    virtual QgsPrintLayout *clone() const /Factory/;

    virtual QgsProject *layoutProject() const;

    virtual QgsMasterLayoutInterface::Type layoutType() const;

    virtual QIcon icon() const;


    QgsLayoutAtlas *atlas();
%Docstring
Returns the print layout's atlas.
%End

    virtual QString name() const;
    virtual void setName( const QString &name );


    virtual QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );


    virtual QDomElement writeLayoutXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readLayoutXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );

    virtual QgsExpressionContext createExpressionContext() const;


  signals:

    void nameChanged( const QString &name );
%Docstring
Emitted when the layout's name is changed.

.. seealso:: :py:func:`setName`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgsprintlayout.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
