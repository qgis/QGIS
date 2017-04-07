/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgssvgannotation.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsSvgAnnotation: QgsAnnotation
{
%Docstring
 An annotation which renders the contents of an SVG file.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssvgannotation.h"
%End
  public:

    QgsSvgAnnotation( QObject *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsSvgAnnotation.
%End

    virtual void writeXml( QDomElement &elem, QDomDocument &doc ) const;
    virtual void readXml( const QDomElement &itemElem, const QDomDocument &doc );

    void setFilePath( const QString &file );
%Docstring
 Sets the file path for the source SVG file.
 \see filePath()
%End

    QString filePath() const;
%Docstring
 Returns the file path for the source SVG file.
 \see setFilePath()
%End

    static QgsSvgAnnotation *create() /Factory/;
%Docstring
 Returns a new QgsSvgAnnotation object.
%End

  protected:

    void renderAnnotation( QgsRenderContext &context, QSizeF size ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgssvgannotation.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
