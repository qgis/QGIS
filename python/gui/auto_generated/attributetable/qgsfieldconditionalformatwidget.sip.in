/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfieldconditionalformatwidget.h             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsFieldConditionalFormatWidget : QWidget
{
%Docstring
A widget for customizing conditional formatting options.

.. versionadded:: 2.12
%End

%TypeHeaderCode
#include "qgsfieldconditionalformatwidget.h"
%End
  public:

    explicit QgsFieldConditionalFormatWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsFieldConditionalFormatWidget.
%End

    void viewRules();
%Docstring
Switches the widget to the rules page.
%End

    void setLayer( QgsVectorLayer *layer );
%Docstring
Sets the vector layer associated with the widget.

:param layer: vector layer
%End

    void editStyle( int index, const QgsConditionalStyle &style );
%Docstring
Switches the widget to the edit style mode for the specified style.

:param index: index of conditional style to edit
:param style: initial conditional styling options
%End

    void loadStyle( const QgsConditionalStyle &style );
%Docstring

:param style: initial conditional styling options
%End

    void reset();
%Docstring
Resets the formatting options to their default state.
%End

    void setPresets( const QList<QgsConditionalStyle> &styles );
%Docstring
Set the presets that can be used for quick pick

:param styles: A list of styles used as presets
%End

    QList<QgsConditionalStyle> defaultPresets() const;
%Docstring
The default presets for the widget.  Normally set when the widget is
created however called setPresets will override the default styles.

:return: List of default presets.
%End

  signals:

    void rulesUpdated( const QString &fieldName );
%Docstring
Emitted when the conditional styling rules are updated.

:param fieldName: name of field whose rules have been modified.
%End

  public slots:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsfieldconditionalformatwidget.h             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
