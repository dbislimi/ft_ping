FROM debian:bookworm-slim

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
	gcc \
	make \
	git \
	valgrind \
	inetutils-ping \
	dnsutils

WORKDIR /app

COPY . .

CMD ["/bin/bash"]
