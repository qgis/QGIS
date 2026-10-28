FROM nginx:alpine

RUN apk add --no-cache openssl

COPY gen-certs.sh /gen-certs.sh
COPY nginx.conf /etc/nginx/nginx.conf

RUN sh /gen-certs.sh

EXPOSE 4442 4443 4444

CMD ["nginx", "-g", "daemon off;"]
