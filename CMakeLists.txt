cmake_minimum_required(VERSION 3.28)
project(DCMGATE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(DCMTK REQUIRED CONFIG)

include_directories(${DCMTK_INCLUDE_DIRS})
link_directories(${DCMTK_LIBRARY_DIRS})

add_executable(DCMGATE  src/main.cpp
                        src/DICOMClient.cpp)

target_link_libraries(DCMGATE dcmect dcmpmap dcmtract dcmseg dcmfg dcmiod dcmrt dcmpstat dcmqrdb dcmwlm dcmdsig cmr dcmsr dcmnet dcmtls dcmtkcharls dcmjpls ijg16 ijg12 ijg8 dcmjpeg dcmimage dcmimgle dcmxml i2d dcmdata oflog ofstd oficonv z)

add_executable(DCMTKTEST  src/DCMTKTEST/main.cpp)

target_link_libraries(DCMTKTEST dcmect dcmpmap dcmtract dcmseg dcmfg dcmiod dcmrt dcmpstat dcmqrdb dcmwlm dcmdsig cmr dcmsr dcmnet dcmtls dcmtkcharls dcmjpls ijg16 ijg12 ijg8 dcmjpeg dcmimage dcmimgle dcmxml i2d dcmdata oflog ofstd oficonv z)