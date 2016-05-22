/**
 * Wraps a checkbox search widget.
 */
class QgsCheckboxSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgscheckboxsearchwidgetwrapper.h>
%End
  public:

    explicit QgsCheckboxSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );

    // QgsSearchWidgetWrapper interface
  public:
    bool applyDirectly();
    QString expression();
    bool valid() const;
    QVariant value() const;
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
