class QgsOptionsDialogBase : QDialog
{
%TypeHeaderCode
#include <qgsoptionsdialogbase.h>
%End
  public:
    /** Constructor
     * @param settingsKey QSettings subgroup key for saving/restore ui states, e.g. "ProjectProperties".
     * @param parent parent object (owner)
     * @param fl widget flags
     * @param settings custom QSettings pointer
     */
    QgsOptionsDialogBase( QString settingsKey, QWidget* parent = 0, Qt::WindowFlags fl = 0, QSettings* settings = 0 );
    ~QgsOptionsDialogBase();

    /** Set up the base ui connections for vertical tabs.
     * @param restoreUi Whether to restore the base ui at this time.
     * @param title the window title
     */
    void initOptionsBase( bool restoreUi = true, QString title = QString() );

    // set custom QSettings pointer if dialog used outside QGIS (in plugin)
    void setSettings( QSettings* settings );

    /** Restore the base ui.
     * Sometimes useful to do at end of subclass's constructor.
     * @param title the window title (it does not need to be defined if previously given to initOptionsBase();
     */
    void restoreOptionsBaseUi( QString title = QString() );

    /** determine if the options list is in icon only mode
     */
    bool iconOnly();

  protected slots:
    void updateOptionsListVerticalTabs();
    void optionsStackedWidget_CurrentChanged( int indx );
    void optionsStackedWidget_WidgetRemoved( int indx );
    void warnAboutMissingObjects();

  protected:
    void showEvent( QShowEvent* e );
    void paintEvent( QPaintEvent* e );

    virtual void updateWindowTitle();
};
