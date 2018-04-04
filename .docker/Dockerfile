FROM      ubuntu:18.04
MAINTAINER Denis Rouzaud <denis@opengis.ch>

LABEL Description="Docker container with QGIS dependencies" Vendor="QGIS.org" Version="1.0"

RUN  apt-get update
RUN apt-get install -y software-properties-common
RUN apt-get update
RUN apt-get install -y curl tar

WORKDIR /root
RUN curl -s -S -O https://www.riverbankcomputing.com/hg/sip/archive/tip.tar.gz \
 && tar xzf tip.tar.gz

RUN apt-get install -y make

RUN mkdir /root/sip419
RUN mv  $(find -type d -iname 'sip-*')/* sip419
WORKDIR /root/sip419
RUN sed -i -r "s/RM_HEXVERSION = '04ffff'/RM_HEXVERSION = '041308'/; s/RM_RELEASE = '4.255.255'/RM_RELEASE = '4.19.8'/" build.py
#RUN python3 build.py prepare && python3 configure.py && make && make install

RUN apt-get install -y python3-sip

WORKDIR /root
RUN curl -s -S -O https://svwh.dl.sourceforge.net/project/pyqt/PyQt5/PyQt-5.9.2/PyQt5_gpl-5.9.2.tar.gz
RUN tar xzf PyQt5_gpl-5.9.2.tar.gz
WORKDIR /root/PyQt5_gpl-5.9.2
RUN  apt-get update
RUN apt-get install -y qt5-qmake
RUN apt-get install -y g++
RUN apt-get install -y flex bison
RUN apt-get install -y python3-dev

WORKDIR /root/sip419
RUN python3 build.py prepare && python3 configure.py && make && make install

RUN apt-get install -y qt5-default
RUN apt-get install -y libqt5svg5-dev
WORKDIR /root/PyQt5_gpl-5.9.2
RUN python3 configure.py --verbose --confirm-license --qmake /usr/lib/x86_64-linux-gnu/qt5/bin/qmake --qsci-api --enable QtGui --enable QtSvg --enable QtSql
RUN make && make install

WORKDIR /root
RUN curl -s -S -O https://cfhcable.dl.sourceforge.net/project/pyqt/QScintilla2/QScintilla-2.10.3/QScintilla_gpl-2.10.3.tar.gz
RUN tar xzf QScintilla_gpl-2.10.3.tar.gz
WORKDIR /root/QScintilla_gpl-2.10.3/Python
# RUN python3 configure.py --pyqt=PyQt5
# RUN make
# RUN make install
