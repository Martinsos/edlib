FROM alpine

MAINTAINER Martin Sosic <sosic.martin@gmail.com>

WORKDIR ~

# Install needed packages to compile Edlib, then pull it from git repo, compile it,
# move edlib-aligner to bin/ so it can be executed, delete edlib source files and delete all packages
# except for libstdc++ (which is needed for edlib-aligner to run) and bash for convenience, in order to keep docker image small.
RUN apk --no-cache add cmake clang clang-dev make gcc g++ libc-dev linux-headers git libstdc++ bash && \
    git clone -b v1.2.3 https://github.com/Martinsos/edlib.git edlib-git && \
    cd edlib-git && cd build && cmake -D CMAKE_BUILD_TYPE=Release .. && make edlib-aligner && \
    cp bin/edlib-aligner /bin && \
    cd ../.. && rm -r edlib-git && \
    apk del cmake clang clang-dev make gcc g++ libc-dev linux-headers git

CMD ["edlib-aligner"]
