FROM ubuntu:16.04

RUN apt install cmake

COPY . /opt/or-tools
WORKDIR /opt/or-tools

RUN make third_party
RUN make cc
RUN make test_cc
RUN make archive_cc
