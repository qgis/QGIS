/**
 * Wraps a date time search widget.
 */
class QgsDateTimeSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgsdatetimesearchwidgetwrapper.h>
%End
  public:

  public:
    explicit QgsDateTimeSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );
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
