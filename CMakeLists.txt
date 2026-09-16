cmake_minimum_required(VERSION 3.16)
project(RobotLabyrinth)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

BISON_TARGET(MyParser Parser/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(MyLexer Lexer/lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(MyLexer MyParser)

add_executable(main
    main.cpp
    Interpreter/Interpreter.cpp
    View/View.cpp
    ${BISON_MyParser_OUTPUTS}
    ${FLEX_MyLexer_OUTPUTS}
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(main PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

add_executable(test
    tests/test.cpp
    Interpreter/Interpreter.cpp
    ${BISON_MyParser_OUTPUTS}
    ${FLEX_MyLexer_OUTPUTS}
)

target_include_directories(test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test PRIVATE Catch2::Catch2WithMain)