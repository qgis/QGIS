
class QgsScaleRangeWidget : QWidget
{
%TypeHeaderCode
#include "qgsscalerangewidget.h"
%End

  public:
    explicit QgsScaleRangeWidget( QWidget *parent /TransferThis/ = 0 );
    ~QgsScaleRangeWidget();

    //! set the map canvas which will be used for the current scale buttons
    /**
     * @brief setMapCanvas set the map canvas which will be used for the current scale buttons
     * if not set, the buttons are hidden.
     */
    void setMapCanvas( QgsMapCanvas* mapCanvas );

    //! return the minimum scale
    double minimumScale();

    //! return the maximum scale
    double maximumScale();

    //! return the minimum scale denominator ( = 1 / maximum scale )
    double minimumScaleDenom();

    //! return the maximum scale denominator ( = 1 / minimum scale )
    double maximumScaleDenom();

    //! call to reload the project scales and apply them to the 2 scales combo boxes
    void reloadProjectScales();

  public slots:
    void setMinimumScale( double scale );

    void setMaximumScale( double scale );

    void setScaleRange( double min, double max );

  signals:

    /** Emitted when the scale range set in the widget is changed.
     * @param min minimum scale
     * @param max maximum scale
     * @note added in QGIS 2.16
     */
    void rangeChanged( double min, double max );
};
