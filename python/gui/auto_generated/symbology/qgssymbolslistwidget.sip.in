/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbolslistwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsSymbolsListWidget : QWidget
{

%TypeHeaderCode
#include "qgssymbolslistwidget.h"
%End
  public:

    QgsSymbolsListWidget( QgsSymbol *symbol, QgsStyle *style, QMenu *menu, QWidget *parent /TransferThis/, QgsVectorLayer *layer = 0 );
%Docstring
Constructor for QgsSymbolsListWidget.

:param symbol: the symbol
:param style: the style
:param menu: the menu where to show it
:param parent: parent widget
:param layer: associated vector layer
%End


    ~QgsSymbolsListWidget();

    void setContext( const QgsSymbolWidgetContext &context );
%Docstring
Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.

:param context: symbol widget context

.. seealso:: :py:func:`context`

.. versionadded:: 3.0
%End

    QgsSymbolWidgetContext context() const;
%Docstring
Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.

.. seealso:: :py:func:`setContext`

.. versionadded:: 3.0
%End

    const QgsVectorLayer *layer() const;
%Docstring
Returns the vector layer associated with the widget.

.. versionadded:: 2.12
%End

  public slots:

    void setSymbolFromStyle( const QModelIndex &index );
    void setSymbolColor( const QColor &color );
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void saveSymbol();

    void symbolAddedToStyle( const QString &name, QgsSymbol *symbol );

    void populateGroups();
%Docstring
Pupulates the groups combo box with available tags and smartgroups
%End

    void openStyleManager();
    void clipFeaturesToggled( bool checked );

    void updateDataDefinedMarkerSize();
    void updateDataDefinedMarkerAngle();
    void updateDataDefinedLineWidth();

  signals:
    void changed();

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssymbolslistwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
