/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssublayersdialog.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSublayersDialog : QDialog
{

%TypeHeaderCode
#include "qgssublayersdialog.h"
%End
  public:

    enum ProviderType
    {
      Ogr,
      Gdal,
      Vsifile
    };

    struct LayerDefinition
    {
      int layerId;
      QString layerName;  //!< Name of the layer (not necessarily unique)
      int count;
      QString type;       //!< Extra type depending on the use (e.g. geometry type for vector sublayers)
    };

    typedef QList<QgsSublayersDialog::LayerDefinition> LayerDefinitionList;

    QgsSublayersDialog( ProviderType providerType,
                        const QString &name,
                        QWidget *parent /TransferThis/ = 0,
                        Qt::WindowFlags fl = 0 );
%Docstring
Constructor for QgsSublayersDialog
%End

    ~QgsSublayersDialog();

    void populateLayerTable( const LayerDefinitionList &list );
%Docstring
Populate the table with layers

.. versionadded:: 2.16
%End

    LayerDefinitionList selection();
%Docstring
Returns list of selected layers

.. versionadded:: 2.16
%End

    void setShowAddToGroupCheckbox( bool showAddToGroupCheckbox );
%Docstring
Set if we should display the add to group checkbox

.. versionadded:: 3.0
%End

    bool showAddToGroupCheckbox() const;
%Docstring
If we should display the add to group checkbox

.. versionadded:: 3.0
%End

    bool addToGroupCheckbox() const;
%Docstring
If we should add layers in a group

.. versionadded:: 3.0
%End

    int countColumn() const;
%Docstring
Returns column with count or -1

.. versionadded:: 3.0
%End

  public slots:
    virtual int exec();


  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssublayersdialog.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
