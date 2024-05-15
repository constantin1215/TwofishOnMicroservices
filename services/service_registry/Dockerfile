FROM ubuntu:20.04

ADD / /qt_app/

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get -y install  \
    python3 \
    python3-pip \
    git \
    cmake \
    build-essential \
    libdbus-1-3 \
    libpulse-mainloop-glib0

RUN pip3 install aqtinstall

ARG QT=6.6.3
ARG QT_MODULES='qthttpserver qtwebsockets'
ARG QT_HOST=linux
ARG QT_TARGET=desktop
ARG QT_ARCH=gcc_64
RUN aqt install --outputdir /opt/qt ${QT} ${QT_HOST} ${QT_TARGET} ${QT_ARCH} -m ${QT_MODULES}

ENV PATH /opt/qt/${QT}/gcc_64/bin:$PATH
ENV QT_PLUGIN_PATH /opt/qt/${QT}/gcc_64/plugins/
ENV QML_IMPORT_PATH /opt/qt/${QT}/gcc_64/qml/
ENV QML2_IMPORT_PATH /opt/qt/${QT}/gcc_64/qml/

RUN cd qt_app && qmake -o Makefile service_registry.pro && make -j$(nproc)

ENTRYPOINT ["./qt_app/service_registry"]
