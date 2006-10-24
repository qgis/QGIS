#################################################################
#
#         QMAKE Project File for Quantum GIS 
# 
#                   Tim Sutton 2006
#
# NOTE: Do not place any hard coded external paths in this file
#       all libs and includes should be specified in settings.pro
#       in the top level qgis directory.
# 
#################################################################

#
# Do not build this file directly - include it in other pros that 
# have subclasses of forms! Qt will not compile any UIs into the 
# lib or app if they are not being used.
#

FORMS =	\
		../widgets/projectionselector/qgsprojectionselectorbase.ui \
		$${WORKDIR}/src/ui/qgisappbase.ui				\
		$${WORKDIR}/src/ui/qgsabout.ui				\
		$${WORKDIR}/src/ui/qgsaddattrdialogbase.ui	        \
		$${WORKDIR}/src/ui/qgsaddattrdialogbase.ui 		\
		$${WORKDIR}/src/ui/qgsattributeactiondialogbase.ui	\
		$${WORKDIR}/src/ui/qgsattributedialogbase.ui		\
		$${WORKDIR}/src/ui/qgsattributetablebase.ui		\
		### $${WORKDIR}/src/ui/qgsattributetabledisplaybase.ui 	\
		$${WORKDIR}/src/ui/qgsbookmarksbase.ui			\
		$${WORKDIR}/src/ui/qgscomposerbase.ui 			\
		$${WORKDIR}/src/ui/qgscomposerlabelbase.ui 		\
		$${WORKDIR}/src/ui/qgscomposermapbase.ui 		\
		$${WORKDIR}/src/ui/qgscomposerpicturebase.ui 		\
		$${WORKDIR}/src/ui/qgscomposerscalebarbase.ui 		\
		$${WORKDIR}/src/ui/qgscomposervectorlegendbase.ui 	\
		$${WORKDIR}/src/ui/qgscompositionbase.ui 		\
		$${WORKDIR}/src/ui/qgscontinuouscolordialogbase.ui	\
		$${WORKDIR}/src/ui/qgscontinuouscolordialogbase.ui 	\
		$${WORKDIR}/src/ui/qgscustomprojectiondialogbase.ui 	\
		$${WORKDIR}/src/ui/qgsdbsourceselectbase.ui		\
		$${WORKDIR}/src/ui/qgsdelattrdialogbase.ui		\
		$${WORKDIR}/src/ui/qgsdelattrdialogbase.ui 		\
		$${WORKDIR}/src/ui/qgsfillstylewidgetbase.ui 		\
		$${WORKDIR}/src/ui/qgsgeomtypedialogbase.ui		\
		$${WORKDIR}/src/ui/qgsgeomtypedialogbase.ui 		\
		$${WORKDIR}/src/ui/qgsgraduatedsymboldialogbase.ui	\
		$${WORKDIR}/src/ui/qgsgraduatedsymboldialogbase.ui 	\
		$${WORKDIR}/src/ui/qgshelpviewerbase.ui			\
		$${WORKDIR}/src/ui/qgsidentifyresultsbase.ui		\
		$${WORKDIR}/src/ui/qgsidentifyresultsbase.ui 		\
		$${WORKDIR}/src/ui/qgslabeldialogbase.ui		\
		$${WORKDIR}/src/ui/qgslabeldialogbase.ui 		\
		$${WORKDIR}/src/ui/qgslayerprojectionselectorbase.ui 	\
		$${WORKDIR}/src/ui/qgslinestyledialogbase.ui		\
		$${WORKDIR}/src/ui/qgslinestyledialogbase.ui 		\
		$${WORKDIR}/src/ui/qgslinestylewidgetbase.ui 		\
		$${WORKDIR}/src/ui/qgsludialogbase.ui			\
		$${WORKDIR}/src/ui/qgsludialogbase.ui 			\
		$${WORKDIR}/src/ui/qgsmapserverexportbase.ui		\
		$${WORKDIR}/src/ui/qgsmarkerdialogbase.ui		\
		$${WORKDIR}/src/ui/qgsmarkerdialogbase.ui 		\
		$${WORKDIR}/src/ui/qgsmeasurebase.ui			\
		$${WORKDIR}/src/ui/qgsmeasurebase.ui 			\
		$${WORKDIR}/src/ui/qgsmessageviewer.ui			\
		$${WORKDIR}/src/ui/qgsnewconnectionbase.ui 		\
		$${WORKDIR}/src/ui/qgsnewhttpconnectionbase.ui		\
		$${WORKDIR}/src/ui/qgsnewhttpconnectionbase.ui 		\
		$${WORKDIR}/src/ui/qgsoptionsbase.ui			\
		$${WORKDIR}/src/ui/qgspastetransformationsbase.ui	\
		$${WORKDIR}/src/ui/qgspastetransformationsbase.ui 	\
		$${WORKDIR}/src/ui/qgspatterndialogbase.ui		\
		$${WORKDIR}/src/ui/qgspatterndialogbase.ui 		\
		$${WORKDIR}/src/ui/qgspgquerybuilderbase.ui 		\
		$${WORKDIR}/src/ui/qgspluginmanagerbase.ui		\
		$${WORKDIR}/src/ui/qgspointstylewidgetbase.ui 		\
		$${WORKDIR}/src/ui/qgsprojectpropertiesbase.ui		\
		$${WORKDIR}/src/ui/qgsrasterlayerpropertiesbase.ui	\
		$${WORKDIR}/src/ui/qgsserversourceselectbase.ui		\
		$${WORKDIR}/src/ui/qgssinglesymboldialogbase.ui		\
		$${WORKDIR}/src/ui/qgsuniquevaluedialogbase.ui		\
		$${WORKDIR}/src/ui/qgsuniquevaluedialogbase.ui 		\
		$${WORKDIR}/src/ui/qgsvectorlayerpropertiesbase.ui		


