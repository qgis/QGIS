class QgsScaleVisibilityDialog : QObject
{
%TypeHeaderCode
#include <qgsscalevisibilitydialog.h>
%End

  public:
    explicit QgsScaleVisibilityDialog( QWidget *parent = 0, QString title = QString(), QgsMapCanvas* mapCanvas = 0 );

    //! return if scale visibilty is enabled
    bool hasScaleVisibility();

    //! return minimum scale (true scale, not scale denominator)
    double minimumScale();

    //! return maximum scale (true scale, not scale denominator)
    double maximumScale();


  public slots:
    //! set if scale visibility is enabled
    void setScaleVisiblity( bool hasScaleVisibility );

    //! set minimum scale (true scale, not scale denominator)
    void setMinimumScale( double minScale );

    //! set maximum scale (true scale, not scale denominator)
    void setMaximumScale( double maxScale );

};
