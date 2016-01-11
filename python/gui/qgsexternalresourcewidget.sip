
class QgsExternalResourceWidget : QWidget
{
%TypeHeaderCode
#include <qgsexternalresourcewidget.h>
%End

  public:

     enum DocumentViewerContent
    {
      NoContent,
      Image,
      Web
    };

    explicit QgsExternalResourceWidget( QWidget* parent /TransferThis/ );

    ~QgsExternalResourceWidget();

    /**
     * @brief documentPath returns the path of the current document in the widget
     * @param type determines the type of the returned null variant if the document is not defined yet
     */
    QVariant documentPath( QVariant::Type type = QVariant::String ) const;
    void setDocumentPath( QVariant documentPath );

    //! access the file picker widget to allow its configuration
    QgsFilePickerWidget* filePickerwidget();

    //! returns if the file picker is visible in the widget
    bool filePickerVisible() const;
    //! set the visiblity of the file picker in the widget
    void setFilePickerVisible( bool visible );

    //! returns the type of content used in the document viewer
    QgsExternalResourceWidget::DocumentViewerContent documentViewerContent() const;
    //! setDocumentViewerContent defines the type of content to be shown. Widget will be adapated accordingly
    void setDocumentViewerContent( QgsExternalResourceWidget::DocumentViewerContent content );

    //! set the configuration of the document viewer
    int documentViewerHeight() const;
    void setDocumentViewerHeight( int height );
    int documentViewerWidth() const ;
    void setDocumentViewerWidth( int width );

    //! defines if the widget is readonly
    void setReadOnly( bool readOnly );

  signals:
    void valueChanged( QString );


};
