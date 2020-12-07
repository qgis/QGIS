/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsformannotation.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFormAnnotation: QgsAnnotation
{
%Docstring
An annotation item that embeds a designer form showing the feature attribute

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsformannotation.h"
%End
  public:

    QgsFormAnnotation( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsFormAnnotation.
%End

    virtual QgsFormAnnotation *clone() const /Factory/;


    virtual QSizeF minimumFrameSize() const;

    QSizeF preferredFrameSize() const;
%Docstring
Returns the optimal frame size
%End

    void setDesignerForm( const QString &uiFile );
%Docstring
Sets the path to the Qt Designer UI file to show in the annotation.

.. seealso:: :py:func:`designerForm`
%End

    QString designerForm() const;
%Docstring
Returns the path to the Qt Designer UI file to show in the annotation.

.. seealso:: :py:func:`setDesignerForm`
%End

    virtual void writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    virtual void readXml( const QDomElement &itemElem, const QgsReadWriteContext &context );


    virtual void setAssociatedFeature( const QgsFeature &feature );


    static QgsFormAnnotation *create() /Factory/;
%Docstring
Returns a new QgsFormAnnotation object.
%End

  protected:

    virtual void renderAnnotation( QgsRenderContext &context, QSizeF size ) const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsformannotation.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
