
class QgsRasterFormatSaveOptionsWidget : QWidget
{
%TypeHeaderCode
#include <qgsrasterformatsaveoptionswidget.h>
%End

  public:

    enum Type
    {
      Default, // everything except profile buttons (save as dlg)
      Full, // everything (options dlg)
      Table, // just table
      LineEdit, // just the line edit
      ProfileLineEdit // Profile + LineEdit
    };

    QgsRasterFormatSaveOptionsWidget( QWidget* parent /TransferThis/ = 0, const QString& format = "GTiff",
                                      QgsRasterFormatSaveOptionsWidget::Type type = Default,
                                      const QString& provider = "gdal" );
    ~QgsRasterFormatSaveOptionsWidget();

    /**
     * Set output raster format, it is used to determine list
     * of available options
     */
    void setFormat( const QString& format );

    /**
     * Set provider key, , it is used to determine list
     * of available options
     */
    void setProvider( const QString& provider );

    /**
     * Set output raster layer
     */
    void setRasterLayer( QgsRasterLayer* rasterLayer );

    /**
     * Set output raster file name
     */
    void setRasterFileName( const QString& file );

    /**
     * Returns list of selected options
     * @see setOptions()
     */
    QStringList options() const;

    /**
     * Populate widget with user-defined options
     * @see options()
     * @note added in QGIS 3.0
     */
    void setOptions( const QString& options );

    /**
     * Set widget look and feel
     */
    void setType( QgsRasterFormatSaveOptionsWidget::Type type = Default );

    /**
     * Set pyramids format to use
     */
    void setPyramidsFormat( QgsRaster::RasterPyramidsFormat format );

  public slots:

    void apply();

    /**
     * Opens window with options desctiption for given provider
     * and output format
     */
    void helpOptions();

    /**
     * Validates options correctness
     */
    QString validateOptions( bool gui = true, bool reportOk = true );

    /**
     * Reloads profiles list from QGIS settings
     */
    void updateProfiles();

  private slots:

    void on_mProfileNewButton_clicked();
    void on_mProfileDeleteButton_clicked();
    void on_mProfileResetButton_clicked();
    void on_mOptionsAddButton_clicked();
    void on_mOptionsDeleteButton_clicked();
    void on_mOptionsLineEdit_editingFinished();
    void optionsTableChanged();
    void optionsTableEnableDeleteButton();
    void updateOptions();
    void swapOptionsUI( int newIndex = -1 );
    void updateControls();

  protected:
    virtual void showEvent( QShowEvent * event );

  signals:
    void optionsChanged();

  private:

    QString settingsKey( QString profile ) const;
    QString currentProfileKey() const;
    QString createOptions( const QString& profile ) const;
    void deleteCreateOptions( const QString& profile );
    void setCreateOptions();
    void setCreateOptions( const QString& profile, const QString& options );
    void setCreateOptions( const QString& profile, const QStringList& list );
    QStringList profiles() const;
    bool eventFilter( QObject *obj, QEvent *event );

};
