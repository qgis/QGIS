
/** \ingroup gui
 * A combobox which lets the user select map scale from predefined list
 * and highlights nearest to current scale value
 **/
class QgsScaleWidget : QWidget
{
%TypeHeaderCode
#include <qgsscalewidget.h>
%End

  public:
    QgsScaleWidget( QWidget* parent /TransferThis/ = 0 );
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
    bool setScaleString( const QString& scaleTxt );
    //! Function to read the selected scale as double
    double scale() const;
    //! Function to set the selected scale from double
    void setScale( double scale );
    //! Function to read the min scale
    double minScale() const;

    //! Helper function to convert a double to scale string
    // Performs rounding, so an exact representation is not to
    // be expected.
    static QString toString( double scale );
    //! Helper function to convert a scale string to double
    static double toDouble( const QString& scaleString, bool *ok = 0 );

  public slots:
    void updateScales( const QStringList &scales = QStringList() );

    //! assign the current scale from the map canvas
    void setScaleFromCanvas();

    //! Function to set the min scale
    void setMinScale( double scale );

  signals:
    //! Signal is emitted when *user* has finished editing/selecting a new scale.
    void scaleChanged( double scale );


};

