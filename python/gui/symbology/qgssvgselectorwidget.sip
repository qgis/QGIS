/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssvgselectorwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsSvgSelectorListModel : QAbstractListModel
{
%Docstring
A model for displaying SVG files with a preview icon. Population of the model is performed in
a background thread to ensure that initial creation of the model is responsive and does
not block the GUI.
%End

%TypeHeaderCode
#include "qgssvgselectorwidget.h"
%End
  public:

    QgsSvgSelectorListModel( QObject *parent /TransferThis/, int iconSize = 30 );
%Docstring
Constructor for QgsSvgSelectorListModel. All SVGs in folders from the application SVG
search paths will be shown.

:param parent: parent object
:param iconSize: desired size of SVG icons to create
%End

    QgsSvgSelectorListModel( QObject *parent /TransferThis/, const QString &path, int iconSize = 30 );
%Docstring
Constructor for creating a model for SVG files in a specific path.

:param parent: parent object
:param path: initial path, which is recursively searched
:param iconSize: desired size of SVG icons to create
%End

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;


  protected:

};


class QgsSvgSelectorGroupsModel : QStandardItemModel
{
%Docstring
A model for displaying SVG search paths. Population of the model is performed in
a background thread to ensure that initial creation of the model is responsive and does
not block the GUI.
%End

%TypeHeaderCode
#include "qgssvgselectorwidget.h"
%End
  public:
    QgsSvgSelectorGroupsModel( QObject *parent /TransferThis/ );
    ~QgsSvgSelectorGroupsModel();

};

class QgsSvgSelectorWidget : QWidget
{

%TypeHeaderCode
#include "qgssvgselectorwidget.h"
%End
  public:

    QgsSvgSelectorWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsSvgSelectorWidget
%End

    QString currentSvgPath() const;

  public slots:
    void setSvgPath( const QString &svgPath );
%Docstring
Accepts absolute paths
%End

  signals:
    void svgSelected( const QString &path );

  protected:
    void populateList();

};

class QgsSvgSelectorDialog : QDialog
{

%TypeHeaderCode
#include "qgssvgselectorwidget.h"
%End
  public:

    QgsSvgSelectorDialog( QWidget *parent /TransferThis/ = 0,
                          Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                          QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close | QDialogButtonBox::Ok,
                          Qt::Orientation orientation = Qt::Horizontal );
%Docstring
Constructor for QgsSvgSelectorDialog.
%End
    ~QgsSvgSelectorDialog();

    QgsSvgSelectorWidget *svgSelector();
%Docstring
Returns pointer to the embedded SVG selector widget
%End

  protected:
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssvgselectorwidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
