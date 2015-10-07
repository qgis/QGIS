/** \ingroup gui
 * New name, for example new layer name dialog. If existing names are provided,
 * the dialog warns users if an entered name already exists.
 * @note added in 2.10
 */
class QgsNewNameDialog : QgsDialog
{
%TypeHeaderCode
#include <qgsnewnamedialog.h>
%End

  public:
    /** New dialog constructor.
     * @param source original data source name, e.g. original layer name of the layer to be copied
     * @param initial initial name
     * @param extensions base name extensions, e.g. raster base name band extensions or vector layer type extensions
     * @param existing existing names
     * @param regexp regular expression to be used as validator, for example db tables should have "[A-Za-z_][A-Za-z0-9_]+"
     * @param cs case sensitivity for new name to existing names comparison
     * @param parent
     * @param flags
     */
    QgsNewNameDialog( const QString& source = QString::null, const QString& initial = QString::null,
                      const QStringList& extensions = QStringList(), const QStringList& existing = QStringList(),
                      const QRegExp& regexp = QRegExp(), Qt::CaseSensitivity cs = Qt::CaseSensitive,
                      QWidget *parent /TransferThis/ = 0, const Qt::WindowFlags& flags = QgisGui::ModalDialogFlags );

    /** Sets the hint string for the dialog (the text shown above the name
     * input box).
     * @param hintString hint text
     * @see hintString()
     * @note added in QGIS 2.12
     */
    void setHintString( const QString& hintString );

    /** Returns the hint string for the dialog (the text shown above the name
     * input box).
     * @see setHintString()
     * @note added in QGIS 2.12
     */
    QString hintString() const;

    /** Sets whether users are permitted to overwrite existing names. If true, then
     * the dialog will reflect that the new name will overwrite an existing name. If false,
     * then the dialog will not accept names which already exist.
     * @note added in QGIS 2.12
     * @see overwriteEnabled()
     */
    void setOverwriteEnabled( bool enabled );

    /** Returns whether users are permitted to overwrite existing names.
     * @note added in QGIS 2.12
     * @see setOverwriteEnabled()
     */
    bool overwriteEnabled() const;

    /** Sets the string used for warning users if a conflicting name exists.
     * @param string warning string. If empty a default warning string will be used.
     * @note added in QGIS 2.12
     * @see conflictingNameWarning()
     */
    void setConflictingNameWarning( const QString& string );

    /** Returns the string used for warning users if a conflicting name exists.
     * @note added in QGIS 2.12
     * @see setConflictingNameWarning()
     */
    QString conflictingNameWarning() const;

    /** Name entered by user.
     * @return new name
     */
    QString name() const;

    /** Test if name or name with at least one extension exists.
     * @param name name or base name
     * @param extensions base name extensions
     * @param existing existing names
     * @param cs case sensitivity for new name to existing names comparison
     * @return true if name exists
     */
    static bool exists( const QString& name, const QStringList& extensions,
                        const QStringList& existing, Qt::CaseSensitivity cs = Qt::CaseSensitive );
  public slots:
    void nameChanged();

  protected:

    QString highlightText( const QString& text );
    static QStringList fullNames( const QString& name, const QStringList& extensions );
    // get list of existing names
    static QStringList matching( const QStringList& newNames, const QStringList& existingNames,
                                 Qt::CaseSensitivity cs = Qt::CaseSensitive );
};

