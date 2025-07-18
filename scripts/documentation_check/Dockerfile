ARG PYTHON_VERSION=3.13
FROM python:${PYTHON_VERSION}

WORKDIR /qgis

ARG DOXYGEN_VERSION=1.9.8
ENV DEBIAN_FRONTEND=noninteractive

RUN cd /usr/local && \
    wget https://www.doxygen.nl/files/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz && \
    tar -xzf doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz && \
    mv doxygen-${DOXYGEN_VERSION} doxygen && \
    python -m pip install --upgrade pip && \
    pip install autopep8 nose2 mock termcolor

RUN apt-get update \
    && apt-get install --no-install-recommends --no-install-suggests -y \
    cmake\
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY ./documentation_check.sh /

ENTRYPOINT [ "/documentation_check.sh", "/qgis", "/usr/local/doxygen"]
