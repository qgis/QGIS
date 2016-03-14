try:
    import sip
    sip.setapi("QVariant", 2)
except:
    pass

import PyQt.uic.pyuic
