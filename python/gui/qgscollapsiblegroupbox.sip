class QgsCollapsibleGroupBox : QGroupBox
{
%TypeHeaderCode
#include <qgscollapsiblegroupbox.h>
%End
  public:
    QgsCollapsibleGroupBox( QWidget *parent = 0 );
    QgsCollapsibleGroupBox( const QString &title, QWidget *parent = 0 );
    ~QgsCollapsibleGroupBox();

    bool isCollapsed() const;
    void setCollapsed( bool collapse );

    //! set this to false to not save/restore collapsed state
    void setSaveCollapsedState( bool save );
    /** set this to true to save/restore checked state
     * @note only turn on mSaveCheckedState for groupboxes NOT used
     * in multiple places or used as options for different parent objects */
    void setSaveCheckedState( bool save );
    bool saveCollapsedState();
    bool saveCheckedState();

    //! set this to a defined string to share save/restore collapsed state across dialogs
    void setSettingGroup( const QString &group );
    QString settingGroup() const;

    //! set this to false to not automatically scroll parent QScrollArea to this widget's contents when expanded
    void setScrollOnExpand( bool scroll );

  signals:
    /** Signal emitted when groupbox collapsed/expanded state is changed, and when first shown */
    void collapsedStateChanged( QgsCollapsibleGroupBox* );

  public slots:
    void checkToggled( bool ckd );
    void toggleCollapsed();

  protected slots:
    void loadState();
    void saveState();

  protected:
    void init();
    void showEvent( QShowEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void changeEvent( QEvent *event );

    void updateStyle();
    QRect titleRect() const;
    QString saveKey() const;

};
