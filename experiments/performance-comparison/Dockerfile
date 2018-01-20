# NOTE: I use ubuntu and not alpine because in alpine I can't use edlib-aligner built on arch.
FROM ubuntu

MAINTAINER Martin Sosic <sosic.martin@gmail.com>

WORKDIR ~

# Install needed packages.
RUN apt-get update && \
    apt-get install -y git && \
    apt-get install -y cmake && \
    apt-get install -y clang

# Install Edlib.
# Expects edlib-aligner to be present in the directory. Make sure to build it and place it here.
# TODO: consider pulling edlib from git and compiling it here? Then I can't use current local state
# of files, but it is compiled directly for ubuntu, and not for arch and then moved to ubuntu.
COPY edlib-aligner /bin

# Install Parasail.
RUN git clone -b v1.1 https://github.com/jeffdaily/parasail.git parasail-v1.1 && \
    cd parasail-v1.1 && mkdir build && cd build && cmake .. && make parasail_aligner && \
    cp parasail_aligner /bin

# Install Seqan. We use it through modified edlib aligner.
RUN git clone -b seqan-v2.2.0 https://github.com/seqan/seqan.git seqan-v2.2.0 && \
    git clone -b seqan-test https://github.com/martinsos/edlib.git edlib-seqan && \
    cd edlib-seqan/src && make aligner SEQAN_INCLUDE_PATH=../../seqan-v2.2.0/include/ CXX=clang++ && \
    cp aligner /bin/seqan-aligner
