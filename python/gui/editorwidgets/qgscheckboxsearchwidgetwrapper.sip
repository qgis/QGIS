/** \ingroup gui
 * \class QgsCheckboxSearchWidgetWrapper
 * Wraps a checkbox edit widget for searching.
 * \note Added in version 2.16
 */

class QgsCheckboxSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgscheckboxsearchwidgetwrapper.h>
%End
  public:

    /** Constructor for QgsCheckboxSearchWidgetWrapper.
     * @param vl associated vector layer
     * @param fieldIdx index of associated field
     * @param parent parent widget
     */
    explicit QgsCheckboxSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );

    /** Returns a variant representing the current state of the widget.
     * @note this will not be a boolean true or false value, it will instead
     * be the values configured to represent checked and unchecked states in
     * the editor widget configuration.
     */
    QVariant value() const;

    // QgsSearchWidgetWrapper interface
  public:
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
