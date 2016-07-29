class QgsBrushStyleComboBox : QComboBox
{
%TypeHeaderCode
#include <qgsbrushstylecombobox.h>
%End

  public:
    QgsBrushStyleComboBox( QWidget* parent /TransferThis/ = NULL );

    Qt::BrushStyle brushStyle() const;

    void setBrushStyle( Qt::BrushStyle style );

  protected:
    QIcon iconForBrush( Qt::BrushStyle style );

};
