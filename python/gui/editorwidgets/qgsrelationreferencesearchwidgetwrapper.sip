
/** \ingroup gui
 * \class QgsRelationReferenceSearchWidgetWrapper
 * Wraps a relation reference search widget.
 * \note Added in version 2.16
 */

class QgsRelationReferenceSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgsrelationreferencesearchwidgetwrapper.h>
%End
  public:

    explicit QgsRelationReferenceSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QgsMapCanvas* canvas, QWidget* parent = nullptr );
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
    void onValueChanged( QVariant value );

  protected slots:
    void setExpression( QString exp );

};

