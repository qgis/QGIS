class QgsDoubleSpinBox : QDoubleSpinBox
{
%TypeHeaderCode
#include <qgsdoublespinbox.h>
%End

  public:
    enum ClearValue
    {
      MinimumValue,
      MaximumValue,
      CustomValue
    };

    explicit QgsDoubleSpinBox( QWidget *parent /TransferThis/ = 0 );

    //! determines if the widget will show a clear button
    //! @note the clear button will set the widget to its minimum value
    void setShowClearButton( const bool showClearButton );
    bool showClearButton() const;

    //! Set the current value to the value defined by the clear value.
    virtual void clear();

    /**
     * @brief setClearValue defines if the clear value should be the minimum or maximum values of the widget or a custom value
     * @param customValue if type is CustomValue, defines the numerical value used as the clear value
     */
    void setClearValue( ClearValue type, double customValue = 0 );
    //! returns the value used when clear() is called.
    double clearValue();

  protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void changeEvent( QEvent* event );
};
