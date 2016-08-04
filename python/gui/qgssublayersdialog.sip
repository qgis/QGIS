class QgsSublayersDialog : QDialog
{
%TypeHeaderCode
#include <qgssublayersdialog.h>
%End
  public:

    enum ProviderType
    {
      Ogr,
      Gdal,
      Vsifile
    };

    //! A structure that defines layers for the purpose of this dialog
    //! @note added in 2.16
    struct LayerDefinition
    {
      LayerDefinition();

      int layerId;        //!< identifier of the layer (one unique layer id may have multiple types though)
      QString layerName;  //!< name of the layer (not necessarily unique)
      int count;          //!< number of features (might be unused)
      QString type;       //!< extra type depending on the use (e.g. geometry type for vector sublayers)
    };

    //! List of layer definitions for the purpose of this dialog
    //! @note added in 2.16
    typedef QList<QgsSublayersDialog::LayerDefinition> LayerDefinitionList;

    QgsSublayersDialog( ProviderType providerType, const QString& name, QWidget* parent /TransferThis/ = 0, const Qt::WindowFlags& fl = 0 );
    ~QgsSublayersDialog();

    //! Populate the table with layers
    //! @note added in 2.16
    void populateLayerTable( const QgsSublayersDialog::LayerDefinitionList& list );

    //! Returns list of selected layers
    //! @note added in 2.16
    QgsSublayersDialog::LayerDefinitionList selection();

    //! @deprecated since 2.16 - use other populateLayerTable() variant
    void populateLayerTable( const QStringList& theList, const QString& delim = ":" ) /Deprecated/;
    //! @deprecated since 2.16 - use selection()
    QStringList selectionNames() /Deprecated/;
    //! @deprecated since 2.16 - use selection()
    QList<int> selectionIndexes() /Deprecated/;

  public slots:
    void on_buttonBox_helpRequested();
    int exec();
};
