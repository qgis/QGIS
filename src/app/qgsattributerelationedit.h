#ifndef QGSATTRIBUTERELATIONEDIT_H
#define QGSATTRIBUTERELATIONEDIT_H

#include "ui_qgsattributerelationedit.h"

#include "qgseditorconfigwidget.h"
#include "qgsfeature.h"
#include "qgsvectordataprovider.h"
#include "qgshelp.h"
#include "qgis_app.h"
#include <QWidget>

/*
namespace Ui {
class QgsAttributeRelationEdit;
}

class QgsAttributeRelationEdit : public QWidget
{
*/

class APP_EXPORT QgsAttributeRelationEdit: public QWidget, private Ui::QgsAttributeRelationEdit
{
    Q_OBJECT

  public:
    explicit QgsAttributeRelationEdit(const QString &relationid, QWidget *parent = 0);
    ~QgsAttributeRelationEdit();

  /**
   * Setter for combo cardinality item
   */
  void setCardinalityCombo( const QString &cardinalityComboItem );

  /**
   * Setter for combo cardinality
   */
  void setCardinality( const QString &cardinality );

  /**
   * Getter for combo cardinality
   */
  QString cardinality();

  QString mRelationId;
  private:

    //Ui::QgsAttributeRelationEdit *ui;
};

#endif // QGSATTRIBUTERELATIONEDIT_H
