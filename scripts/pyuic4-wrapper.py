try:
    import sip
    sip.setapi("QVariant", 2)
except:
    pass

try:
    import PyQt4.uic.pyuic
except ImportError:
    import PyQt5.uic.pyuic
