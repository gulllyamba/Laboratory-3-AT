#include "Interpreter.hpp"
#include <random>
#include <chrono>

static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern std::string parser_error_message;
extern std::shared_ptr<ASTNode> root_node;

std::string Interpreter::make_global_str_id(const std::string& type_prefix, int id) {
    return type_prefix + "_" + std::to_string(id);
}

bool Interpreter::has_path(const std::string& start, const std::string& target, std::map<std::string, bool>& visited) {
    if (start == target) return true;
    visited[start] = true;
    
    if (bindings.find(start) != bindings.end()) {
        for (const auto& next_node : bindings[start]) {
            if (!visited[next_node]) {
                if (has_path(next_node, target, visited)) return true;
            }
        }
    }
    return false;
}

bool Interpreter::run_simulation() {
    parser_error_message = "";
    root_node = nullptr;
    if (!code_to_run.empty() && code_to_run.back() != '\n') code_to_run.push_back('\n');

    YY_BUFFER_STATE buffer = yy_scan_string(code_to_run.c_str());
    int parse_result = yyparse();
    yy_delete_buffer(buffer);

    if (parse_result != 0 || !parser_error_message.empty() || !root_node) return false;
    if (root_node) root_node->execute(*this);
    return true;
}

std::queue<RobotStep> Interpreter::get_steps_queue() const {
    return steps_queue;
}

bool Interpreter::move_robot(const std::string& cmd) {
    int next_x = robot_x;
    int next_y = robot_y;
    if (cmd == "mf") next_y -= 1;
    else if (cmd == "mb") next_y += 1;
    else if (cmd == "mr") next_x += 1;
    else if (cmd == "ml") next_x -= 1;

    bool success = false;
    if (next_y >= 0 && next_y < (int)labyrinth.size() && next_x >= 0 && next_x < (int)labyrinth[next_y].size()) {
        if (labyrinth[next_y][next_x] != '#') {
            robot_x = next_x;
            robot_y = next_y;
            success = true;
            steps_queue.emplace(cmd, robot_x, robot_y, success);
        }
    }
    std::cout << "move_robot: " << success << ", queue_size: " << steps_queue.size() << "\n";
    return success;
}

bool Interpreter::teleport_robot() {
    std::uniform_int_distribution<int> dist_y(0, labyrinth.size() - 1);
    int next_y = dist_y(rng);
    std::uniform_int_distribution<int> dist_x(0, labyrinth[next_y].size() - 1);
    int next_x = dist_x(rng);

    bool success = false;
    if (labyrinth[next_y][next_x] != '#') {
        robot_x = next_x;
        robot_y = next_y;
        success = true;
    }

    steps_queue.emplace("tp", robot_x, robot_y, success);
    return success;
}

void Interpreter::set_int(int id, int value) {
    if (!check_and_register_id(id, VarType::INT_VAR)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id));
    if (int_vars_stack.empty()) int_vars_stack.emplace_back();

    if (int_vars_stack.back().find(id) == int_vars_stack.back().end()) {
        for (int i = int_vars_stack.size() - 2; i >= 0; --i) {
            if (int_vars_stack[i].find(id) != int_vars_stack[i].end()) {
                int_vars_stack[i][id] = value;
                return;
            }
        }
    }
    int_vars_stack.back()[id] = value;
}

int Interpreter::get_int(int id) {
    if (!check_and_register_id(id, VarType::INT_VAR)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id));
    if (int_vars_stack.empty()) int_vars_stack.emplace_back();

    if (int_vars_stack.back().find(id) == int_vars_stack.back().end()) {
        for (int i = int_vars_stack.size() - 2; i >= 0; --i) {
            if (int_vars_stack[i].find(id) != int_vars_stack[i].end()) {
                return int_vars_stack[i][id];
            }
        }
    }
    return int_vars_stack.back()[id];
}

void Interpreter::set_int_array(int id, const std::vector<int>& indices, int value) {
    if (!check_and_register_id(id, VarType::INT_ARRAY)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id) + " с массивом целых чисел.");
    if (int_arrays_stack.empty()) int_arrays_stack.emplace_back();

    auto key = std::make_pair(id, indices);
    if (int_arrays_stack.back().find(key) == int_arrays_stack.back().end()) {
        for (int i = int_arrays_stack.size() - 2; i >= 0; --i) {
            if (int_arrays_stack[i].find(key) != int_arrays_stack[i].end()) {
                int_arrays_stack[i][key] = value;
                return;
            }
        }
    }
    int_arrays_stack.back()[key] = value;
}

int Interpreter::get_int_array(int id, const std::vector<int>& indices) {
    if (!check_and_register_id(id, VarType::INT_ARRAY)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id) + " с массивом целых чисел.");
    if (int_arrays_stack.empty()) int_arrays_stack.emplace_back();

    auto key = std::make_pair(id, indices);
    if (int_arrays_stack.back().find(key) == int_arrays_stack.back().end()) {
        for (int i = int_arrays_stack.size() - 2; i >= 0; --i) {
            if (int_arrays_stack[i].find(key) != int_arrays_stack[i].end()) {
                return int_arrays_stack[i][key];
            }
        }
    }
    return int_arrays_stack.back()[key];
}

void Interpreter::set_bool(int id, bool value) {
    if (!check_and_register_id(id, VarType::BOOL_VAR)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id));
    if (bool_vars_stack.empty()) bool_vars_stack.emplace_back();

    if (bool_vars_stack.back().find(id) == bool_vars_stack.back().end()) {
        for (int i = bool_vars_stack.size() - 2; i >= 0; --i) {
            if (bool_vars_stack[i].find(id) != bool_vars_stack[i].end()) {
                bool_vars_stack[i][id] = value;
                return;
            }
        }
    }
    bool_vars_stack.back()[id] = value;
}

bool Interpreter::get_bool(int id) {
    if (!check_and_register_id(id, VarType::BOOL_VAR)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id));
    if (bool_vars_stack.empty()) bool_vars_stack.emplace_back();

    if (bool_vars_stack.back().find(id) == bool_vars_stack.back().end()) {
        for (int i = bool_vars_stack.size() - 2; i >= 0; --i) {
            if (bool_vars_stack[i].find(id) != bool_vars_stack[i].end()) {
                return bool_vars_stack[i][id];
            }
        }
    }
    return bool_vars_stack.back()[id];
}

void Interpreter::set_bool_array(int id, const std::vector<int>& indices, bool value) {
    if (!check_and_register_id(id, VarType::BOOL_ARRAY)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id) + " с логическим массивом.");
    if (bool_arrays_stack.empty()) bool_arrays_stack.emplace_back();

    auto key = std::make_pair(id, indices);
    if (bool_arrays_stack.back().find(key) == bool_arrays_stack.back().end()) {
        for (int i = bool_arrays_stack.size() - 2; i >= 0; --i) {
            if (bool_arrays_stack[i].find(key) != bool_arrays_stack[i].end()) {
                bool_arrays_stack[i][key] = value;
                return;
            }
        }
    }
    bool_arrays_stack.back()[key] = value;
}

bool Interpreter::get_bool_array(int id, const std::vector<int>& indices) {
    if (!check_and_register_id(id, VarType::BOOL_ARRAY)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id) + " с логическим массивом.");
    if (bool_arrays_stack.empty()) bool_arrays_stack.emplace_back();

    auto key = std::make_pair(id, indices);
    if (bool_arrays_stack.back().find(key) == bool_arrays_stack.back().end()) {
        for (int i = bool_arrays_stack.size() - 2; i >= 0; --i) {
            if (bool_arrays_stack[i].find(key) != bool_arrays_stack[i].end()) {
                return bool_arrays_stack[i][key];
            }
        }
    }
    return bool_arrays_stack.back()[key];
}

void Interpreter::set_proc_array(int id, const std::vector<int>& indices, std::shared_ptr<ASTNode> body) {
    if (!check_and_register_id(id, VarType::PROC_ARRAY)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id) + " с массивом процедур.");
    if (procedure_arrays_stack.empty()) procedure_arrays_stack.emplace_back();

    auto key = std::make_pair(id, indices);
    for (int i = static_cast<int>(procedure_arrays_stack.size()) - 1; i >= 0; --i) {
        auto it = procedure_arrays_stack[i].find(key);
        if (it != procedure_arrays_stack[i].end()) {
            it->second = body;
            return;
        }
    }
    procedure_arrays_stack.back()[key] = body;
}

std::shared_ptr<ASTNode> Interpreter::get_proc_array(int id, const std::vector<int>& indices) {
    if (!check_and_register_id(id, VarType::PROC_ARRAY)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id) + " с массивом процедур.");
    if (procedure_arrays_stack.empty()) procedure_arrays_stack.emplace_back();
    
    auto key = std::make_pair(id, indices);
    for (int i = static_cast<int>(procedure_arrays_stack.size()) - 1; i >= 0; --i) {
        auto it = procedure_arrays_stack[i].find(key);
        if (it != procedure_arrays_stack[i].end()) return it->second;
    }
    procedure_arrays_stack.back()[key] = nullptr;
    return nullptr;
}

void Interpreter::define_procedure(int id, std::shared_ptr<ASTNode> body) {
    if (!check_and_register_id(id, VarType::PROC_VAR)) throw std::runtime_error("Error: пересечение ID " + std::to_string(id));
    if (procedures_stack.empty()) procedures_stack.emplace_back();
    procedures_stack.back()[id] = body;
}

void Interpreter::call_procedure(int id) {
    trigger_bindings("proc", id);
    std::shared_ptr<ASTNode> proc_body = nullptr;
    for (int i = procedures_stack.size() - 1; i >= 0; --i) {
        auto it = procedures_stack[i].find(id);
        if (it != procedures_stack[i].end()) {
            proc_body = it->second;
            break;
        }
    }

    if (proc_body) {
        int_vars_stack.emplace_back();
        bool_vars_stack.emplace_back();

        std::cout << "вызвалась процедура: " << id << ", стек(" << proc_id_stack.size() << ")\n";
        proc_body->execute(*this);

        if (jump_requested) throw std::runtime_error("Error: недопустимый переход (GOTO) за пределы процедуры!");

        int_vars_stack.pop_back();
        bool_vars_stack.pop_back();
    }
}

void Interpreter::call_recurcive_procedure(int id) {
    trigger_bindings("proc", id);
    std::shared_ptr<ASTNode> proc_body = nullptr;
    for (int i = procedures_stack.size() - 1; i >= 0; --i) {
        auto it = procedures_stack[i].find(id);
        if (it != procedures_stack[i].end()) {
            proc_body = it->second;
            break;
        }
    }
    if (proc_body) {
        int_vars_stack.emplace_back();
        bool_vars_stack.emplace_back();

        if (int_vars_stack.size() > 1) {
            int_vars_stack.back() = int_vars_stack[int_vars_stack.size() - 2];
        }
        if (bool_vars_stack.size() > 1) {
            bool_vars_stack.back() = bool_vars_stack[bool_vars_stack.size() - 2];
        }

        std::cout << "вызвалась процедура: " << id << ", стек(" << proc_id_stack.size() << ")\n";
        proc_body->execute(*this);

        if (jump_requested) throw std::runtime_error("Error: недопустимый переход (GOTO) за пределы процедуры!");

        int_vars_stack.pop_back();
        bool_vars_stack.pop_back();
    }
}

void Interpreter::copy_procedure(int to_id, int from_id) {
    std::shared_ptr<ASTNode> from_body = nullptr;
    for (int i = procedures_stack.size() - 1; i >= 0; --i) {
        auto it = procedures_stack[i].find(from_id);
        if (it != procedures_stack[i].end()) {
            from_body = it->second;
            break;
        }
    }
    for (int i = procedures_stack.size() - 1; i >= 0; --i) {
        auto it = procedures_stack[i].find(to_id);
        if (it != procedures_stack[i].end()) {
            it->second = from_body;
            return;
        }
    }
    procedures_stack.back()[to_id] = from_body;
}

bool Interpreter::is_proc_np(int id) {
    for (int i = procedures_stack.size() - 1; i >= 0; --i) {
        auto it = procedures_stack[i].find(id);
        if (it != procedures_stack[i].end()) {
            if (!it->second) return true;
            return std::dynamic_pointer_cast<ASTNoCommand>(it->second) != nullptr;
        }
    }
    return true;
}

bool Interpreter::is_inside_proc() const {
    return inside_procedure;
}

void Interpreter::set_inside_proc(bool val) {
    inside_procedure = val;
}

void Interpreter::push_proc_id(int id) {
    proc_id_stack.push(id);
}

int Interpreter::top_proc_id() {
    return proc_id_stack.top();
}

void Interpreter::pop_proc_id() {
    proc_id_stack.pop();
}

bool Interpreter::proc_id_empty() {
    return proc_id_stack.empty();
}

void Interpreter::request_jump(int label_id) {
    jump_target_label = label_id;
    jump_requested = true;
}

bool Interpreter::is_jump_requested() const {
    return jump_requested;
}

int Interpreter::get_jump_target() const {
    return jump_target_label;
}

void Interpreter::clear_jump() {
    jump_requested = false;
}

bool Interpreter::check_and_register_id(int id, VarType type) {
    auto it = global_id_registry.find(id);
    if (it != global_id_registry.end()) {
        if (it->second != type) throw std::runtime_error(std::format("Error: конфликт идентификаторов! ID {} уже используется другим типом данных.", id));
    }
    else global_id_registry[id] = type;
    return true;
}

std::string Interpreter::get_type_name(VarType type) {
    switch(type) {
        case VarType::INT_VAR: return "Целочисленная переменная";
        case VarType::BOOL_VAR: return "Логическая переменная";
        case VarType::PROC_VAR: return "Переменная-процедура";
        case VarType::INT_ARRAY: return "Целочисленный массив";
        case VarType::BOOL_ARRAY: return "Логический массив";
        case VarType::PROC_ARRAY: return "Массив процедур";
    }
    return "Неизвестный тип";
}

bool Interpreter::bind_identifiers(const std::string& var_type, int var_id, int proc_id) {
    std::string from = make_global_str_id(var_type, var_id);
    std::string to = make_global_str_id("proc", proc_id);

    std::map<std::string, bool> visited;
    if (has_path(to, from, visited)) {
        std::cerr << std::format("Runtime Warning: попытка рекурсивного связывания {} @ {} отклонена!", from, to) << std::endl;
        return false;
    }

    auto& list = bindings[from];
    if (std::find(list.begin(), list.end(), to) == list.end()) {
        list.push_back(to);
    }
    return true;
}

bool Interpreter::unbind_identifiers(const std::string& var_type, int var_id, int proc_id) {
    std::string from = make_global_str_id(var_type, var_id);
    std::string to = make_global_str_id("proc", proc_id);

    if (bindings.find(from) != bindings.end()) {
        auto& list = bindings[from];
        auto it = std::find(list.begin(), list.end(), to);
        if (it != list.end()) list.erase(it);
    }
    return true;
}

void Interpreter::trigger_bindings(const std::string& var_type, int var_id) {
    std::string from = make_global_str_id(var_type, var_id);
    if (bindings.find(from) != bindings.end()) {
        std::vector<std::string> current_bindings = bindings[from];
        for (const auto& proc_str_id : current_bindings) {
            int proc_id = std::stoi(proc_str_id.substr(5));
            bool backup = is_inside_proc();
            set_inside_proc(true);
            call_procedure(proc_id);
            set_inside_proc(backup);
        }
    }
}

int Interpreter::debug_get_int(int id) {
    return int_vars_stack[0][id];
}

bool Interpreter::debug_get_bool(int id) {
    return bool_vars_stack[0][id];
}

int Interpreter::get_robot_x() {
    return robot_x;
}

int Interpreter::get_robot_y() {
    return robot_y;
}