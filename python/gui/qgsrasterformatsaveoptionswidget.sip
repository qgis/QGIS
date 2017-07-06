/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrasterformatsaveoptionswidget.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterFormatSaveOptionsWidget: QWidget
{
%Docstring
 A widget to select format-specific raster saving options
%End

%TypeHeaderCode
#include "qgsrasterformatsaveoptionswidget.h"
%End
  public:

    enum Type
    {
      Default,
      Full,
      Table,
      LineEdit,
      ProfileLineEdit
    };

    QgsRasterFormatSaveOptionsWidget( QWidget *parent /TransferThis/ = 0,
                                      const QString &format = "GTiff",
                                      QgsRasterFormatSaveOptionsWidget::Type type = Default,
                                      const QString &provider = "gdal" );

    void setFormat( const QString &format );
%Docstring
 Set output raster format, it is used to determine list
 of available options
%End

    void setProvider( const QString &provider );
%Docstring
 Set provider key, , it is used to determine list
 of available options
%End

    void setRasterLayer( QgsRasterLayer *rasterLayer );
%Docstring
 Set output raster layer
%End

    void setRasterFileName( const QString &file );
%Docstring
 Set output raster file name
%End

    QStringList options() const;
%Docstring
 Returns list of selected options
.. seealso:: setOptions()
 :rtype: list of str
%End

    void setOptions( const QString &options );
%Docstring
 Populate widget with user-defined options. String should contain
 key=value pairs separated by spaces, e.g. "TILED=YES TFW=YES"
.. seealso:: options()
.. versionadded:: 3.0
%End

    void setType( QgsRasterFormatSaveOptionsWidget::Type type = Default );
%Docstring
 Set widget look and feel
%End

    void setPyramidsFormat( QgsRaster::RasterPyramidsFormat format );
%Docstring
 Set pyramids format to use
%End

  public slots:

    void apply();

    void helpOptions();
%Docstring
 Opens window with options desctiption for given provider
 and output format
%End

    QString validateOptions( bool gui = true, bool reportOk = true );
%Docstring
 Validates options correctness
 :rtype: str
%End

    void updateProfiles();
%Docstring
 Reloads profiles list from QGIS settings
%End

  private slots:
    void on_mProfileNewButton_clicked() ;
  private slots:
    void on_mProfileDeleteButton_clicked() ;
  private slots:
    void on_mProfileResetButton_clicked() ;
  private slots:
    void on_mOptionsAddButton_clicked() ;
  private slots:
    void on_mOptionsDeleteButton_clicked() ;
  private slots:
    void on_mOptionsLineEdit_editingFinished() ;
  private slots:
    void optionsTableChanged() ;
  private slots:
    void optionsTableEnableDeleteButton() ;
  private slots:
    void updateOptions() ;
  private slots:
    void swapOptionsUI( int newIndex = -1 ) ;
  private slots:
    void updateControls() ;
  protected:
    virtual void showEvent( QShowEvent *event );

  signals:
    void optionsChanged();

  private:
    QString settingsKey( QString profile ) const ;
%Docstring
 :rtype: str
%End
  private:
    QString currentProfileKey() const ;
%Docstring
 :rtype: str
%End
  private:
    QString createOptions( const QString &profile ) const ;
%Docstring
 :rtype: str
%End
  private:
    void deleteCreateOptions( const QString &profile ) ;
  private:
    void setCreateOptions() ;
  private:
    void setCreateOptions( const QString &profile, const QString &options ) ;
  private:
    void setCreateOptions( const QString &profile, const QStringList &list ) ;
  private:
    QStringList profiles() const ;
%Docstring
 :rtype: list of str
%End
  private:
    virtual bool eventFilter( QObject *obj, QEvent *event ) ;

  private:
    QString pseudoFormat() const ;
%Docstring
 :rtype: str
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrasterformatsaveoptionswidget.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
