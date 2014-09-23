
/** LineEdit with builtin clear button
 */
class QgsFilterLineEdit : QLineEdit
{
%TypeHeaderCode
#include <qgsfilterlineedit.h>
%End
  public:
    QgsFilterLineEdit( QWidget* parent = 0 );

    /**
     * Sets the current text with NULL support
     *
     * @param value The text to set. If a Null string is provided, the text will match the nullValue.
     */
    void setValue( QString value );

    /**
     * Returns the text of this edit with NULL support
     *
     * @return Current text (Null string if it matches the nullValue property )
     */
    QString value() const;

    /**
     * Determine if the current text represents Null.
     *
     * @return True if the value is Null.
     */
    bool isNull() const;

  signals:
    void cleared();

    /**
     * Same as textChanged(const QString& ) but with support for Null values.
     *
     * @param value The current text or Null string if it matches the nullValue property.
     */
    void valueChanged( const QString& value );

};
