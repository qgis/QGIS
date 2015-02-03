class QgsVectorGradientColorRampV2Dialog : QDialog
{
%TypeHeaderCode
#include <qgsvectorgradientcolorrampv2dialog.h>
%End

  public:
    QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent = NULL );

  public slots:
    void setColor1( const QColor& color );
    void setColor2( const QColor& color );

    void toggledStops( bool on );
    void addStop();
    void removeStop();

    void stopDoubleClicked( QTreeWidgetItem* item, int column );
    void setItemStopColor( const QColor& newColor );

  protected slots:
    void on_cboType_currentIndexChanged( int index );
    void on_btnInformation_pressed();

  protected:

    void updateStops();
    void updatePreview();
    void setStopColor( QTreeWidgetItem* item, QColor color );
};
