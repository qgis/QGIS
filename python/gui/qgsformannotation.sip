class QgsFormAnnotation : QgsAnnotation
{
%TypeHeaderCode
#include <qgsformannotation.h>
%End
  public:

    QgsFormAnnotation( QObject* parent /TransferThis/ = nullptr );

    QSizeF minimumFrameSize() const;
    QSizeF preferredFrameSize() const;

    void setDesignerForm( const QString& uiFile );
    QString designerForm() const;

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc );

    void setAssociatedFeature( const QgsFeature& feature );

    static QgsFormAnnotation* create() /Factory/;

  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const;

};
