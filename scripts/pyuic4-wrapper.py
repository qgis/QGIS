try:
    import sip
    sip.setapi("QVariant", 2)
except:
    pass

import PyQt4.uic.pyuic
