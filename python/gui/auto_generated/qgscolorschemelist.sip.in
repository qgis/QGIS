/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorschemelist.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsColorSwatchDelegate : QAbstractItemDelegate
{
%Docstring
A delegate for showing a color swatch in a list

.. seealso:: :py:class:`QgsColorSchemeList`

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorschemelist.h"
%End
  public:
    QgsColorSwatchDelegate( QWidget *parent /TransferThis/ = 0 );
    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

    virtual QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

    virtual bool editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index );


};


class QgsColorSchemeModel: QAbstractItemModel
{
%Docstring
A model for colors in a color scheme

.. seealso:: :py:class:`QgsColorSchemeList`

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorschemelist.h"
%End
  public:

    explicit QgsColorSchemeModel( QgsColorScheme *scheme, const QString &context = QString(), const QColor &baseColor = QColor(), QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor

:param scheme: color scheme for list
:param context: context string for color scheme
:param baseColor: base color for color scheme
:param parent: parent object
%End

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual Qt::DropActions supportedDropActions() const;

    virtual QStringList mimeTypes() const;

    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

    virtual bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );

    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;

    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );


    QgsNamedColorList colors() const;
%Docstring
Returns a list of colors shown in the widget

:return: colors shown in the widget
%End

    void setScheme( QgsColorScheme *scheme, const QString &context = QString(), const QColor &baseColor = QColor() );
%Docstring
Sets the color scheme to show in the widget

:param scheme: color scheme
:param context: context for color scheme
:param baseColor: base color for color scheme
%End

    QString context() const;
%Docstring
Get the current color scheme context for the model

:return: context string which is passed to scheme for color generation

.. seealso:: :py:func:`baseColor`
%End

    QColor baseColor() const;
%Docstring
Get the base color for the color scheme used by the model

:return: base color which is passed to scheme for color generation

.. seealso:: :py:func:`context`
%End

    void addColor( const QColor &color, const QString &label = QString(), bool allowDuplicate = false );
%Docstring
Add a color to the list

:param color: color to add
:param label: label for color
:param allowDuplicate: set to true to allow duplicate colors to be added (colors which are already present in the list)
%End

    bool isDirty() const;
%Docstring
Returns whether the color scheme model has been modified

:return: true if colors have been modified
%End

};

class QgsColorSchemeList: QTreeView
{
%Docstring
An editable list of color swatches, taken from an associated :py:class:`QgsColorScheme`.

.. seealso:: :py:class:`QgsColorSchemeList`

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorschemelist.h"
%End
  public:

    QgsColorSchemeList( QWidget *parent /TransferThis/ = 0, QgsColorScheme *scheme = 0, const QString &context = QString(), const QColor &baseColor = QColor() );
%Docstring
Construct a new color swatch grid.

:param parent: parent widget
:param scheme: QgsColorScheme for colors to show in the list
:param context: context string provided to color scheme
:param baseColor: base color for color scheme
%End

    bool saveColorsToScheme();
%Docstring
Saves the current colors shown in the list back to a color scheme, if supported
by the color scheme.

.. note::

   this method is only effective if the color scheme is editable
%End

    bool importColorsFromGpl( QFile &file );
%Docstring
Import colors from a GPL palette file to the list

:param file: file to import

.. seealso:: :py:func:`exportColorsToGpl`
%End

    bool exportColorsToGpl( QFile &file );
%Docstring
Export colors to a GPL palette file from the list

:param file: destination file

.. seealso:: :py:func:`importColorsFromGpl`
%End

    bool isDirty() const;
%Docstring
Returns whether the color scheme list has been modified

:return: true if colors have been modified
%End

    QgsColorScheme *scheme();
%Docstring
Returns the scheme currently selected in the list.

.. versionadded:: 3.0

.. seealso:: :py:func:`setScheme`
%End

  public slots:

    void setScheme( QgsColorScheme *scheme, const QString &context = QString(), const QColor &baseColor = QColor() );
%Docstring
Sets the color scheme to show in the list

:param scheme: QgsColorScheme for colors to show in the list
:param context: context string provided to color scheme
:param baseColor: base color for color scheme

.. seealso:: :py:func:`scheme`
%End

    void removeSelection();
%Docstring
Removes any selected colors from the list
%End

    void addColor( const QColor &color, const QString &label = QString(), bool allowDuplicate = false );
%Docstring
Adds a color to the list

:param color: color to add
:param label: optional label for color
:param allowDuplicate: set to true to allow duplicate colors to be added, ie colors which already exist in the list
%End

    void pasteColors();
%Docstring
Pastes colors from clipboard to the list

.. seealso:: :py:func:`copyColors`
%End

    void copyColors();
%Docstring
Copies colors from the list to the clipboard

.. seealso:: :py:func:`pasteColors`
%End

    void showImportColorsDialog();
%Docstring
Displays a file picker dialog allowing users to import colors into the list from a file.

.. versionadded:: 3.0

.. seealso:: :py:func:`showExportColorsDialog`
%End

    void showExportColorsDialog();
%Docstring
Displays a file picker dialog allowing users to export colors from the list into a file.

.. versionadded:: 3.0

.. seealso:: :py:func:`showImportColorsDialog`
%End

  signals:

    void colorSelected( const QColor &color );
%Docstring
Emitted when a color is selected from the list

:param color: color selected
%End

  protected:

    virtual void keyPressEvent( QKeyEvent *event );


    virtual void mousePressEvent( QMouseEvent *event );


    virtual void mouseReleaseEvent( QMouseEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorschemelist.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
