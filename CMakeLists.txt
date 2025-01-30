cmake_minimum_required(VERSION 3.10)
project(RuLRS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Определяем TARGET_NATIVE для всей сборки
add_definitions(-DTARGET_NATIVE)

# Добавляем пути к исходникам
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/lib
)

# Собираем список исходных файлов
file(GLOB_RECURSE SOURCES 
    "src/lib/*.cpp"
    "src/src/*.cpp"
)

# Создаем исполняемый файл
add_executable(rulrs_native ${SOURCES}) 
