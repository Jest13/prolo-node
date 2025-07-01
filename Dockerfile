# Multistage docker build, requires docker 17.05

# builder stage
FROM ubuntu:20.04 AS builder

RUN set -ex && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get --no-install-recommends --yes install \
        automake \
        autotools-dev \
        bsdmainutils \
        build-essential \
        ca-certificates \
        ccache \
        cmake \
        curl \
        git \
        libtool \
        pkg-config \
        gperf \
        gettext \
        autoconf \
        libssl-dev \
        libncurses5-dev \
        unzip \
        python3 \
        libunbound-dev \
        libunwind8-dev \
        libpcsclite-dev \
        libhidapi-dev \
        libzmq3-dev \
        libminiupnpc-dev \
        libsodium-dev \
        libboost-all-dev \
        libreadline-dev \
        libpgm-dev \
        libnorm-dev \
        libusb-1.0-0-dev

WORKDIR /src
COPY . .

ARG NPROC
RUN set -ex && \
    git submodule update --init --recursive && \
    rm -rf build && \
    mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j${NPROC:-$(nproc)}

# runtime stage
FROM ubuntu:20.04

RUN set -ex && \
    apt-get update && \
    apt-get --no-install-recommends --yes install \
        ca-certificates \
        libunbound8 \
        libunwind8 \
        libminiupnpc17 \
        libzmq5 \
        libreadline8 \
        libhidapi-libusb0 \
        libsodium23 \
        libboost-system1.71.0 \
        libboost-filesystem1.71.0 \
        libboost-thread1.71.0 \
        libboost-chrono1.71.0 \
        libboost-date-time1.71.0 \
        libboost-regex1.71.0 \
        libboost-serialization1.71.0 \
        libboost-program-options1.71.0 && \
    apt-get clean && \
    rm -rf /var/lib/apt

COPY --from=builder /src/build/bin /usr/local/bin/

# Create prolo user
RUN adduser --system --group --disabled-password prolo && \
    mkdir -p /wallet /home/prolo/.bitprolo && \
    chown -R prolo:prolo /home/prolo/.bitprolo && \
    chown -R prolo:prolo /wallet

VOLUME /home/prolo/.bitprolo
VOLUME /wallet

EXPOSE 45000
EXPOSE 45001

USER prolo

ENTRYPOINT ["prolod"]
CMD ["--p2p-bind-ip=0.0.0.0", "--p2p-bind-port=45000", "--rpc-bind-ip=0.0.0.0", "--rpc-bind-port=45001", "--non-interactive", "--confirm-external-bind"]