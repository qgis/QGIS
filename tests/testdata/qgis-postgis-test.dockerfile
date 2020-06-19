
ARG PG_VERSION=11.0-2.5

FROM kartoza/postgis:${PG_VERSION}

ADD auth_system/certs_keys/postgres.crt /etc/ssl/certs/postgres_cert.crt
ADD auth_system/certs_keys/postgres.key /etc/ssl/private/postgres_key.key
ADD auth_system/certs_keys/issuer_ca_cert.pem /etc/ssl/certs/issuer_ca_cert.pem

RUN chmod 400 /etc/ssl/private/postgres_key.key

# Compile and install PointCloud.
# NOTE: release 1.2.0 would not build against PostgreSQL-11:
# https://github.com/pgpointcloud/pointcloud/issues/248
RUN apt-get -y update; apt-get -y install \
  autoconf \
  build-essential \
  libxml2-dev \
  postgresql-client-11 \
  postgresql-server-dev-11 \
  zlib1g-dev
RUN wget -O- \
  https://github.com/pgpointcloud/pointcloud/archive/v1.2.1.tar.gz \
  | tar xz && \
  cd pointcloud-1.2.1 && \
  ./autogen.sh && \
  ./configure --with-pgconfig=/usr/lib/postgresql/11/bin/pg_config && \
  make && make install && \
  cd .. && rm -Rf pointcloud-1.2.1
