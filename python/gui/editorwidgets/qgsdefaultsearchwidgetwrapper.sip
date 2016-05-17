/**
 * Wraps a search widget. Default form is just a QgsLineFilterEdit
 */
class QgsDefaultSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%TypeHeaderCode
#include <qgsdefaultsearchwidgetwrapper.h>
%End
  public:

    explicit QgsDefaultSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ = nullptr );

    // QgsSearchWidgetWrapper interface
  public:
    QString expression();
    bool applyDirectly();
    FilterFlags supportedFlags() const;
    FilterFlags defaultFlags() const;
    virtual QString createExpression( FilterFlags flags ) const;

  public slots:

    virtual void clearWidget();
    virtual void setEnabled( bool enabled );

  protected slots:
    void setExpression( QString exp );

  protected:
    QWidget* createWidget( QWidget* parent );
    void initWidget( QWidget* editor );
    bool valid() const;

    /** Returns a pointer to the line edit part of the widget.
     * @note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QgsFilterLineEdit* lineEdit();

    /** Returns a pointer to the case sensitivity check box in the widget.
     * @note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QCheckBox* caseSensitiveCheckBox();

};
