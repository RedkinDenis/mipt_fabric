cmake_minimum_required(VERSION 3.10)
project(esp_sim)

add_compile_definitions(SIM)

add_executable(esp_sim  src/main.cpp)
add_subdirectory(lib)
target_link_libraries(esp_sim PRIVATE lib)

include(FetchContent)
#########################################
## SFML
#########################################
set(SFML_VERSION "2.6.0")
FetchContent_Declare(
    sfml
    GIT_REPOSITORY "https://github.com/SFML/SFML.git"
    GIT_TAG        "${SFML_VERSION}"
)

FetchContent_GetProperties(sfml)
if(NOT sfml_POPULATED)
    FetchContent_Populate(sfml)
    add_subdirectory(${sfml_SOURCE_DIR} ${sfml_BINARY_DIR})
endif()

target_link_libraries(esp_sim
    PRIVATE
        sfml-graphics
        sfml-system
        sfml-window
)

#########################################
## ETL
#########################################
set(ETL_VERSION "20.38.11")
FetchContent_Declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl
  GIT_TAG       "${ETL_VERSION}"
)

FetchContent_MakeAvailable(etl)

target_link_libraries(esp_sim PRIVATE etl::etl)