#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../Interpreter/Interpreter.hpp"
#include <fstream>

TEST_CASE("Language") {
    std::ifstream file("/home/ilya/TA/lab3/tests/test.txt");
    std::string test_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::ifstream map_file("/home/ilya/TA/lab3/tests/map_test.txt");
    std::vector<std::vector<char>> MAP;

    std::string line;
    while (std::getline(map_file, line)) {
        std::vector<char> v(line.size() - 1);
        for (size_t j = 0 ; j < line.size() - 1; ++j) {
            v[j] = line[j];
        }
        MAP.push_back(v);
    }
    
    Interpreter interpreter(MAP, 1, 1, test_code);
    bool success = interpreter.run_simulation();
    REQUIRE(success == true);

    SECTION("Variables") {
        REQUIRE(interpreter.debug_get_int(1) == -1);
        REQUIRE(interpreter.debug_get_int(2) == 1);

        REQUIRE(interpreter.debug_get_bool(21) == true);
        REQUIRE(interpreter.debug_get_bool(22) == false);
    }
    SECTION("Arrays") {
        REQUIRE(interpreter.debug_get_int(4) == 1);
        REQUIRE(interpreter.debug_get_int(5) == 0);
        REQUIRE(interpreter.debug_get_int(6) == 1);

        REQUIRE(interpreter.debug_get_bool(24) == true);
        REQUIRE(interpreter.debug_get_bool(25) == false);
        REQUIRE(interpreter.debug_get_bool(26) == true);

        REQUIRE(interpreter.debug_get_int(7) == 1);
    }
    SECTION("Ariphmetic operations") {
        REQUIRE(interpreter.debug_get_int(8) == 2);
        REQUIRE(interpreter.debug_get_int(9) == 1);
    }
    SECTION("Logic operations") {
        REQUIRE(interpreter.debug_get_bool(27) == false);
        REQUIRE(interpreter.debug_get_bool(28) == true);
        REQUIRE(interpreter.debug_get_bool(29) == false);
    }
    SECTION("Equal operation") {
        REQUIRE(interpreter.debug_get_bool(30) == true);
        REQUIRE(interpreter.debug_get_bool(31) == false);

        REQUIRE(interpreter.debug_get_bool(32) == true);
        REQUIRE(interpreter.debug_get_bool(33) == false);

        REQUIRE(interpreter.debug_get_bool(34) == true);
        REQUIRE(interpreter.debug_get_bool(35) == false);
    }
    SECTION("While loop") {
        REQUIRE(interpreter.debug_get_int(10) == 3);
    }
    SECTION("Jump") {
        REQUIRE(interpreter.debug_get_int(11) == 0);
    }
    SECTION ("Bind operator") {
        REQUIRE(interpreter.debug_get_int(12) == -2);
        REQUIRE(interpreter.debug_get_int(13) == -1);
        REQUIRE(interpreter.debug_get_int(14) == 2);
        REQUIRE(interpreter.debug_get_int(15) == 1);
    }
    SECTION("Unbind operator") {
        REQUIRE(interpreter.debug_get_int(16) == -1);
        REQUIRE(interpreter.debug_get_bool(36) == true);
        REQUIRE(interpreter.debug_get_bool(37) == false);
        REQUIRE(interpreter.debug_get_bool(38) == true);
    }
    SECTION("Move operators") {
        REQUIRE(interpreter.get_robot_x() == 1);
        REQUIRE(interpreter.get_robot_y() == 6);

        REQUIRE(interpreter.debug_get_bool(100) == false);
        REQUIRE(interpreter.debug_get_bool(101) == false);
        REQUIRE(interpreter.debug_get_bool(102) == false);
    }
}