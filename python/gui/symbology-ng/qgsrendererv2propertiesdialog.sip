class QgsRendererV2PropertiesDialog : QDialog
{
%TypeHeaderCode
#include <qgsrendererv2propertiesdialog.h>
%End

  public:
    QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded = false );

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    void apply();
    void onOK();

  protected:

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

};
