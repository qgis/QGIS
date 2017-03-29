class QgsProcessingAlgorithm
{
%TypeHeaderCode
#include <qgsprocessingalgorithm.h>
%End

  public:

     enum Flag
    {
      FlagHideFromToolbox,
      FlagHideFromModeler,
      FlagSupportsBatch,
      FlagDeprecated,
    };
    typedef QFlags<QgsProcessingAlgorithm::Flag> Flags;

    QgsProcessingAlgorithm();
    virtual ~QgsProcessingAlgorithm();

    virtual QString name() const = 0;
    virtual QString displayName() const = 0;
    virtual QStringList tags() const;
    virtual QIcon icon() const;
    virtual QString svgIconPath() const;
    virtual QString group() const;
    virtual Flags flags() const;
};
QFlags<QgsProcessingAlgorithm::Flag> operator|(QgsProcessingAlgorithm::Flag f1, QFlags<QgsProcessingAlgorithm::Flag> f2);
