/**
 * Wraps a value relation search  widget. This widget will offer a combobox with values from another layer
 * referenced by a foreign key (a constraint may be set but is not required on data level).
 * It will be used as a search widget and produces expression to look for in the layer.
 */

class QgsValueRelationSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgsvaluerelationsearchwidgetwrapper.h>
%End
  public:
    explicit QgsValueRelationSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );
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

  public slots:

    //! Called when current value of search widget changes
    void onValueChanged();

  protected slots:
    void setExpression( QString exp );

};
