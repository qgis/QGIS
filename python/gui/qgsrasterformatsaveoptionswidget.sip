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

.. seealso:: :py:func:`setOptions`
%End

    void setOptions( const QString &options );
%Docstring
Populate widget with user-defined options. String should contain
key=value pairs separated by spaces, e.g. "TILED=YES TFW=YES"

.. seealso:: :py:func:`options`

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
%End

    void updateProfiles();
%Docstring
Reloads profiles list from QGIS settings
%End

  private slots:
    void mProfileNewButton_clicked() ;
    void mProfileDeleteButton_clicked() ;
    void mProfileResetButton_clicked() ;
    void mOptionsAddButton_clicked() ;
    void mOptionsDeleteButton_clicked() ;
    void mOptionsLineEdit_editingFinished() ;
    void optionsTableChanged() ;
    void optionsTableEnableDeleteButton() ;
    void updateOptions() ;
    void swapOptionsUI( int newIndex = -1 ) ;
    void updateControls() ;
  protected:
    virtual void showEvent( QShowEvent *event );


  signals:
    void optionsChanged();

  private:
    QString settingsKey( QString profile ) const ;
    QString currentProfileKey() const ;
    QString createOptions( const QString &profile ) const ;
    void deleteCreateOptions( const QString &profile ) ;
    void setCreateOptions() ;
    void setCreateOptions( const QString &profile, const QString &options ) ;
    void setCreateOptions( const QString &profile, const QStringList &list ) ;
    QStringList profiles() const ;
    virtual bool eventFilter( QObject *obj, QEvent *event ) ;

    QString pseudoFormat() const ;
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrasterformatsaveoptionswidget.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
