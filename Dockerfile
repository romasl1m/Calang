FROM ubuntu:24.04

RUN apt update && apt install -y \
    g++ \
    cmake \
    make \
    libcurl4-openssl-dev

WORKDIR /app

COPY . .

RUN mkdir build2 && cd build2 && \
    cmake .. && \
    make

EXPOSE 8080

CMD ["./build2/calang"]
