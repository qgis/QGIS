/** \ingroup gui
 * \class QgsTaskManagerWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager
 * @see QgsTaskManager
 * @note introduced in QGIS 2.16
 */
class QgsTaskManagerWidget : QWidget
{
%TypeHeaderCode
#include <qgstaskmanagerwidget.h>
%End
  public:

     /** Constructor for QgsTaskManagerWidget
     * @param manager task manager associated with widget
     * @param parent parent widget
     */
     QgsTaskManagerWidget( QgsTaskManager* manager, QWidget* parent /TransferThis/ = nullptr );

};

/** \ingroup gui
 * \class QgsTaskManagerFloatingWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager
 * @see QgsTaskManager
 * @note introduced in QGIS 2.16
 */
class QgsTaskManagerFloatingWidget : QgsFloatingWidget
{
%TypeHeaderCode
#include <qgstaskmanagerwidget.h>
%End

  public:

    /** Constructor for QgsTaskManagerWidget
     * @param manager task manager associated with widget
     * @param parent parent widget
     */
    QgsTaskManagerFloatingWidget( QgsTaskManager* manager, QWidget* parent = nullptr );

};

/** \ingroup gui
 * \class QgsTaskManagerFloatingWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager
 * @see QgsTaskManager
 * @note introduced in QGIS 2.16
 */
class QgsTaskManagerStatusBarWidget : QToolButton
{
%TypeHeaderCode
#include <qgstaskmanagerwidget.h>
%End

  public:

    /** Constructor for QgsTaskManagerStatusBarWidget
     * @param manager task manager associated with widget
     * @param parent parent widget
     */
    QgsTaskManagerStatusBarWidget( QgsTaskManager* manager, QWidget* parent = nullptr );

};


/** \ingroup gui
 * \class QgsTaskManagerModel
 * A model representing a QgsTaskManager
 * @see QgsTaskManager
 * @note introduced in QGIS 2.16
 */
class QgsTaskManagerModel: QAbstractItemModel
{
%TypeHeaderCode
#include <qgstaskmanagerwidget.h>
%End
  public:

    /** Constructor for QgsTaskManagerModel
     * @param manager task manager for model
     * @param parent parent object
     */
    explicit QgsTaskManagerModel( QgsTaskManager* manager, QObject* parent /TransferThis/ = nullptr );

    //reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags( const QModelIndex & index ) const;
    bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

};


/** \ingroup gui
 * \class QgsProgressBarDelegate
 * A delegate for showing a progress bar within a view
 * @note introduced in QGIS 2.16
 */
class QgsProgressBarDelegate : QStyledItemDelegate
{
%TypeHeaderCode
#include <qgstaskmanagerwidget.h>
%End
  public:

    /** Constructor for QgsProgressBarDelegate
     * @param parent parent object
     */
    QgsProgressBarDelegate( QObject* parent /TransferThis/ = nullptr );

    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

/** \ingroup gui
 * \class QgsProgressBarDelegate
 * A delegate for showing task status within a view. Clicks on the delegate will cause the task to be cancelled (via the model).
 * @note introduced in QGIS 2.16
 */
class QgsTaskStatusDelegate : QStyledItemDelegate
{
%TypeHeaderCode
#include <qgstaskmanagerwidget.h>
%End
  public:

    /** Constructor for QgsTaskStatusDelegate
     * @param parent parent object
     */
    QgsTaskStatusDelegate( QObject* parent /TransferThis/ = nullptr );

    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index );
};
