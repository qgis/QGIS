
/** A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale valu
 */
class QgsScaleComboBox : QComboBox
{
%TypeHeaderCode
#include <qgsscalecombobox.h>
%End

 public:
    QgsScaleComboBox(QWidget * parent = 0);
    ~QgsScaleComboBox();
};

