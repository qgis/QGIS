class QgsSearchHighlightOptionWidget : QObject
{
%TypeHeaderCode
#include <qgsoptionsdialogbase.h>
%End
  public:
    explicit QgsSearchHighlightOptionWidget( QWidget* widget = 0 );
    ~QgsSearchHighlightOptionWidget();

    bool isValid();

    bool searchHighlight( QString searchText );

    void reset();

    QWidget* widget();
};



class QgsOptionsDialogBase : QDialog
{
%TypeHeaderCode
#include <qgsoptionsdialogbase.h>
%End
  public:
    /** Constructor
     * @param settingsKey QgsSettings subgroup key for saving/restore ui states, e.g. "ProjectProperties".
     * @param parent parent object (owner)
     * @param fl widget flags
     * @param settings custom QgsSettings pointer
     */
    QgsOptionsDialogBase( const QString& settingsKey, QWidget* parent /TransferThis/ = 0, const Qt::WindowFlags& fl = 0, QgsSettings* settings = 0 );
    ~QgsOptionsDialogBase();

    /** Set up the base ui connections for vertical tabs.
     * @param restoreUi Whether to restore the base ui at this time.
     * @param title the window title
     */
    void initOptionsBase( bool restoreUi = true, const QString& title = QString() );

    // set custom QgsSettings pointer if dialog used outside QGIS (in plugin)
    void setSettings( QgsSettings* settings );

    /** Restore the base ui.
     * Sometimes useful to do at end of subclass's constructor.
     * @param title the window title (it does not need to be defined if previously given to initOptionsBase();
     */
    void restoreOptionsBaseUi( const QString& title = QString() );

    /** Determine if the options list is in icon only mode
     */
    bool iconOnly();

  public slots:

    /**
     * searchText searches for a text in all the pages of the stacked widget and highlight the results
     * @param text the text to search
     */
    void searchText( QString text );

  protected slots:
    void updateOptionsListVerticalTabs();
    void optionsStackedWidget_CurrentChanged( int indx );
    void optionsStackedWidget_WidgetRemoved( int indx );
    void warnAboutMissingObjects();

  protected:
    void showEvent( QShowEvent* e );
    void paintEvent( QPaintEvent* e );

    virtual void updateWindowTitle();

    void registerTextSearchWidgets();
};
