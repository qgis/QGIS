
/** \ingroup gui
 * \class QgsCollapsibleGroupBoxBasic
 * A groupbox that collapses/expands when toggled.
 * Basic class QgsCollapsibleGroupBoxBasic does not auto-save collapsed or checked state
 * Holding Alt modifier key when toggling collapsed state will synchronize the toggling across other collapsible group boxes with the same syncGroup QString value
 * Holding Shift modifier key when attempting to toggle collapsed state will expand current group box, then collapse any others with the same syncGroup QString value
 * @note To add Collapsible properties in promoted QtDesigner widgets, you can add the following "Dynamic properties" by clicking on the green + in the propreties palette:
 * bool collapsed, QString syncGroup
 */

class QgsCollapsibleGroupBoxBasic : QGroupBox
{
%TypeHeaderCode
#include <qgscollapsiblegroupbox.h>
%End
  public:
    QgsCollapsibleGroupBoxBasic( QWidget *parent = 0 );
    QgsCollapsibleGroupBoxBasic( const QString &title, QWidget *parent = 0 );
    ~QgsCollapsibleGroupBoxBasic();

    bool isCollapsed() const;
    void setCollapsed( bool collapse );

    /** Named group which synchronizes collapsing action when triangle is clicked while holding alt modifier key */
    QString syncGroup() const;
    void setSyncGroup( QString grp );

    //! set this to false to not automatically scroll parent QScrollArea to this widget's contents when expanded
    void setScrollOnExpand( bool scroll );

  signals:
    /** Signal emitted when groupbox collapsed/expanded state is changed, and when first shown */
    void collapsedStateChanged( bool collapsed );

  public slots:
    void checkToggled( bool ckd );
    void toggleCollapsed();

  protected:
    void init();
    void showEvent( QShowEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void changeEvent( QEvent *event );

    void updateStyle();
    QRect titleRect() const;
};

/** \ingroup gui
 * \class QgsCollapsibleGroupBox
 * A groupbox that collapses/expands when toggled and can save its collapsed and checked states.
 * By default, it auto-saves only its collapsed state to the global settings based on the widget and it's parent names.
 * Holding Alt modifier key when toggling collapsed state will synchronize the toggling across other collapsible group boxes with the same syncGroup QString value
 * Holding Shift modifier key when attempting to toggle collapsed state will expand current group box, then collapse any others with the same syncGroup QString value
 * @see basic class QgsCollapsibleGroupBoxBasic which does not auto-save states
 * @note To add Collapsible properties in promoted QtDesigner widgets, you can add the following "Dynamic properties" by clicking on the green + in the propreties palette:
 * bool collapsed, bool saveCollapsedState, bool saveCheckedState, QString syncGroup
 */

class QgsCollapsibleGroupBox : QgsCollapsibleGroupBoxBasic
{
%TypeHeaderCode
#include <qgscollapsiblegroupbox.h>
%End
  public:
    QgsCollapsibleGroupBox( QWidget *parent = 0, QSettings* settings = 0 );
    QgsCollapsibleGroupBox( const QString &title, QWidget *parent = 0, QSettings* settings = 0 );
    ~QgsCollapsibleGroupBox();

    // set custom QSettings pointer if group box was already created from QtDesigner promotion
    void setSettings( QSettings* settings );

    //! set this to false to not save/restore collapsed state
    void setSaveCollapsedState( bool save );
    /** set this to true to save/restore checked state
     * @note only turn on mSaveCheckedState for groupboxes NOT used
     * in multiple places or used as options for different parent objects */
    void setSaveCheckedState( bool save );
    bool saveCollapsedState();
    bool saveCheckedState();

    //! set this to a defined string to share save/restore states across different parent dialogs
    void setSettingGroup( const QString &group );
    QString settingGroup() const;

  protected slots:
    void loadState();
    void saveState();

  protected:
    void init();
    void showEvent( QShowEvent *event );
    QString saveKey() const;
};
