# Imports
from .base import BasePlatform


class FriendCodePlatform(BasePlatform):
    DOMAINS = ('friendco.de',)
    PATTERNS = {
        'https': r'https://(?P<domain>.+)/(?P<owner>.+)@user/(?P<repo>.+).git',
    }
    FORMATS = {
        'https': r'https://%(domain)s/%(owner)s@user/%(repo)s.git',
    }
