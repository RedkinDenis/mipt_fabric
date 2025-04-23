set(LIB_SOURCE 
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl_like/sdl/sim_display.cpp # SIMULATOR

    ${CMAKE_CURRENT_SOURCE_DIR}/sdl_like/sdl/sdl_like.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pong/pong.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/menu/menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tetris/tetris.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tetris/tetris_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/button/button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/doom-nano/doom.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/doom-nano/entities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/doom-nano/types.cpp
)

set(LIB_INCLUDE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl_like
    ${CMAKE_CURRENT_SOURCE_DIR}/menu
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/pong
    ${CMAKE_CURRENT_SOURCE_DIR}/tetris
    ${CMAKE_CURRENT_SOURCE_DIR}/doom-nano
)

add_library(lib ${LIB_SOURCE})
target_include_directories(lib PUBLIC ${LIB_INCLUDE})

target_link_libraries(lib
    PRIVATE
        sfml-graphics
        sfml-system
        sfml-window
)

target_link_libraries(lib PRIVATE etl::etl)