# Multistage docker build, requires docker 17.05

# builder stage
FROM ubuntu:20.04 AS builder

RUN set -ex && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get --no-install-recommends --yes install \
        automake \
        autotools-dev \
        autoconf \
        libtool \
        libtool-bin \
        pkg-config \
        libssl-dev \
        bsdmainutils \
        build-essential \
        cmake \
        curl \
        git \
        gperf \
        ca-certificates \
        python3 \
        file \
        wget \
        unzip


WORKDIR /src
COPY . .

ARG NPROC
RUN set -ex && \
    git submodule init && git submodule update && \
    rm -rf build && \
    if [ -z "$NPROC" ] ; \
    then make -j$(nproc) depends target=x86_64-linux-gnu ; \
    else make -j$NPROC depends target=x86_64-linux-gnu ; \
    fi

# runtime stage
FROM ubuntu:20.04

RUN set -ex && \
    apt-get update && \
    apt-get --no-install-recommends --yes install ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt
COPY --from=builder /src/build/x86_64-linux-gnu/release/bin /usr/local/bin/

# Create prolo user
RUN adduser --system --group --disabled-password prolo && \
	mkdir -p /wallet /home/prolo/.bitprolo && \
	chown -R prolo:prolo /home/prolo/.bitprolo && \
	chown -R prolo:prolo /wallet

# Contains the blockchain
VOLUME /home/prolo/.bitprolo

# Generate your wallet via accessing the container and run:
# cd /wallet
# prolo-wallet-cli
VOLUME /wallet

EXPOSE 18080
EXPOSE 18081

# switch to user prolo
USER prolo

ENTRYPOINT ["prolod"]
CMD ["--p2p-bind-ip=0.0.0.0", "--p2p-bind-port=18080", "--rpc-bind-ip=0.0.0.0", "--rpc-bind-port=18081", "--non-interactive", "--confirm-external-bind"]

