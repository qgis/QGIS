class QgsGroupBoxCollapseButton : QToolButton
{
  public:
    QgsGroupBoxCollapseButton( QWidget *parent /TransferThis/ = 0 );

    ~QgsGroupBoxCollapseButton();

    bool altDown() const;
    void setAltDown( bool updown );

    bool shiftDown() const;
    void setShiftDown( bool shiftdown );

  protected:
    void mouseReleaseEvent( QMouseEvent *event );
};

/** \ingroup gui
 * \class QgsCollapsibleGroupBoxBasic
 * A groupbox that collapses/expands when toggled.
 * Basic class QgsCollapsibleGroupBoxBasic does not auto-save collapsed or checked state
 * Holding Alt modifier key when toggling collapsed state will synchronize the toggling across other collapsible group boxes with the same syncGroup QString value
 * Holding Shift modifier key when attempting to toggle collapsed state will expand current group box, then collapse any others with the same syncGroup QString value
 * @note To add Collapsible properties in promoted QtDesigner widgets, you can add the following "Dynamic properties" by clicking on the green + in the propreties palette:
 * bool collapsed, QString syncGroup, bool scrollOnExpand
 */

class QgsCollapsibleGroupBoxBasic : QGroupBox
{
%TypeHeaderCode
#include <qgscollapsiblegroupbox.h>
%End
  public:
    QgsCollapsibleGroupBoxBasic( QWidget *parent /TransferThis/ = 0 );
    QgsCollapsibleGroupBoxBasic( const QString &title, QWidget *parent /TransferThis/ = 0 );
    ~QgsCollapsibleGroupBoxBasic();

    /**
     * Returns the current collapsed state of this group box
     */
    bool isCollapsed() const;
    /**
     * Collapse or uncollapse this groupbox
     *
     * @param collapse Will collapse on true and uncollapse on false
     */
    void setCollapsed( bool collapse );

    /**
     * Named group which synchronizes collapsing action when triangle is clicked while holding alt modifier key
     */
    QString syncGroup() const;

    /**
     * Named group which synchronizes collapsing action when triangle is clicked while holding alt modifier key
     */
    void setSyncGroup( const QString& grp );

    //! Set this to false to not automatically scroll parent QScrollArea to this widget's contents when expanded
    void setScrollOnExpand( bool scroll );

    //! If this is set to false the parent QScrollArea will not be automatically scrolled to this widget's contents when expanded
    bool scrollOnExpand();

  signals:
    /** Signal emitted when groupbox collapsed/expanded state is changed, and when first shown */
    void collapsedStateChanged( bool collapsed );

  public slots:
    void checkToggled( bool ckd );
    void checkClicked( bool ckd );
    void toggleCollapsed();

  protected:
    void init();

    /** Visual fixes for when group box is collapsed/expanded */
    void collapseExpandFixes();

    void showEvent( QShowEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void changeEvent( QEvent *event );

    void updateStyle();
    QRect titleRect() const;
    void clearModifiers();
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
    QgsCollapsibleGroupBox( QWidget *parent /TransferThis/ = 0, QSettings* settings = 0 );
    QgsCollapsibleGroupBox( const QString &title, QWidget *parent /TransferThis/ = 0, QSettings* settings = 0 );
    ~QgsCollapsibleGroupBox();

    // set custom QSettings pointer if group box was already created from QtDesigner promotion
    void setSettings( QSettings* settings );

    //! set this to false to not save/restore collapsed state
    void setSaveCollapsedState( bool save );
    /** Set this to true to save/restore checked state
     * @note only turn on mSaveCheckedState for groupboxes NOT used
     * in multiple places or used as options for different parent objects */
    void setSaveCheckedState( bool save );
    bool saveCollapsedState();
    bool saveCheckedState();

    //! Set this to a defined string to share save/restore states across different parent dialogs
    void setSettingGroup( const QString &group );
    //! Returns the name of the setting group in which the collapsed state will be saved
    QString settingGroup() const;

  protected slots:
    /**
     * Will load the collapsed and checked state
     *
     * The configuration path from which it is loaded is defined by
     *  * The object name
     *  * The settingGroup
     */
    void loadState();
    /**
     * Will save the collapsed and checked state
     *
     * The configuration path to which it is saved is defined by
     *  * The object name
     *  * The settingGroup
     */
    void saveState();

  protected:
    void init();
    void showEvent( QShowEvent *event );
    QString saveKey() const;
};
