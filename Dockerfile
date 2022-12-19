FROM weckyy702/raychel_ci:latest

#Copy source files and set the working director

COPY . /usr/src/Raychel
WORKDIR /usr/src/Raychel

ARG COMPILER

RUN cmake -DCMAKE_CXX_COMPILER=${COMPILER} -DRAYCHEL_BUILD_TESTS=ON .
RUN cmake --build . --target all test

LABEL Name=raychel Version=0.0.1
