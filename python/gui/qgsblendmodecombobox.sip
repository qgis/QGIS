/** \ingroup gui
 * A combobox which lets the user select blend modes from a predefined list
 **/
class QgsBlendModeComboBox : QComboBox
{
%TypeHeaderCode
#include <qgsblendmodecombobox.h>
%End
  public:
    QgsBlendModeComboBox( QWidget* parent = 0 );
    virtual ~QgsBlendModeComboBox();

    //! Function to read the selected blend mode as QPainter::CompositionMode
    QPainter::CompositionMode blendMode();
    //! Function to set the selected blend mode from QPainter::CompositionMode
    void setBlendMode( QPainter::CompositionMode blendMode );

  public slots:
    void updateModes();

};
