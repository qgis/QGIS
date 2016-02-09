
class QgsFileWidget : QWidget
{
%TypeHeaderCode
#include <qgsfilewidget.h>
%End

  public:

    enum StorageMode
    {
      GetFile,
      GetDirectory
    };

    enum RelativeStorage
    {
      Absolute,
      RelativeProject,
      RelativeDefaultPath
    };

    explicit QgsFileWidget( QWidget* parent /TransferThis/ );

    ~QgsFileWidget();

   //! Returns the current file path
    QString filePath();

    //! Sets the file path
    void setFilePath( QString path );

    //! defines if the widget is readonly
    void setReadOnly( bool readOnly );

    //! returns the open file dialog title
    QString dialogTitle() const;
    /**
     * @brief setDialogTitle defines the open file dialog title
     * @note if not defined, the title is "Select a file" or "Select a directory" depending on the configuration.
     */
    void setDialogTitle( const QString& title );

    //! returns the filter used for QDialog::getOpenFileName
    QString filter() const;
    /**
     * @brief setFilter sets the filter used by the model to filters. The filter is used to specify the kind of files that should be shown.
     * @param filter Only files that match the given filter are shown, it may be an empty string. If you want multiple filters, separate them with ';;',
     */
    void setFilter( const QString &filter );

    //! determines if the tool button is shown
    bool fileWidgetButtonVisible() const;
    //! determines if the tool button is shown
    void setFileWidgetButtonVisible( bool visible );

    //! determines if the file path will be shown as a link
    bool useLink() const;
    //! determines if the file path will be shown as a link
    void setUseLink( bool useLink );

    //! determines if the links shows the full path or not
    bool fullUrl() const;
    void setFullUrl( bool fullUrl );

    //! determinies the default root path
    QString defaultRoot() const;
    void setDefaultRoot( const QString& defaultRoot );

    //! determines the storage mode (i.e. file or directory)
    QgsFileWidget::StorageMode storageMode() const;
    void setStorageMode( QgsFileWidget::StorageMode storageMode );

    //! determines if the relative path is with respect to the project path or the default path
    QgsFileWidget::RelativeStorage relativeStorage() const;
    void setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage );

  signals:
    void fileChanged( const QString& );


};
