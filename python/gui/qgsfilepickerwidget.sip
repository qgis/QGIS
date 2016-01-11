
class QgsFilePickerWidget : QWidget
{
%TypeHeaderCode
#include <qgsfilepickerwidget.h>
%End

  public:

    enum StorageMode
    {
      File,
      Directory
    };

    enum RelativeStorage
    {
      Absolute,
      RelativeProject,
      RelativeDefaultPath
    };

    explicit QgsFilePickerWidget( QWidget* parent /TransferThis/ );

    ~QgsFilePickerWidget();

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
    void setDialogTitle( QString title );

    //! determines if the tool button is shown
    bool filePickerButtonVisible() const;
    //! determines if the tool button is shown
    void setFilePickerButtonVisible( bool visible );

    //! determines if the file path will be shown as a link
    bool useLink() const;
    //! determines if the file path will be shown as a link
    void setUseLink( bool useLink );

    //! determines if the links shows the full path or not
    bool fullUrl() const;
    void setFullUrl( bool fullUrl );

    //! determinies the default root path
    QString defaultRoot() const;
    void setDefaultRoot( QString defaultRoot );

    //! determines the storage mode (i.e. file or directory)
    QgsFilePickerWidget::StorageMode storageMode() const;
    void setStorageMode( QgsFilePickerWidget::StorageMode storageMode );

    //! determines if the relative path is with respect to the project path or the default path
    QgsFilePickerWidget::RelativeStorage relativeStorage() const;
    void setRelativeStorage( QgsFilePickerWidget::RelativeStorage relativeStorage );

  signals:
    void fileChanged( QString );


};
