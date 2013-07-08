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
     */
    QgsOptionsDialogBase( QString settingsKey, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsOptionsDialogBase();

    /** Set up the base ui connections for vertical tabs.
     * @param restoreUi Whether to restore the base ui at this time.
     */
    void initOptionsBase( bool restoreUi = true );

    /** Restore the base ui.
     * Sometimes useful to do at end of subclass's constructor.
     */
    void restoreOptionsBaseUi();

  protected slots:
    void updateOptionsListVerticalTabs();
    void optionsStackedWidget_CurrentChanged( int indx );
    void optionsStackedWidget_WidgetRemoved( int indx );
    void warnAboutMissingObjects();

  protected:
    void showEvent( QShowEvent* e );
    void paintEvent( QPaintEvent* e );
};
