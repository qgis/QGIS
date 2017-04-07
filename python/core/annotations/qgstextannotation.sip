/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgstextannotation.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsTextAnnotation: QgsAnnotation
{
%Docstring
 An annotation item that displays formatted text from a QTextDocument document.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextannotation.h"
%End
  public:

    QgsTextAnnotation( QObject *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsTextAnnotation.
%End

    const QTextDocument *document() const;
%Docstring
 Returns the text document which will be rendered
 within the annotation.
 \see setDocument()
%End

    void setDocument( const QTextDocument *doc );
%Docstring
 Sets the text document which will be rendered
 within the annotation. Ownership is not transferred.
 \see document()
%End

    virtual void writeXml( QDomElement &elem, QDomDocument &doc ) const;
    virtual void readXml( const QDomElement &itemElem, const QDomDocument &doc );

    static QgsTextAnnotation *create() /Factory/;
%Docstring
 Returns a new QgsTextAnnotation object.
%End

  protected:

    void renderAnnotation( QgsRenderContext &context, QSizeF size ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgstextannotation.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
