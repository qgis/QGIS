class QgsDoubleSpinBox : QDoubleSpinBox
{
%TypeHeaderCode
#include <qgsdoublespinbox.h>
%End

  public:
    explicit QgsDoubleSpinBox( QWidget *parent /TransferThis/ = 0 );

    //! determines if the widget will show a clear button
    //! @note the clear button will set the widget to its minimum value
    void setShowClearButton( const bool showClearButton );
    bool showClearButton() const;

    //! Set the current value to the minimum
    virtual void clear();

  protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void changeEvent( QEvent* event );
};
