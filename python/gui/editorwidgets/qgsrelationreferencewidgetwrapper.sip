
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
    bool valid() const;

  public slots:
    virtual void setValue( const QVariant& value );
    virtual void setEnabled( bool enabled );

  protected:
    /**
     * This should update the widget with a visual cue if a constraint status
     * changed.
     *
     * By default a stylesheet will be applied on the widget that changes the
     * background color to red.
     *
     * This can be overwritten in subclasses to allow individual widgets to
     * change the visual cue.
     * @note added in QGIS 2.16
     */
    void updateConstraintWidgetStatus( bool constraintValid );
};
