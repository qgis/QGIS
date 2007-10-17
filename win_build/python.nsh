;Python include file for NSIS
;Written by Tisham Dhar(what_nick) (mailto:tisham at apogee.com.au)
!ifndef PYTHON_USED
!define PYTHON_USED

!macro CHECK_PYTHON
# Read handler for python files from registry
ReadRegStr $9 HKEY_LOCAL_MACHINE "SOFTWARE\Python\PythonCore\2.5\InstallPath" ""

# The point pythonskip needs to be defined
IfFileExists $9\python.exe ok ng

ng:
   MessageBox MB_OK "Python is not installed on this system.$\nUsing bundled python25.dll"
   SetOutPath "$INSTDIR"
   File "C:\Program Files\qgis${PRODUCT_VERSION_NUMBER}\pydep\python25.dll"
   goto sipng
ok:
   MessageBox MB_OK "Python located $9"

testsip:
  IfFileExists $9\Lib\site-packages\sip.pyd sipok sipng
sipok:
  goto testpyqt4

sipng:
  #copy over included sip
  SetOutPath "$INSTDIR\python"
  File "C:\Program Files\qgis${PRODUCT_VERSION_NUMBER}\pydep\sipconfig.py"
  File "C:\Program Files\qgis${PRODUCT_VERSION_NUMBER}\pydep\sip.pyd"
  goto pyqtng

testpyqt4:
  IfFileExists $9\Lib\site-packages\PyQt4 pyqtok pyqtng

pyqtng:
  MessageBox MB_OK "PyQt4 is not installed.$\nInstalling Bundled PyQt4"
#copy over bundled pyqt4 instead of skipping
  goto installpyqt4
pyqtok:
  MessageBox MB_OK "PyQt4 located.$\nNote: Overwriting with included copy."

installpyqt4:
  SetOutPath "$INSTDIR\python"
  File /r    "C:\Program Files\qgis${PRODUCT_VERSION_NUMBER}\pydep\PyQt4"
pymacroend:

!macroend

!endif