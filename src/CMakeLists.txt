project(websocketpp)

include_directories(${Boost_INCLUDE_DIRS})

set(websocketpp_SRCS base64/base64.cpp
                     md5/md5.c
                     messages/data.cpp
                     network_utilities.cpp
                     processors/hybi_header.cpp
                     processors/hybi_util.cpp
                     sha1/sha1.cpp
                     uri.cpp )


file(GLOB websocketpp_root_HDRS  *.hpp)
file(GLOB websocketpp_http_HDRS  http/*.hpp)
file(GLOB websocketpp_logger_HDRS  logger/*.hpp)
file(GLOB websocketpp_messages_HDRS  messages/*.hpp)
file(GLOB websocketpp_processors_HDRS  processors/*.hpp)
file(GLOB websocketpp_rng_HDRS  rng/*.hpp)
file(GLOB websocketpp_roles_HDRS  roles/*.hpp)
file(GLOB websocketpp_sockets_HDRS  sockets/*.hpp)
file(GLOB websocketpp_utf8_validator_HDRS  utf8_validator/*.hpp)

set(websocketpp_base64_HDRS base64/base64.h)
set(websocketpp_sha1_HDRS sha1/sha1.h)
set(websocketpp_md5_HDRS md5/md5.h md5/md5.hpp)

set(websocketpp_HDRS base64/base64.h sha1/sha1.h md5/md5.hpp ${websocketpp_HDRS})

add_library(websocketpp STATIC ${websocketpp_SRCS})
set_target_properties(websocketpp PROPERTIES COMPILE_FLAGS -fPIC)
target_link_libraries(websocketpp ${Boost_LIBRARIES})

set_target_properties(websocketpp PROPERTIES VERSION ${WEBSOCKETPP_VERSION} SOVERSION ${WEBSOCKETPP_VERSION_SONAME} )

install(TARGETS websocketpp RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})

install(FILES ${websocketpp_root_HDRS} DESTINATION include/websocketpp COMPONENT Devel)
install(FILES ${websocketpp_http_HDRS} DESTINATION include/websocketpp/http COMPONENT Devel)
install(FILES ${websocketpp_logger_HDRS} DESTINATION include/websocketpp/logger COMPONENT Devel)
install(FILES ${websocketpp_messages_HDRS} DESTINATION include/websocketpp/messages COMPONENT Devel)
install(FILES ${websocketpp_processors_HDRS} DESTINATION include/websocketpp/processors COMPONENT Devel)
install(FILES ${websocketpp_rng_HDRS} DESTINATION include/websocketpp/rng COMPONENT Devel)
install(FILES ${websocketpp_roles_HDRS} DESTINATION include/websocketpp/roles COMPONENT Devel)
install(FILES ${websocketpp_sockets_HDRS} DESTINATION include/websocketpp/sockets COMPONENT Devel)
install(FILES ${websocketpp_utf8_validator_HDRS} DESTINATION include/websocketpp/utf8_validator COMPONENT Devel)
install(FILES ${websocketpp_base64_HDRS} DESTINATION include/websocketpp/base64 COMPONENT Devel)
install(FILES ${websocketpp_sha1_HDRS} DESTINATION include/websocketpp/sha1 COMPONENT Devel)
install(FILES ${websocketpp_md5_HDRS} DESTINATION include/websocketpp/md5 COMPONENT Devel)
