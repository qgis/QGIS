# The following has been generated automatically from src/core/processing/qgsprocessingparameters.h
QgsProcessingFeatureSourceDefinition.FlagOverrideDefaultGeometryCheck = QgsProcessingFeatureSourceDefinition.Flag.FlagOverrideDefaultGeometryCheck
QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature = QgsProcessingFeatureSourceDefinition.Flag.FlagCreateIndividualOutputPerInputFeature
QgsProcessingFeatureSourceDefinition.Flags = lambda flags=0: QgsProcessingFeatureSourceDefinition.Flag(flags)
_force_int = lambda v: v if isinstance(v, int) else int(v.value)


QgsProcessingFeatureSourceDefinition.Flag.__bool__ = lambda flag: _force_int(flag)
QgsProcessingFeatureSourceDefinition.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProcessingFeatureSourceDefinition.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProcessingFeatureSourceDefinition.Flag.__or__ = lambda flag1, flag2: QgsProcessingFeatureSourceDefinition.Flag(_force_int(flag1) | _force_int(flag2))
QgsProcessingParameterDefinition.FlagAdvanced = QgsProcessingParameterDefinition.Flag.FlagAdvanced
QgsProcessingParameterDefinition.FlagHidden = QgsProcessingParameterDefinition.Flag.FlagHidden
QgsProcessingParameterDefinition.FlagOptional = QgsProcessingParameterDefinition.Flag.FlagOptional
QgsProcessingParameterDefinition.FlagIsModelOutput = QgsProcessingParameterDefinition.Flag.FlagIsModelOutput
QgsProcessingParameterDefinition.Flags = lambda flags=0: QgsProcessingParameterDefinition.Flag(flags)
_force_int = lambda v: v if isinstance(v, int) else int(v.value)


QgsProcessingParameterDefinition.Flag.__bool__ = lambda flag: _force_int(flag)
QgsProcessingParameterDefinition.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProcessingParameterDefinition.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProcessingParameterDefinition.Flag.__or__ = lambda flag1, flag2: QgsProcessingParameterDefinition.Flag(_force_int(flag1) | _force_int(flag2))
QgsProcessingParameterFile.File = QgsProcessingParameterFile.Behavior.File
QgsProcessingParameterFile.Folder = QgsProcessingParameterFile.Behavior.Folder
QgsProcessingParameterNumber.Integer = QgsProcessingParameterNumber.Type.Integer
QgsProcessingParameterNumber.Double = QgsProcessingParameterNumber.Type.Double
QgsProcessingParameterField.Any = QgsProcessingParameterField.DataType.Any
QgsProcessingParameterField.Numeric = QgsProcessingParameterField.DataType.Numeric
QgsProcessingParameterField.String = QgsProcessingParameterField.DataType.String
QgsProcessingParameterField.DateTime = QgsProcessingParameterField.DataType.DateTime
QgsProcessingParameterField.Binary = QgsProcessingParameterField.DataType.Binary
QgsProcessingParameterField.Boolean = QgsProcessingParameterField.DataType.Boolean
QgsProcessingParameterDateTime.DateTime = QgsProcessingParameterDateTime.Type.DateTime
QgsProcessingParameterDateTime.Date = QgsProcessingParameterDateTime.Type.Date
QgsProcessingParameterDateTime.Time = QgsProcessingParameterDateTime.Type.Time
