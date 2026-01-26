/***************************************************************************
    qgsvectorlayerdigitizingproperties.cpp
    --------------------------------------
  copyright            : (C) 2018 by Matthias Kuhn
  email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerdigitizingproperties.h"

#include "qgsanalysis.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsdoublespinbox.h"
#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckfactory.h"
#include "qgsgeometrycheckregistry.h"
#include "qgsgeometryoptions.h"
#include "qgsmaplayercombobox.h"
#include "qgsproject.h"
#include "qgsunittypes.h"

#include <QFormLayout>

#include "moc_qgsvectorlayerdigitizingproperties.cpp"

QgsVectorLayerDigitizingPropertiesPage::QgsVectorLayerDigitizingPropertiesPage( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( u"mOptsPage_Digitizing"_s );

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayer );

  if ( vlayer && vlayer->isSpatial() )
  {
    mRemoveDuplicateNodesCheckbox->setEnabled( true );
    mGeometryPrecisionLineEdit->setEnabled( true );
    mGeometryPrecisionLineEdit->setValidator( new QDoubleValidator( mGeometryPrecisionLineEdit ) );

    const double precision( vlayer->geometryOptions()->geometryPrecision() );
    QString precisionStr( QLocale().toString( precision, 'g', 17 ) );
    if ( precision == 0.0 )
      precisionStr = QString();
    mGeometryPrecisionLineEdit->setText( precisionStr );

    mRemoveDuplicateNodesManuallyActivated = vlayer->geometryOptions()->removeDuplicateNodes();
    mRemoveDuplicateNodesCheckbox->setChecked( mRemoveDuplicateNodesManuallyActivated );
    if ( !precisionStr.isNull() )
      mRemoveDuplicateNodesCheckbox->setEnabled( false );
    connect( mGeometryPrecisionLineEdit, &QLineEdit::textChanged, this, [this] {
      if ( !mGeometryPrecisionLineEdit->text().isEmpty() )
      {
        if ( mRemoveDuplicateNodesCheckbox->isEnabled() )
          mRemoveDuplicateNodesManuallyActivated = mRemoveDuplicateNodesCheckbox->isChecked();
        mRemoveDuplicateNodesCheckbox->setEnabled( false );
        mRemoveDuplicateNodesCheckbox->setChecked( true );
      }
      else
      {
        mRemoveDuplicateNodesCheckbox->setEnabled( true );
        mRemoveDuplicateNodesCheckbox->setChecked( mRemoveDuplicateNodesManuallyActivated );
      }
    } );

    mPrecisionUnitsLabel->setText( u"[%1]"_s.arg( QgsUnitTypes::toAbbreviatedString( vlayer->crs().mapUnits() ) ) );

    QLayout *geometryCheckLayout = new QVBoxLayout();
    const QList<QgsGeometryCheckFactory *> geometryCheckFactories = QgsAnalysis::geometryCheckRegistry()->geometryCheckFactories( vlayer, QgsGeometryCheck::FeatureNodeCheck, QgsGeometryCheck::Flag::AvailableInValidation );
    const QStringList activeChecks = vlayer->geometryOptions()->geometryChecks();
    for ( const QgsGeometryCheckFactory *factory : geometryCheckFactories )
    {
      QCheckBox *cb = new QCheckBox( factory->description() );
      cb->setChecked( activeChecks.contains( factory->id() ) );
      mGeometryCheckFactoriesGroupBoxes.insert( cb, factory->id() );
      geometryCheckLayout->addWidget( cb );
    }
    mGeometryValidationGroupBox->setLayout( geometryCheckLayout );
    mGeometryValidationGroupBox->setVisible( !geometryCheckFactories.isEmpty() );

    QLayout *topologyCheckLayout = new QVBoxLayout();
    const QList<QgsGeometryCheckFactory *> topologyCheckFactories = QgsAnalysis::geometryCheckRegistry()->geometryCheckFactories( vlayer, QgsGeometryCheck::LayerCheck, QgsGeometryCheck::Flag::AvailableInValidation );

    for ( const QgsGeometryCheckFactory *factory : topologyCheckFactories )
    {
      QCheckBox *cb = new QCheckBox( factory->description() );
      cb->setChecked( activeChecks.contains( factory->id() ) );
      mGeometryCheckFactoriesGroupBoxes.insert( cb, factory->id() );
      topologyCheckLayout->addWidget( cb );
      if ( factory->id() == "QgsGeometryGapCheck"_L1 )
      {
        const QVariantMap gapCheckConfig = vlayer->geometryOptions()->checkConfiguration( u"QgsGeometryGapCheck"_s );

        mGapCheckAllowExceptionsActivatedCheckBox = new QgsCollapsibleGroupBox( tr( "Allowed Gaps" ) );
        mGapCheckAllowExceptionsActivatedCheckBox->setCheckable( true );
        mGapCheckAllowExceptionsActivatedCheckBox->setChecked( gapCheckConfig.value( u"allowedGapsEnabled"_s, false ).toBool() );
        QGridLayout *layout = new QGridLayout();
        mGapCheckAllowExceptionsActivatedCheckBox->setLayout( layout );
        topologyCheckLayout->addWidget( mGapCheckAllowExceptionsActivatedCheckBox );
        mGapCheckAllowExceptionsLayerComboBox = new QgsMapLayerComboBox();
        mGapCheckAllowExceptionsLayerComboBox->setFilters( Qgis::LayerFilter::PolygonLayer );
        mGapCheckAllowExceptionsLayerComboBox->setExceptedLayerList( QList<QgsMapLayer *> { vlayer } );
        mGapCheckAllowExceptionsLayerComboBox->setLayer( QgsProject::instance()->mapLayer( gapCheckConfig.value( u"allowedGapsLayer"_s ).toString() ) );
        layout->addWidget( new QLabel( tr( "Layer" ) ), 0, 0 );
        layout->addWidget( mGapCheckAllowExceptionsLayerComboBox, 0, 1 );
        mGapCheckAllowExceptionsBufferSpinBox = new QgsDoubleSpinBox();
        mGapCheckAllowExceptionsBufferSpinBox->setInputMethodHints( Qt::ImhFormattedNumbersOnly );
        mGapCheckAllowExceptionsBufferSpinBox->setSuffix( QgsUnitTypes::toAbbreviatedString( vlayer->crs().mapUnits() ) );
        mGapCheckAllowExceptionsBufferSpinBox->setValue( gapCheckConfig.value( u"allowedGapsBuffer"_s ).toDouble() );
        layout->addWidget( new QLabel( tr( "Buffer" ) ), 0, 2 );
        layout->addWidget( mGapCheckAllowExceptionsBufferSpinBox, 0, 3 );
      }
    }
    mTopologyChecksGroupBox->setLayout( topologyCheckLayout );
    mTopologyChecksGroupBox->setVisible( !topologyCheckFactories.isEmpty() );
  }
  else
  {
    mRemoveDuplicateNodesCheckbox->setEnabled( false );
    mGeometryPrecisionLineEdit->setEnabled( false );
    mGeometryAutoFixesGroupBox->setEnabled( false );
  }

  setProperty( "helpPage", u"working_with_vector/vector_properties.html#digitizing-properties"_s );
}

void QgsVectorLayerDigitizingPropertiesPage::apply()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mLayer );
  if ( !vlayer )
  {
    return;
  }

  vlayer->geometryOptions()->setRemoveDuplicateNodes( mRemoveDuplicateNodesCheckbox->isChecked() );
  bool ok = true;
  double precision( QLocale().toDouble( mGeometryPrecisionLineEdit->text(), &ok ) );
  if ( !ok )
    precision = 0.0;
  vlayer->geometryOptions()->setGeometryPrecision( precision );

  QStringList activeChecks;
  QHash<QCheckBox *, QString>::const_iterator it;
  for ( it = mGeometryCheckFactoriesGroupBoxes.constBegin(); it != mGeometryCheckFactoriesGroupBoxes.constEnd(); ++it )
  {
    if ( it.key()->isChecked() )
      activeChecks << it.value();
  }
  vlayer->geometryOptions()->setGeometryChecks( activeChecks );

  if ( mGapCheckAllowExceptionsActivatedCheckBox )
  {
    QVariantMap gapCheckConfig;
    gapCheckConfig.insert( u"allowedGapsEnabled"_s, mGapCheckAllowExceptionsActivatedCheckBox->isChecked() );
    QgsMapLayer *currentLayer = mGapCheckAllowExceptionsLayerComboBox->currentLayer();
    gapCheckConfig.insert( u"allowedGapsLayer"_s, currentLayer ? currentLayer->id() : QString() );
    gapCheckConfig.insert( u"allowedGapsBuffer"_s, mGapCheckAllowExceptionsBufferSpinBox->value() );

    vlayer->geometryOptions()->setCheckConfiguration( u"QgsGeometryGapCheck"_s, gapCheckConfig );
  }
}


QgsVectorLayerDigitizingPropertiesFactory::QgsVectorLayerDigitizingPropertiesFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QIcon( ":/images/themes/default/propertyicons/digitizing.svg" ) );
  setTitle( tr( "Digitizing" ) );
}

QgsMapLayerConfigWidget *QgsVectorLayerDigitizingPropertiesFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const
{
  Q_UNUSED( dockWidget )
  return new QgsVectorLayerDigitizingPropertiesPage( layer, canvas, parent );
}

bool QgsVectorLayerDigitizingPropertiesFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == Qgis::LayerType::Vector;
}
