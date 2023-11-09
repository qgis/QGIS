cmake_minimum_required(VERSION 2.8.11)

if(o2_WITH_FACEBOOK)
    add_subdirectory(facebookdemo)
endif(o2_WITH_FACEBOOK)

if(o2_WITH_TWITTER)
    add_subdirectory(twitterdemo)
endif(o2_WITH_TWITTER)

if(o2_WITH_SPOTIFY)
    add_subdirectory(spotifydemo)
endif(o2_WITH_SPOTIFY)
