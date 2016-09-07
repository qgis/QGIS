/** \class QgsFilterLineEdit
 * \ingroup gui
 * QLineEdit subclass with built in support for clearing the widget's value and
 * handling custom null value representations.
 *
 * When using QgsFilterLineEdit the value(), setValue() and clearValue() methods should be used
 * instead of QLineEdit's text(), setText() and clear() methods, and the valueChanged()
 * signal should be used instead of textChanged().
 **/
class QgsFilterLineEdit : QLineEdit
{
%TypeHeaderCode
#include <qgsfilterlineedit.h>
%End
  public:

    /** Constructor for QgsFilterLineEdit.
     * @param parent parent widget
     * @param nullValue string for representing null values
     */
    QgsFilterLineEdit( QWidget* parent /TransferThis/ = 0, const QString& nullValue = QString::null );

    /** Sets the string representation for null values in the widget. This does not
     * affect the values returned for null values by value(), rather it only affects
     * the text that is shown to users when the widget's value is null.
     * @param nullValue string to show when widget's value is null
     * @see nullValue()
     */
    void setNullValue( const QString& nullValue );

    /** Returns the string used for representating null values in the widget.
     * @see setNullValue()
     * @see isNull()
     */
    QString nullValue() const;

    /**
     * Sets the current text for the widget with support for handling null values.
     *
     * @param value The text to set. If a null string is provided, the text shown in the
     * widget will be set to the current nullValue().
     * @see value()
     */
    void setValue( const QString& value );

    /**
     * Returns the text of this edit with support for handling null values. If the text
     * in the widget matches the current nullValue() then the returned value will be
     * a null string.
     *
     * @return Current text (or null string if it matches the nullValue() property )
     * @see setValue()
     */
    QString value() const;

    /**
     * Determine if the current text represents null.
     *
     * @return True if the widget's value is null.
     * @see nullValue()
     */
    bool isNull() const;

  public slots:

    /** Clears the widget and resets it to the null value.
     * @see nullValue()
     */
    virtual void clearValue();

  signals:

    /** Emitted when the widget is cleared
     * @see clearValue()
     */
    void cleared();

    /**
     * Same as textChanged() but with support for null values.
     *
     * @param value The current text or null string if it matches the nullValue() property.
     */
    void valueChanged( const QString& value );

  protected:
    void mousePressEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );
    void focusInEvent( QFocusEvent* e );
    void paintEvent( QPaintEvent* e );
    void leaveEvent( QEvent* e );
};
