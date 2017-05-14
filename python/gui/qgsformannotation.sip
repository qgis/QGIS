class QgsFormAnnotation : QgsAnnotation
{
%TypeHeaderCode
#include <qgsformannotation.h>
%End
  public:

    QgsFormAnnotation( QObject *parent /TransferThis/ = 0 );

    virtual QgsFormAnnotation *clone() const /Factory/;

    QSizeF minimumFrameSize() const;
    QSizeF preferredFrameSize() const;

    void setDesignerForm( const QString &uiFile );
    QString designerForm() const;

    virtual void writeXml( QDomElement& elem, QDomDocument & doc, const QgsReadWriteContext &context ) const;
    virtual void readXml( const QDomElement& itemElem, const QgsReadWriteContext &context );

    void setAssociatedFeature( const QgsFeature &feature );

    static QgsFormAnnotation *create() /Factory/;

  protected:

    void renderAnnotation( QgsRenderContext &context, QSizeF size ) const;

};
