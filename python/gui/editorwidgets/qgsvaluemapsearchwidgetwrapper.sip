/**
 * Wraps a value map search widget. This widget will offer a combobox with values from another layer
 * referenced by a foreign key (a constraint may be set but is not required on data level).
 * It will be used as a search widget and produces expression to look for in the layer.
 */

class QgsValueMapSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgsvaluemapsearchwidgetwrapper.h>
%End
  public:

    explicit QgsValueMapSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );
    bool applyDirectly();
    QString expression();
    bool valid() const;
    FilterFlags supportedFlags() const;
    FilterFlags defaultFlags() const;
    virtual QString createExpression( FilterFlags flags ) const;

  public slots:

    virtual void clearWidget();
    virtual void setEnabled( bool enabled );

  protected:
    QWidget* createWidget( QWidget* parent );
    void initWidget( QWidget* editor );

  protected slots:
    void setExpression( QString exp );

};
