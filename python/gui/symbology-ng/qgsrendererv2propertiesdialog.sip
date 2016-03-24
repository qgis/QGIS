class QgsRendererV2PropertiesDialog : QDialog
{
%TypeHeaderCode
#include <qgsrendererv2propertiesdialog.h>
%End

  public:
    QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded = false );
    ~QgsRendererV2PropertiesDialog();

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    void apply();
    void onOK();

  protected:

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

};
