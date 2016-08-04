
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

    /**
     * @brief QgsExternalResourceWidget creates a widget with a file widget and a document viewer
     * Both part of the widget are optional.
     * @see QgsFileWidget
     */
    explicit QgsExternalResourceWidget( QWidget *parent = 0 );

    /**
     * @brief documentPath returns the path of the current document in the widget
     * @param type determines the type of the returned null variant if the document is not defined yet
     */
    QVariant documentPath( QVariant::Type type = QVariant::String ) const;
    void setDocumentPath( const QVariant& documentPath );

    //! access the file widget to allow its configuration
    QgsFileWidget* fileWidget();

    //! returns if the file widget is visible in the widget
    bool fileWidgetVisible() const;
    //! set the visiblity of the file widget in the layout
    void setFileWidgetVisible( bool visible );

    //! returns the type of content used in the document viewer
    QgsExternalResourceWidget::DocumentViewerContent documentViewerContent() const;
    //! setDocumentViewerContent defines the type of content to be shown. Widget will be adapated accordingly
    void setDocumentViewerContent( QgsExternalResourceWidget::DocumentViewerContent content );

    //! returns the height of the document viewer
    int documentViewerHeight() const;
    /**
     * @brief setDocumentViewerWidth set the height of the document viewer.
     * @param height the height. Use 0 for automatic best display.
     */
    void setDocumentViewerHeight( int height );
    //! returns the width of the document viewer
    int documentViewerWidth() const ;
    /**
     * @brief setDocumentViewerWidth set the width of the document viewer.
     * @param width the width. Use 0 for automatic best display.
     */
    void setDocumentViewerWidth( int width );

    //! defines if the widget is readonly
    void setReadOnly( bool readOnly );

    /**
     * Configures if paths are handled absolute or relative and if relative,
     * which should be the base path.
     */
    QgsFileWidget::RelativeStorage relativeStorage() const;

    /**
     * Configures if paths are handled absolute or relative and if relative,
     * which should be the base path.
     */
    void setRelativeStorage( const QgsFileWidget::RelativeStorage& relativeStorage );


    /**
     * Configures the base path which should be used if the relativeStorage property
     * is set to QgsFileWidget::RelativeDefaultPath.
     */
    QString defaultRoot() const;

    /**
     * Configures the base path which should be used if the relativeStorage property
     * is set to QgsFileWidget::RelativeDefaultPath.
     */
    void setDefaultRoot(const QString& defaultRoot);

  signals:
    //! emitteed as soon as the current document changes
    void valueChanged( const QString& );
};
