
class QgsFileWidget : QWidget
{
%TypeHeaderCode
#include <qgsfilewidget.h>
%End

  public:
    /**
     * @brief The StorageMode enum determines if the file picker should pick files or directories
     */
    enum StorageMode
    {
      GetFile,
      GetDirectory
    };

    /**
     * @brief The RelativeStorage enum determines if path is absolute, relative to the current project path or relative to a defined default path.
     */
    enum RelativeStorage
    {
      Absolute,
      RelativeProject,
      RelativeDefaultPath
    };

    /**
     * @brief QgsFileWidget creates a widget for selecting a file or a folder.
     */
    explicit QgsFileWidget( QWidget *parent /TransferThis/ );

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

    //! returns the filters used for QDialog::getOpenFileName
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

    //! returns if the links shows the full path or not
    bool fullUrl() const;
    //! determines if the links shows the full path or not
    void setFullUrl( bool fullUrl );

    //! returns the default root path
    QString defaultRoot() const;
    //! determines the default root path used as the first shown location when picking a file and used if the RelativeStorage is RelativeDefaultPath
    void setDefaultRoot( const QString& defaultRoot );

    //! returns the storage mode (i.e. file or directory)
    QgsFileWidget::StorageMode storageMode() const;
    //! determines the storage mode (i.e. file or directory)
    void setStorageMode( QgsFileWidget::StorageMode storageMode );

    //! returns if the relative path is with respect to the project path or the default path
    QgsFileWidget::RelativeStorage relativeStorage() const;
    //! determines if the relative path is with respect to the project path or the default path
    void setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage );

  signals:
    //! emitted as soon as the current file or directory is changed
    void fileChanged( const QString& );


};
