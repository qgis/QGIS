class QgsNewVectorLayerDialog : QDialog
{
%TypeHeaderCode
#include <qgsnewvectorlayerdialog.h>
%End

  public:

    // run the dialog, create the layer.
    // @return fileName on success, empty string use aborted, QString::null if creation failed
    static QString runAndCreateLayer( QWidget* parent = 0, QString* enc = 0 );

    QgsNewVectorLayerDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsNewVectorLayerDialog();
    /** Returns the selected geometry type*/
    QGis::WkbType selectedType() const;
    /** Appends the chosen attribute names and types to at*/
    void attributes( QList< QPair<QString, QString> >& at ) const;
    /** Returns the file format for storage*/
    QString selectedFileFormat() const;
    /** Returns the file format for storage*/
    QString selectedFileEncoding() const;
    /** Returns the selected crs id*/
    int selectedCrsId() const;

  protected slots:
    void on_mAddAttributeButton_clicked();
    void on_mRemoveAttributeButton_clicked();
    void on_mFileFormatComboBox_currentIndexChanged( int index );
    void on_mTypeBox_currentIndexChanged( int index );
    void on_buttonBox_helpRequested();
    void nameChanged( QString );
    void selectionChanged();
};
