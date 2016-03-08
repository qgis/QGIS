class QgsSvgSelectorListModel : QAbstractListModel
{
%TypeHeaderCode
#include <qgssvgselectorwidget.h>
%End
  public:
    QgsSvgSelectorListModel( QObject* parent /TransferThis/ );

    // Constructor to create model for icons in a specific path
    QgsSvgSelectorListModel( QObject* parent /TransferThis/, const QString& path );

    int rowCount( const QModelIndex & parent = QModelIndex() ) const;

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
};

class QgsSvgSelectorGroupsModel : QStandardItemModel
{
%TypeHeaderCode
#include <qgssvgselectorwidget.h>
%End
  public:
    QgsSvgSelectorGroupsModel( QObject* parent /TransferThis/ );
};


class QgsSvgSelectorWidget : QWidget
{
%TypeHeaderCode
#include <qgssvgselectorwidget.h>
%End
  public:
    QgsSvgSelectorWidget( QWidget* parent /TransferThis/ = 0 );
    ~QgsSvgSelectorWidget();

    static QgsSvgSelectorWidget* create( QWidget* parent = 0 ) /Factory/;

    QString currentSvgPath() const;
    QString currentSvgPathToName() const;

    QTreeView* groupsTreeView();
    QListView* imagesListView();
    QLineEdit* filePathLineEdit();
    QPushButton* filePathButton();
    QCheckBox* relativePathCheckbox();
    QLayout* selectorLayout();

  public slots:
    /** Accepts absolute and relative paths */
    void setSvgPath( const QString& svgPath );

  signals:
    void svgSelected( const QString& path );

  protected:
    void populateList();
};

class QgsSvgSelectorDialog : QDialog
{
%TypeHeaderCode
#include <qgssvgselectorwidget.h>
%End
  public:
    QgsSvgSelectorDialog( QWidget* parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags,
                          const QDialogButtonBox::StandardButtons& buttons = QDialogButtonBox::Close | QDialogButtonBox::Ok,
                          Qt::Orientation orientation = Qt::Horizontal );
    ~QgsSvgSelectorDialog();

    //! Returns the central layout. Widgets added to it must have this dialog as parent
    QVBoxLayout* layout();

    //! Returns the button box
    QDialogButtonBox* buttonBox();

    //! Returns pointer to the embedded SVG selector widget
    QgsSvgSelectorWidget* svgSelector();

};
