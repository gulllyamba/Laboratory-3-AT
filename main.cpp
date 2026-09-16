#include <iostream>
#include "Interpreter/Interpreter.hpp"
#include <fstream>
#include <format>
#include <SFML/Graphics.hpp>
#include "View/View.hpp"

void print_map(const std::vector<std::vector<char>>& MAP) {
    for (size_t i = 0; i < MAP.size(); ++i) {
        for (size_t j = 0; j < MAP[i].size(); ++j) {
            std::cout << MAP[i][j];
        }
        std::cout << "\n";
    }
}

int main() {
    std::ifstream file("/home/ilya/TA/lab3/exe/robot_ultra.txt");
    std::string test_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::ifstream map_file("/home/ilya/TA/lab3/exe/robot_settings_2.txt");
    std::vector<std::vector<char>> MAP;

    std::string line;
    int x, y;
    map_file >> x >> y;
    while (std::getline(map_file, line)) {
        if (line.empty()) continue;
        std::vector<char> v(line.size());
        for (size_t j = 0 ; j < line.size(); ++j) {
            v[j] = line[j];
        }
        MAP.push_back(v);
    }
    
    Interpreter interpreter(MAP, x, y, test_code);
    try {
        if (interpreter.run_simulation() && interpreter.debug_get_bool(29) == true) {
            std::queue<RobotStep> queue = interpreter.get_steps_queue();
            std::cout << "queue_size: " << queue.size() << "\n";
            View visualizer(MAP, x, y);
            visualizer.set_steps_queue(std::move(queue));
            visualizer.run_simulation();
        }
        else {
            std::cout << "Ошибка во время симуляции или парсинга!" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Выполнение остановлено из-за нарушения ТЗ:" << std::endl;
        std::cerr << e.what() << std::endl;
    }

    // std::ifstream file("/home/ilya/TA/lab3/exe/fib.txt");
    // if (!file.is_open()) {
    //     std::cerr << "Файл не открылся!" << std::endl;
    //     return 1;
    // }
    // std::string test_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // Interpreter interpreter(MAP, 1, 1, test_code);
    
    // std::cout << "n-ое число Фибоначчи:" << std::endl;
    // try {
    //     if (interpreter.run_simulation()) {
    //         std::cout << "Успех! Симуляция завершена." << std::endl;
    //         std::cout << "--- Состояние памяти ---" << std::endl;
    //         std::cout << "n: " << interpreter.debug_get_int(99) << " (ожидаем 20)" << std::endl;
    //         std::cout << "n-ое число: " << interpreter.debug_get_int(2) << " (ожидаем 6765)" << std::endl;
    //     }
    //         else {
    //         std::cout << "Ошибка во время симуляции или парсинга!" << std::endl;
    //     }
    // }
    // catch (const std::exception& e) {
    //     std::cerr << "Выполнение остановлено из-за нарушения ТЗ:" << std::endl;
    //     std::cerr << e.what() << std::endl;
    // }

    // std::ifstream file("/home/ilya/TA/lab3/exe/sum.txt");
    // if (!file.is_open()) {
    //     std::cerr << "Файл не открылся!" << std::endl;
    //     return 1;
    // }
    // std::string test_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // Interpreter interpreter(MAP, 1, 1, test_code);
    
    // std::cout << "Сумма от 1 до n(рекурсивно):" << std::endl;
    // try {
    //     if (interpreter.run_simulation()) {
    //         std::cout << "Успех! Симуляция завершена." << std::endl;
    //         std::cout << "--- Состояние памяти ---" << std::endl;
    //         std::cout << "n: " << interpreter.debug_get_int(5) << " (ожидаем 10)" << std::endl;
    //         std::cout << "сумма от 1 до n: " << interpreter.debug_get_int(2) << " (ожидаем 55)" << std::endl;
    //     }
    //         else {
    //         std::cout << "Ошибка во время симуляции или парсинга!" << std::endl;
    //     }
    // }
    // catch (const std::exception& e) {
    //     std::cerr << "Выполнение остановлено из-за нарушения ТЗ:" << std::endl;
    //     std::cerr << e.what() << std::endl;
    // }
    
    return 0;
}