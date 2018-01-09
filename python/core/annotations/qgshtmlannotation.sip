/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgshtmlannotation.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsHtmlAnnotation: QgsAnnotation
{
%Docstring
An annotation item that embeds HTML content.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgshtmlannotation.h"
%End
  public:

    QgsHtmlAnnotation( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsHtmlAnnotation.
%End

    virtual QgsHtmlAnnotation *clone() const /Factory/;


    virtual QSizeF minimumFrameSize() const;


    void setSourceFile( const QString &htmlFile );
%Docstring
Sets the file path for the source HTML file.

.. seealso:: :py:func:`sourceFile`
%End

    QString sourceFile() const;
%Docstring
Returns the file path for the source HTML file.

.. seealso:: :py:func:`setSourceFile`
%End

    virtual void writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    virtual void readXml( const QDomElement &itemElem, const QgsReadWriteContext &context );


    virtual void setAssociatedFeature( const QgsFeature &feature );


    static QgsHtmlAnnotation *create() /Factory/;
%Docstring
Returns a new QgsHtmlAnnotation object.
%End

  protected:

    virtual void renderAnnotation( QgsRenderContext &context, QSizeF size ) const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgshtmlannotation.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
