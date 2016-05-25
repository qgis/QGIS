/** \ingroup gui
 * \class QgsDateTimeSearchWidgetWrapper
 * Wraps a date/time edit widget for searching.
 * \note Added in version 2.16
 */

class QgsDateTimeSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgsdatetimesearchwidgetwrapper.h>
%End
  public:

  public:

    /** Constructor for QgsDateTimeSearchWidgetWrapper.
     * @param vl associated vector layer
     * @param fieldIdx index of associated field
     * @param parent parent widget
     */
    explicit QgsDateTimeSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );

    /** Returns a variant representing the current state of the widget, respecting
     * the editor widget's configured field format for date/time values.
     */
    bool valid() const;

    bool applyDirectly();
    QString expression();
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
