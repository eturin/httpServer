FROM alpine

RUN apk update                                                      \
    && apk add mc bash strace build-base clang  strace boost        \
    boost-dev git make cmake postgresql-dev libxml2 curl libstdc++  \
    netcat-openbsd iputils net-tools 

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

RUN mkdir -p /opt/api/                                              \
    && cd /opt/api/                                                 \
    && git clone https://github.com/jtv/libpqxx.git                 \
    && cd libpqxx                                                   \
    && git checkout 7.3.1                                           \
    && mkdir build                                                  \
#    && ./configure                                                  \
    && cd build                                                     \
    && cmake ../ && make install                                    \
#    && make install                                                 \
    && cd /opt/api/                                                 \ 
    && rm -rf libpqxx                                               \                                            
    && git clone https://github.com/eturin/httpServer.git project   \
    && mkdir project/build                                          \
    && cd project/build                                             \
    && cmake ../                                                    \
    && make
   
WORKDIR /opt/api/project/build
ENTRYPOINT ./httpServer 0.0.0.0 8888 10 . 
