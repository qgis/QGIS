class QgsStyleSaveDialog : QDialog
{
%TypeHeaderCode
#include <qgsstylesavedialog.h>
%End

  public:
    /** Constructor for QgsSymbolSaveDialog
     * @param parent parent widget
     * @param type the QgsStyle entity type being saved
     */
    QgsStyleSaveDialog( QWidget* parent /TransferThis/ = NULL, QgsStyle::StyleEntity type = QgsStyle::SymbolEntity );

    //! returns the text value of the name element
    QString name() const;

    //! returns the text value of the tags element
    QString tags() const;

    //! returns whether the favorite element is checked
    bool isFavorite() const;
};
