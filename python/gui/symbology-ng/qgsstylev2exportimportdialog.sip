class QgsStyleV2ExportImportDialog : QDialog
{
%TypeHeaderCode
#include <qgsstylev2exportimportdialog.h>
%End
  public:
    enum Mode
    {
      Export,
      Import
    };

    // constructor
    // mode argument must be 0 for saving and 1 for loading
    QgsStyleV2ExportImportDialog( QgsStyleV2* style, QWidget *parent /TransferThis/ = NULL, Mode mode = Export );
    ~QgsStyleV2ExportImportDialog();

    /**
     * @brief selectSymbols select symbols by name
     * @param symbolNames list of symbol names
     */
    void selectSymbols( const QStringList& symbolNames );
    /**
     * @brief deselectSymbols deselect symbols by name
     * @param symbolNames list of symbol names
     */
    void deselectSymbols( const QStringList& symbolNames );

  public slots:
    void doExportImport();
    /**
     * @brief selectByGroup open select by group dialog
     */
    void selectByGroup();
    /**
     * @brief selectAll selects all symbols
     */
    void selectAll();
    /**
     * @brief clearSelection deselects all symbols
     */
    void clearSelection();
    /**
     * Select the symbols belonging to the given group
     * @param groupName the name of the group to be selected
     */
    void selectGroup( const QString& groupName );
    /**
     * Deselect the symbols belonging to the given group
     * @param groupName the name of the group to be deselected
     */
    void deselectGroup( const QString& groupName );
    /**
     * @brief selectSmartgroup selects all symbols from a smart group
     * @param groupName
     */
    void selectSmartgroup( const QString& groupName );
    /**
     * @brief deselectSmartgroup deselects all symbols from a smart group
     * @param groupName
     */
    void deselectSmartgroup( const QString& groupName );

    void importTypeChanged( int );
    void browse();
};
