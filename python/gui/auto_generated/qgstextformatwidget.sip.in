/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgstextformatwidget.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsTextFormatWidget : QWidget, protected Ui::QgsTextFormatWidgetBase
{
%Docstring
A widget for customizing text formatting settings.

QgsTextFormatWidget provides a widget for controlling the appearance of text rendered
using QgsTextRenderer. The widget includes all settings contained within
a QgsTextFormat, including shadow, background and buffer.

Additionally, the widget can handle labeling settings due to the large overlap between
the text renderer settings and the labeling settings. This mode is possible by
subclassing QgsTextFormatWidget and calling the protected constructor with a mode
of Labeling.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextformatwidget.h"
%End
  public:

    QgsTextFormatWidget( const QgsTextFormat &format = QgsTextFormat(), QgsMapCanvas *mapCanvas = 0, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsTextFormatWidget.

:param format: initial formatting settings to show in widget
:param mapCanvas: associated map canvas
:param parent: parent widget
%End

    ~QgsTextFormatWidget();

    QgsTextFormat format() const;
%Docstring
Returns the current formatting settings defined by the widget.
%End

    void setFormat( const QgsTextFormat &format );
%Docstring
Sets the current formatting settings

.. versionadded:: 3.2
%End

  public slots:

    void setDockMode( bool enabled );
%Docstring
Sets whether the widget should be shown in a compact dock mode.

:param enabled: set to ``True`` to show in dock mode.
%End

  signals:

    void widgetChanged();
%Docstring
Emitted when the text format defined by the widget changes
%End

  protected:

    enum Mode
    {
      Text,
      Labeling,
    };

    QgsTextFormatWidget( QgsMapCanvas *mapCanvas, QWidget *parent /TransferThis/, Mode mode );
%Docstring
Constructor for QgsTextFormatWidget.

:param mapCanvas: associated map canvas
:param parent: parent widget
:param mode: widget mode
%End

    void updateWidgetForFormat( const QgsTextFormat &format );
%Docstring
Updates the widget's state to reflect the settings in a :py:class:`QgsTextFormat`.

:param format: source format
%End

    void setPreviewBackground( const QColor &color );
%Docstring
Sets the background color for the text preview widget.

:param color: background color
%End

    void enableDataDefinedAlignment( bool enable );
%Docstring
Controls whether data defined alignment buttons are enabled.

:param enable: set to ``True`` to enable alignment controls
%End


  protected slots:

    void updateLinePlacementOptions();
%Docstring
Updates line placement options to reflect current state of widget
%End

    void updatePlacementWidgets();
%Docstring
Updates label placement options to reflect current state of widget
%End

};



class QgsTextFormatDialog : QDialog
{
%Docstring
A simple dialog for customizing text formatting settings.

QgsTextFormatDialog provides a dialog for controlling the appearance of text rendered
using :py:class:`QgsTextRenderer`. The dialog includes all settings contained within
a :py:class:`QgsTextFormat`, including shadow, background and buffer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextformatwidget.h"
%End
  public:

    QgsTextFormatDialog( const QgsTextFormat &format, QgsMapCanvas *mapCanvas = 0, QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor for QgsTextFormatDialog.

:param format: initial format settings to show in dialog
:param mapCanvas: optional associated map canvas
:param parent: parent widget
:param fl: window flags for dialog
%End

    ~QgsTextFormatDialog();

    QgsTextFormat format() const;
%Docstring
Returns the current formatting settings defined by the widget.
%End

};


class QgsTextFormatPanelWidget : QgsPanelWidgetWrapper
{
%Docstring
A panel widget for customizing text formatting settings.

QgsTextFormatPanelWidget provides a panel widget for controlling the appearance of text rendered
using :py:class:`QgsTextRenderer`. The dialog includes all settings contained within
a :py:class:`QgsTextFormat`, including shadow, background and buffer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextformatwidget.h"
%End
  public:

    QgsTextFormatPanelWidget( const QgsTextFormat &format, QgsMapCanvas *mapCanvas = 0, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsTextFormatPanelWidget.

:param format: initial format settings to show in dialog
:param mapCanvas: optional associated map canvas
:param parent: parent widget
%End

    QgsTextFormat format() const;
%Docstring
Returns the current formatting settings defined by the widget.
%End

    virtual void setDockMode( bool dockMode );


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgstextformatwidget.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
