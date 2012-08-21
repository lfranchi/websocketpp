project(websocketpp-qt)
cmake_minimum_required(VERSION 2.8.6)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Qt 4.7.0  REQUIRED COMPONENTS QtCore)
include(${QT_USE_FILE})

# boost
list(APPEND REQUIRED_BOOST_COMPONENTS system regex date_time random)
if(WIN32)
    list(APPEND REQUIRED_BOOST_COMPONENTS thread_win32-mt)
else()
    list(APPEND REQUIRED_BOOST_COMPONENTS thread)
endif()
find_package(Boost REQUIRED COMPONENTS ${REQUIRED_BOOST_COMPONENTS})

set(websocketpp_qt_SRCS WebSocketWrapper.cpp main.cpp)

include_directories(/usr/local/include ${Boost_INCLUDE_DIR} ${QT_INCLUDE_DIR})
set(CMAKE_EXE_LINKER_FLAGS -L/usr/local/lib)

add_executable(websocketpp_qt ${websocketpp_qt_SRCS})
target_link_libraries(websocketpp_qt websocketpp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${QT_LIBRARIES})

set_target_properties(websocketpp_qt PROPERTIES AUTOMOC TRUE)
