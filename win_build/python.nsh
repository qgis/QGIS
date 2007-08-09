;Python include file for NSIS
;Written by Tisham Dhar(what_nick) (mailto:tisham at apogee.com.au)
!ifndef PYTHON_USED
!define PYTHON_USED

!macro CHECK_PYTHON
# Read handler for python files from registry
ReadRegStr $9 HKEY_LOCAL_MACHINE "SOFTWARE\Python\PythonCore\2.5\InstallPath" ""

IfFileExists $9 ok ng
ng:
   MessageBox MB_OK "Python is not installed on this system.$\nPlease install Python2.5 first."
   Quit
ok:
   MessageBox MB_OK "Python located $9"

!macroend

!endif