/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsoptionsdialogbase.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsSearchHighlightOptionWidget : QObject
{
%Docstring
 Container for a widget to be used to search text in the option dialog
 If the widget type is handled, it is valid.
 It can perform a text search in the widget and highlight it in case of success.
 This uses stylesheets.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsoptionsdialogbase.h"
%End
  public:

    explicit QgsSearchHighlightOptionWidget( QWidget *widget = 0 );
%Docstring
 Constructor
 \param widget the widget used to search text into
%End

    bool isValid();
%Docstring
 Returns if it valid: if the widget type is handled and if the widget is not still available
 :rtype: bool
%End

    bool searchHighlight( const QString &searchText );
%Docstring
 search for a text pattern and highlight the widget if the text is found
 :return: true if the text pattern is found
 :rtype: bool
%End

    void reset();
%Docstring
  reset the style to the original state
%End

    QWidget *widget();
%Docstring
 return the widget
 :rtype: QWidget
%End

};



class QgsOptionsDialogBase : QDialog
{
%Docstring
 A base dialog for options and properties dialogs that offers vertical tabs.
 It handles saving/restoring of geometry, splitter and current tab states,
 switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
 and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots

 To use:
 1) Start with copy of qgsoptionsdialog_template.ui and build options/properties dialog.
 2) In source file for dialog, inherit this class instead of QDialog, then in constructor:
    ...
    setupUi( this ); // set up .ui file objects
    initOptionsBase( false ); // set up this class to use .ui objects, optionally restoring base ui
    ...
    restoreOptionsBaseUi(); // restore the base ui with initOptionsBase or use this later on
%End

%TypeHeaderCode
#include "qgsoptionsdialogbase.h"
%End
  public:

    QgsOptionsDialogBase( const QString &settingsKey, QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = 0, QgsSettings *settings = 0 );
%Docstring
 Constructor
 \param settingsKey QgsSettings subgroup key for saving/restore ui states, e.g. "ProjectProperties".
 \param parent parent object (owner)
 \param fl widget flags
 \param settings custom QgsSettings pointer
%End
    ~QgsOptionsDialogBase();

    void initOptionsBase( bool restoreUi = true, const QString &title = QString() );
%Docstring
 Set up the base ui connections for vertical tabs.
 \param restoreUi Whether to restore the base ui at this time.
 \param title the window title
%End

    void setSettings( QgsSettings *settings );

    void restoreOptionsBaseUi( const QString &title = QString() );
%Docstring
 Restore the base ui.
 Sometimes useful to do at end of subclass's constructor.
 \param title the window title (it does not need to be defined if previously given to initOptionsBase();
%End

    bool iconOnly();
%Docstring
 Determine if the options list is in icon only mode
 :rtype: bool
%End

  public slots:

    void searchText( const QString &text );
%Docstring
 searchText searches for a text in all the pages of the stacked widget and highlight the results
 \param text the text to search
.. versionadded:: 3.0
%End

  protected slots:
    virtual void updateOptionsListVerticalTabs();
%Docstring
Update tabs on the splitter move
%End
    virtual void optionsStackedWidget_CurrentChanged( int index );
%Docstring
Select relevant tab on current page change
%End
    virtual void optionsStackedWidget_WidgetRemoved( int index );
%Docstring
Remove tab and unregister widgets on page remove
%End

    void warnAboutMissingObjects();

  protected:
    virtual void showEvent( QShowEvent *e );

    virtual void paintEvent( QPaintEvent *e );


    virtual void updateWindowTitle();

    void registerTextSearchWidgets();
%Docstring
 register widgets in the dialog to search for text in it
 it is automatically called if a line edit has "mSearchLineEdit" as object name.
.. versionadded:: 3.0
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsoptionsdialogbase.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
