/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaplayerconfigwidgetfactory.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapLayerConfigWidgetFactory
{
%Docstring
.. versionadded:: 2.16
 Factory class for creating custom map layer property pages
%End

%TypeHeaderCode
#include "qgsmaplayerconfigwidgetfactory.h"
%End
  public:

    QgsMapLayerConfigWidgetFactory();
%Docstring
Constructor
%End

    QgsMapLayerConfigWidgetFactory( const QString &title, const QIcon &icon );
%Docstring
Constructor
%End

    virtual ~QgsMapLayerConfigWidgetFactory();

    virtual QIcon icon() const;
%Docstring
 The icon that will be shown in the UI for the panel.
 :return: A QIcon for the panel icon.
 :rtype: QIcon
%End

    void setIcon( const QIcon &icon );
%Docstring
 Set the icon for the factory object.
 \param icon The icon to show in the interface.
%End

    virtual QString title() const;
%Docstring
 The title of the panel.
.. note::

   This may or may not be shown to the user.
 :return: Title of the panel
 :rtype: str
%End

    void setTitle( const QString &title );
%Docstring
 Set the title for the interface
.. note::

   Not all users may show this as a label
 e.g style dock uses this as a tooltip.
 \param title The title to set.
%End

    virtual bool supportsStyleDock() const;
%Docstring
 Flag if widget is supported for use in style dock.
 The default implementation returns false.
 :return: True if supported
 :rtype: bool
%End

    void setSupportsStyleDock( bool supports );
%Docstring
 Set support flag for style dock
 \param supports True if this widget is supported in the style dock.
%End

    virtual bool supportLayerPropertiesDialog() const;
%Docstring
 Flag if widget is supported for use in layer properties dialog.
 The default implementation returns false.
 :return: True if supported
 :rtype: bool
%End

    void setSupportLayerPropertiesDialog( bool supports );
%Docstring
 Set support flag for style dock
 \param supports True if this widget is supported in the style dock.
%End

    virtual bool supportsLayer( QgsMapLayer *layer ) const;
%Docstring
 Check if the layer is supported for this widget.
 :return: True if this layer is supported for this widget
 :rtype: bool
%End

    virtual QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget = true, QWidget *parent /TransferThis/ = 0 ) const = 0 /Factory/;
%Docstring
 Factory function to create the widget on demand as needed by the dock.
.. note::

   This function is called each time the panel is selected. Keep it light for better UX.
 \param layer The active layer in the dock.
 \param canvas The map canvas.
 \param dockWidget True of the widget will be shown a dock style widget.
 \param parent The parent of the widget.
 :return: A new QgsMapStylePanel which is shown in the map style dock.
 :rtype: QgsMapLayerConfigWidget
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaplayerconfigwidgetfactory.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
