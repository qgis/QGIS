
class QgsRelationReferenceWidgetWrapper : QgsEditorWidgetWrapper
{
%TypeHeaderCode
#include <qgsrelationreferencewidgetwrapper.h>
%End

  public:
    explicit QgsRelationReferenceWidgetWrapper( QgsVectorLayer* vl,
        int fieldIdx,
        QWidget* editor,
        QgsMapCanvas* canvas,
        QgsMessageBar* messageBar,
        QWidget* parent /TransferThis/ = 0 );

    virtual QWidget* createWidget( QWidget* parent );
    virtual void initWidget( QWidget* editor );
    virtual QVariant value();

  public slots:
    virtual void setValue( const QVariant& value );
    virtual void setEnabled( bool enabled );
};
