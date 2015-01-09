
/** A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 */
class QgsScaleWidget : QWidget
{
%TypeHeaderCode
#include <qgsscalewidget.h>
%End

  public:
    QgsScaleWidget( QWidget* parent = 0 );
    ~QgsScaleWidget();

    //! shows a button to set the scale to the current scale of the map canvas next to the combobox
    //! @note the map canvas must be defined to show the button
    void setShowCurrentScaleButton( bool showCurrentScaleButton );
    bool showCurrentScaleButton();

    //! set the map canvas associated to the current button
    void setMapCanvas( QgsMapCanvas* canvas );

    //! Function to read the selected scale as text
    QString scaleString();
    //! Function to set the selected scale from text
    bool setScaleString( QString scaleTxt );
    //! Function to read the selected scale as double
    double scale();
    //! Function to set the selected scale from double
    void setScale( double scale );

    //! Helper function to convert a double to scale string
    // Performs rounding, so an exact representation is not to
    // be expected.
    static QString toString( double scale );
    //! Helper function to convert a scale string to double
    static double toDouble( QString scaleString, bool *ok = NULL );

  signals:
    //! Signal is emitted when *user* has finished editing/selecting a new scale.
    void scaleChanged();

  public slots:
    void updateScales( const QStringList &scales = QStringList() );

};

