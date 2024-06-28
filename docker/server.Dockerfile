FROM gcc

WORKDIR /appsrc

COPY ./src src
COPY ./libs libs
COPY ./scripts scripts
COPY ./Makefile Makefile

RUN apt update
RUN apt upgrade -y
RUN bash scripts/installdeps-ubuntu-server.sh
RUN make server -j2

WORKDIR /app

RUN cp /appsrc/build/debug/server/pongserver .

CMD ["/app/pongserver", "-p", "51009"]
