class QgsAttributeTypeLoadDialog : QDialog
{
%TypeHeaderCode
#include <qgsattributetypeloaddialog.h>
%End
  public:
    QgsAttributeTypeLoadDialog( QgsVectorLayer *vl );
    ~QgsAttributeTypeLoadDialog();

    /**
     * Overloaded accept method which will write the feature field
     * values, then delegate to QDialog::accept()
     */
    void accept();

    /**
     * Sets predefined vector layer for selection of data
     * @param layer Vector layer which is to be set as predefined one
     */
    void setVectorLayer( QgsVectorLayer *layer );

    /**
     * Getter to value map which is currently active
     * @return value map of vlues selected from layer
     */
    QMap<QString, QVariant> &valueMap();

    /**
     * Returns true if the "Add NULL value" checkbox has been checked.
     *
     * @return true if the "Add NULL value" checkbox has been checked.
     */
    bool insertNull();

};
