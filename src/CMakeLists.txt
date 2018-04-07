cmake_minimum_required (VERSION 2.8.11)

add_library(pricer-lib
    SHARED
    Option.cpp
    SequentialPricer.cpp
    OpenCLPricer.cpp
    kernel.cl
    Pricer.hpp
    )

install(TARGETS pricer-lib
    DESTINATION lib)

file(
    GLOB
    headers
    Option.hpp
    Pricer.hpp
    )

install(FILES ${headers}
    DESTINATION include/${CMAKE_PROJECT_NAME})

set(EXEC "pricer")

add_executable(${EXEC} main.cpp ${EMBEDDED_OPENCL_KERNELS})

target_link_libraries(${EXEC} PUBLIC ${OpenCL_LIBRARY} pricer-lib ${LIBS})

add_custom_command(TARGET ${EXEC} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                       ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cl
                       $<TARGET_FILE_DIR:${EXEC}>
                   )
