# Mac Plist Macros

FUNCTION (GET_VERSION_PLIST PLISTFILE OUTVAR)
	SET (PVERSION "")
	IF (EXISTS ${PLISTFILE})
		FILE (READ "${PLISTFILE}" info_plist)
		STRING (REGEX REPLACE "\n" "" info_plist "${info_plist}")
		STRING (REGEX MATCH "<key>CFBundleShortVersionString</key>[ \t]*<string>([0-9\\.]*)</string>" PLISTVERSION "${info_plist}")
		STRING (REGEX REPLACE "<key>CFBundleShortVersionString</key>[ \t]*<string>([0-9\\.]*)</string>" "\\1" PVERSION "${PLISTVERSION}")
	ENDIF (EXISTS ${PLISTFILE})
	SET (${OUTVAR} ${PVERSION} PARENT_SCOPE)
ENDFUNCTION (GET_VERSION_PLIST)
