
class QgsRasterFormatSaveOptionsWidget : QWidget
{
%TypeHeaderCode
#include <qgsrasterformatsaveoptionswidget.h>
%End
 
  public:
    QgsRasterFormatSaveOptionsWidget( QWidget* parent = 0, QString format = "GTiff", QString provider = "gdal" );
    ~QgsRasterFormatSaveOptionsWidget();

    void setFormat( QString format );
    void setProvider( QString provider );
    QStringList options() const;
    void showProfileButtons( bool show = true );

  public slots:

    void apply();
    void helpOptions();
    bool validateOptions( bool gui = true );

  private slots:

    void on_mProfileNewButton_clicked();
    void on_mProfileDeleteButton_clicked();
    void on_mProfileResetButton_clicked();
    void on_mOptionsAddButton_clicked();
    void on_mOptionsDeleteButton_clicked();
    void on_mOptionsLabel_clicked(); 
    void on_mOptionsLineEdit_editingFinished();
    void optionsTableChanged();
    void optionsTableEnableDeleteButton();
    void updateOptions();

  private:

    QString settingsKey( QString profile ) const;
    QString currentProfileKey() const;
    QString createOptions( QString profile ) const;
    void deleteCreateOptions( QString profile );
    void setCreateOptions( );
    void setCreateOptions( QString profile, QString options );
    void setCreateOptions( QString profile, QStringList list );
    QStringList profiles() const;
    void updateProfiles();
    bool eventFilter(QObject *obj, QEvent *event);

};
