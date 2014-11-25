/** \ingroup gui
 * \class QgsColorSwatchDelegate
 * A delegate for showing a color swatch in a list
 * @see QgsColorSchemeList
 * @note introduced in QGIS 2.5
 */
class QgsColorSwatchDelegate : QAbstractItemDelegate
{
%TypeHeaderCode
#include <qgscolorschemelist.h>
%End

 public:
    QgsColorSwatchDelegate( QWidget *parent /TransferThis/ = 0 );
    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index );
};

/** \ingroup gui
 * \class QgsColorSchemeModel
 * A model for colors in a color scheme
 * @see QgsColorSchemeList
 * @note introduced in QGIS 2.5
 */
class QgsColorSchemeModel: QAbstractItemModel
{
%TypeHeaderCode
#include <qgscolorschemelist.h>
%End

  public:

    /**Constructor
     * @param scheme color scheme for list
     * @param context context string for color scheme
     * @param baseColor base color for color scheme
     * @param parent parent object
     */
    explicit QgsColorSchemeModel( QgsColorScheme* scheme, const QString context = QString(), const QColor &baseColor = QColor(), QObject* parent /TransferThis/ = 0 );

    ~QgsColorSchemeModel();

    //reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags( const QModelIndex & index ) const;
    bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
    QMimeData *mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    /**Returns a list of colors shown in the widget
     * @returns colors shown in the widget
     */
    QgsNamedColorList colors() const;

    /**Sets the color scheme to show in the widget
     * @param scheme color scheme
     * @param context context for color scheme
     * @param baseColor base color for color scheme
     */
    void setScheme( QgsColorScheme* scheme, const QString &context = QString(), const QColor &baseColor = QColor() );

    /**Get the current color scheme context for the model
     * @returns context string which is passed to scheme for color generation
     * @see baseColor
     */
    QString context() const;

    /**Get the base color for the color scheme used by the model
     * @returns base color which is passed to scheme for color generation
     * @see context
     */
    QColor baseColor() const;

    /**Add a color to the list
     * @param color color to add
     * @param label label for color
     */
    void addColor( const QColor color, const QString label = QString() );

    /**Returns whether the color scheme model has been modified
     * @returns true if colors have been modified
     */
    bool isDirty() const;

};

/** \ingroup gui
 * \class QgsColorSchemeList
 * An editable list of color swatches, taken from an associated QgsColorScheme.
 * @see QgsColorSchemeList
 * @note introduced in QGIS 2.5
 */
class QgsColorSchemeList: QTreeView
{
%TypeHeaderCode
#include <qgscolorschemelist.h>
%End

  public:

    /**Construct a new color swatch grid.
     * @param parent parent widget
     * @param scheme QgsColorScheme for colors to show in the list
     * @param context context string provided to color scheme
     * @param baseColor base color for color scheme
     */
    QgsColorSchemeList( QWidget *parent /TransferThis/ = 0, QgsColorScheme* scheme = 0, const QString &context = QString(), const QColor &baseColor = QColor() );

    virtual ~QgsColorSchemeList();

    /**Saves the current colors shown in the list back to a color scheme, if supported
     * by the color scheme.
     * @note this method is only effective if the color scheme is editable
     */
    bool saveColorsToScheme();

    /**Import colors from a GPL palette file to the list
     * @param file file to import
     * @see exportColorsToGpl
     */
    bool importColorsFromGpl( QFile &file );

    /**Export colors to a GPL palette file from the list
     * @param file destination file
     * @see importColorsFromGpl
     */
    bool exportColorsToGpl( QFile &file );

    /**Returns whether the color scheme list has been modified
     * @returns true if colors have been modified
     */
    bool isDirty() const;

  public slots:

    /**Sets the color scheme to show in the list
     * @param scheme QgsColorScheme for colors to show in the list
     * @param context context string provided to color scheme
     * @param baseColor base color for color scheme
     */
    void setScheme( QgsColorScheme* scheme, const QString &context = QString(), const QColor &baseColor = QColor() );

    /**Removes any selected colors from the list
     */
    void removeSelection();

    /**Adds a color to the list
     * @param color color to add
     * @param label optional label for color
     */
    void addColor( const QColor &color, const QString &label = QString() );

    /**Pastes colors from clipboard to the list
     * @see copyColors
     */
    void pasteColors();

    /**Copies colors from the list to the clipboard
     * @see pasteColors
     */
    void copyColors();

  signals:

    /**Emitted when a color is selected from the list
     * @param color color selected
     */
    void colorSelected( const QColor &color );

  protected:

    void keyPressEvent( QKeyEvent* event );

    void mousePressEvent( QMouseEvent* event );

    void mouseReleaseEvent( QMouseEvent* event );

};

