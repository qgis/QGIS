
/** LineEdit with builtin clear button
 */
class QgsFilterLineEdit : QLineEdit
{
%TypeHeaderCode
#include <qgsfilterlineedit.h>
%End

  public:
    QgsFilterLineEdit( QWidget* parent = 0 );

  signals:
    void cleared();

  protected:
    void resizeEvent( QResizeEvent * );
    void changeEvent( QEvent * );
};
