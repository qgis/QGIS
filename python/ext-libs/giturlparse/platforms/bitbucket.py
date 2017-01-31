# Imports
from .base import BasePlatform

class BitbucketPlatform(BasePlatform):
    PATTERNS = {
        'https': r'https://(?P<_user>.+)@(?P<domain>.+)/(?P<owner>.+)/(?P<repo>.+).git',
        'ssh': r'git@(?P<domain>.+):(?P<owner>.+)/(?P<repo>.+).git'
    }
    FORMATS = {
        'https': r'https://%(owner)s@%(domain)s/%(owner)s/%(repo)s.git',
        'ssh': r'git@%(domain)s:%(owner)s/%(repo)s.git'
    }
    DOMAINS = ('bitbucket.org',)
    DEFAULTS = {
        '_user': 'git'
    }
