/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgssearchwidgettoolbutton.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSearchWidgetToolButton : QToolButton
{
%Docstring
 A tool button widget which is displayed next to search widgets in forms, and
 allows for controlling how the widget behaves and how the filtering/searching
 operates.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgssearchwidgettoolbutton.h"
%End
  public:

    explicit QgsSearchWidgetToolButton( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsSearchWidgetToolButton.
 \param parent parent object
%End

    void setAvailableFlags( QgsSearchWidgetWrapper::FilterFlags flags );
%Docstring
 Sets the available filter flags to show in the widget. Any active flags
 (see activeFlags()) which are not present in the new available filter
 flags will be cleared;
 \param flags available flags to show in widget
.. seealso:: availableFlags()
.. seealso:: setActiveFlags()
.. seealso:: setDefaultFlags()
%End

    void setDefaultFlags( QgsSearchWidgetWrapper::FilterFlags flags );
%Docstring
 Sets the default filter flags to show in the widget.
 \param flags default flags to show in widget
.. seealso:: setAvailableFlags()
.. seealso:: setActiveFlags()
%End

    QgsSearchWidgetWrapper::FilterFlags availableFlags() const;
%Docstring
 Returns the available filter flags shown in the widget.
.. seealso:: setAvailableFlags()
.. seealso:: activeFlags()
 :rtype: QgsSearchWidgetWrapper.FilterFlags
%End

    void setActiveFlags( QgsSearchWidgetWrapper::FilterFlags flags );
%Docstring
 Sets the current active filter flags for the widget. Any flags
 which are not present in the available filter flags (see availableFlags())
 will not be set.
 \param flags active flags to show in widget
.. seealso:: toggleFlag()
.. seealso:: activeFlags()
.. seealso:: setAvailableFlags()
%End

    void toggleFlag( QgsSearchWidgetWrapper::FilterFlag flag );
%Docstring
 Toggles an individual active filter flag for the widget. Any flags
 which are not present in the available filter flags (see availableFlags())
 will be ignore. Other flags may be cleared if they conflict with the newly
 toggled flag.
 \param flag flag to toggle
.. seealso:: setActiveFlags()
.. seealso:: activeFlags()
%End

    QgsSearchWidgetWrapper::FilterFlags activeFlags() const;
%Docstring
 Returns the active filter flags shown in the widget.
.. seealso:: setActiveFlags()
.. seealso:: toggleFlag()
.. seealso:: availableFlags()
 :rtype: QgsSearchWidgetWrapper.FilterFlags
%End

    bool isActive() const;
%Docstring
 Returns true if the widget is set to be included in the search.
.. seealso:: setInactive()
.. seealso:: setActive()
 :rtype: bool
%End

  public slots:

    void setInactive();
%Docstring
 Sets the search widget as inactive, ie do not search the corresponding field.
.. seealso:: isActive()
.. seealso:: setActive()
%End

    void setActive();
%Docstring
 Sets the search widget as active by selecting the first available search type.
.. seealso:: isActive()
.. seealso:: setInactive()
%End

  signals:

    void activeFlagsChanged( QgsSearchWidgetWrapper::FilterFlags flags );
%Docstring
 Emitted when the active flags selected in the widget is changed
 \param flags active flags
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgssearchwidgettoolbutton.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
