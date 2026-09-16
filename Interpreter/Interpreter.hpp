#pragma once

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <stack>
#include "RobotStep.hpp"
#include <iostream>

enum class VarType {
    INT_VAR,
    BOOL_VAR,
    PROC_VAR,
    INT_ARRAY,
    BOOL_ARRAY,
    PROC_ARRAY
};

class Interpreter;

class ASTNode {
    public:
        virtual ~ASTNode() = default;
        virtual void execute(class Interpreter& interpreter) = 0;
};

template <typename T>
class ASTExpression {
    public:
        virtual ~ASTExpression() = default;
        virtual T evaluate(Interpreter& interpreter) = 0;
};

class Interpreter {
    private:
        friend class ASTBlock;

        std::string code_to_run;

        std::vector<std::vector<char>> labyrinth;
        int robot_x, robot_y;

        std::queue<RobotStep> steps_queue;

        std::vector<std::map<int, int>> int_vars_stack;
        std::vector<std::map<int, bool>> bool_vars_stack;
        std::vector<std::map<int, std::shared_ptr<ASTNode>>> procedures_stack;
        std::stack<int> proc_id_stack;

        std::map<int, VarType> global_id_registry;

        std::vector<std::map<std::pair<int, std::vector<int>>, int>> int_arrays_stack;
        std::vector<std::map<std::pair<int, std::vector<int>>, bool>> bool_arrays_stack;
        std::vector<std::map<std::pair<int, std::vector<int>>, std::shared_ptr<ASTNode>>> procedure_arrays_stack;

        bool inside_procedure = false;

        bool jump_requested = false;
        int jump_target_label = -1;

        std::map<std::string, std::vector<std::string>> bindings;

        std::string make_global_str_id(const std::string& type_prefix, int id);
        bool has_path(const std::string& start, const std::string& target, std::map<std::string, bool>& visited);

    public:
        Interpreter(const std::vector<std::vector<char>>& map, int start_x, int start_y, const std::string& code) : labyrinth(map), robot_x(start_x), robot_y(start_y), code_to_run(code) {
            int_vars_stack.emplace_back(); 
            bool_vars_stack.emplace_back();
            procedures_stack.emplace_back();
            int_arrays_stack.emplace_back();
            bool_arrays_stack.emplace_back();
            procedure_arrays_stack.emplace_back();
        }

        bool run_simulation();
        std::queue<RobotStep> get_steps_queue() const;

        bool move_robot(const std::string& cmd);
        bool teleport_robot();

        void set_int(int id, int value);
        int get_int(int id);

        void set_int_array(int id, const std::vector<int>& indices, int value);
        int get_int_array(int id, const std::vector<int>& indices);

        void set_bool(int id, bool value);
        bool get_bool(int id);

        void set_bool_array(int id, const std::vector<int>& indices, bool value);
        bool get_bool_array(int id, const std::vector<int>& indices);

        void set_proc_array(int id, const std::vector<int>& indices, std::shared_ptr<ASTNode> body);
        std::shared_ptr<ASTNode> get_proc_array(int id, const std::vector<int>& indices);

        void define_procedure(int id, std::shared_ptr<ASTNode> body);
        void call_procedure(int id);
        void call_recurcive_procedure(int id);
        void copy_procedure(int to_id, int from_id);
        bool is_proc_np(int id);
        
        bool is_inside_proc() const;
        void set_inside_proc(bool val);

        void push_proc_id(int id);
        int top_proc_id();
        void pop_proc_id();
        bool proc_id_empty();

        void request_jump(int label_id);
        bool is_jump_requested() const;
        int get_jump_target() const;
        void clear_jump();

        bool check_and_register_id(int id, VarType type);
        std::string get_type_name(VarType type);

        bool bind_identifiers(const std::string& var_type, int var_id, int proc_id);
        bool unbind_identifiers(const std::string& var_type, int var_id, int proc_id);
        void trigger_bindings(const std::string& var_type, int var_id);

        ///////////////////////////////
        int debug_get_int(int id);
        bool debug_get_bool(int id);
        int get_robot_x();
        int get_robot_y();
};

class ASTExpressionStatement : public ASTNode {
    private:
        std::shared_ptr<ASTExpression<bool>> expr_;
    public:
        ASTExpressionStatement(ASTExpression<bool>* expr) : expr_(expr) {}
        void execute(Interpreter& interpreter) override {
            expr_->evaluate(interpreter);
        }
};

class ASTIntLiteral : public ASTExpression<int> {
    private:
        int value_;
    public:
        ASTIntLiteral(int value) : value_(value) {}
        int evaluate(Interpreter& interpreter) override {
            return value_;
        }
};

class ASTIntVar : public ASTExpression<int> {
    private:
        int id_;
    public:
        ASTIntVar(int id) : id_(id) {}
        int evaluate(Interpreter& interpreter) override {
            interpreter.trigger_bindings("int", id_);
            return interpreter.get_int(id_);
        }
};

class ASTBoolLiteral : public ASTExpression<bool> {
    private:
        bool value_;
    public:
        ASTBoolLiteral(bool value) : value_(value) {}
        bool evaluate(Interpreter& interpreter) override {
            return value_;
        }
};

class ASTBoolVar : public ASTExpression<bool> {
    private:
        int id_;
    public:
        ASTBoolVar(int id) : id_(id) {}
        bool evaluate(Interpreter& interpreter) override {
            interpreter.trigger_bindings("bool", id_);
            return interpreter.get_bool(id_);
        }
};

class ASTPierceArrow : public ASTExpression<bool> {
    private:
        std::shared_ptr<ASTExpression<bool>> left_;
        std::shared_ptr<ASTExpression<bool>> right_;
    public:
        ASTPierceArrow(ASTExpression<bool>* left, ASTExpression<bool>* right) : left_(left), right_(right) {}
        bool evaluate(Interpreter& interpreter) override {
            return !(left_->evaluate(interpreter) || right_->evaluate(interpreter));
        }
};

class ASTIntEqExpression : public ASTExpression<bool> {
    private:
        std::shared_ptr<ASTExpression<int>> left_;
        std::shared_ptr<ASTExpression<int>> right_;
    public:
        ASTIntEqExpression(ASTExpression<int>* left, ASTExpression<int>* right) : left_(left), right_(right) {}
        bool evaluate(Interpreter& interpreter) override {
            return left_->evaluate(interpreter) == right_->evaluate(interpreter);
        }
};

class ASTBoolEqExpression : public ASTExpression<bool> {
    private:
        std::shared_ptr<ASTExpression<bool>> left_;
        std::shared_ptr<ASTExpression<bool>> right_;
    public:
        ASTBoolEqExpression(ASTExpression<bool>* left, ASTExpression<bool>* right) : left_(left), right_(right) {}
        bool evaluate(Interpreter& interpreter) override {
            return left_->evaluate(interpreter) == right_->evaluate(interpreter);
        }
};

class ASTProcEqNpExpression : public ASTExpression<bool> {
    private:
        int proc_id_;
    public:
        ASTProcEqNpExpression(int proc_id) : proc_id_(proc_id) {}
        bool evaluate(Interpreter& interpreter) override {
            return interpreter.is_proc_np(proc_id_);
        }
};

class ASTWhileLoop : public ASTNode {
    private:
        std::shared_ptr<ASTExpression<bool>> condition_;
        std::shared_ptr<ASTNode> body_;
    public:
        ASTWhileLoop(ASTExpression<bool>* condition, ASTNode* body) : condition_(condition), body_(body) {}
        void execute(Interpreter& interpreter) override {
            while (condition_->evaluate(interpreter)) {
                if (body_) body_->execute(interpreter);
                if (interpreter.is_jump_requested()) return;
            }
        }
};

class ASTRobotCommand : public ASTExpression<bool> {
    private:
        std::string command_;
    public:
        ASTRobotCommand(const std::string& cmd) : command_(cmd) {}
        bool evaluate(Interpreter& interpreter) override {
            return interpreter.move_robot(command_);
        }
};

class ASTTeleportCommand : public ASTExpression<bool> {
    public:
        bool evaluate(Interpreter& interpreter) override {
            return interpreter.teleport_robot();
        }
};

class ASTNoCommand : public ASTNode {
    public:
        void execute(Interpreter& interpreter) override {}
};

class ASTIntAssign : public ASTNode {
    private:
        int id_;
        std::shared_ptr<ASTExpression<int>> expr_;
    public:
        ASTIntAssign(int id, ASTExpression<int>* expr) : id_(id), expr_(expr) {}
        void execute(Interpreter& interpreter) override {
            interpreter.set_int(id_, expr_->evaluate(interpreter));
            std::cout << "(" << id_ << ") <-" << interpreter.get_int(id_) << "\n";
        }
};

class ASTBoolAssign : public ASTNode {
    private:
        int id_;
        std::shared_ptr<ASTExpression<bool>> expr_;
    public:
        ASTBoolAssign(int id, ASTExpression<bool>* expr) : id_(id), expr_(expr) {}
        void execute(Interpreter& interpreter) override {
            interpreter.set_bool(id_, expr_->evaluate(interpreter));
        }
};

class ASTIntIncrement : public ASTNode {
    private:
        int id_;
    public:
        ASTIntIncrement(int id) : id_(id) {}
        void execute(Interpreter& interpreter) override {
            interpreter.set_int(id_, interpreter.get_int(id_) + 1);
        }
};

class ASTIntDecrement : public ASTNode {
    private:
        int id_;
    public:
        ASTIntDecrement(int id) : id_(id) {}
        void execute(Interpreter& interpreter) override {
            interpreter.set_int(id_, interpreter.get_int(id_) - 1);
        }
};

class ASTProcDefine : public ASTNode {
    private:
        int id_;
        std::shared_ptr<ASTNode> body_;
    public:
        ASTProcDefine(int id, ASTNode* body) : id_(id), body_(body) {}
        void execute(Interpreter& interpreter) override {
            interpreter.define_procedure(id_, body_);
        }
};

class ASTProcCopy : public ASTNode {
    private:
        int to_id_;
        int from_id_;
    public:
        ASTProcCopy(int to_id, int from_id) : to_id_(to_id), from_id_(from_id) {}
        void execute(Interpreter& interpreter) override {
            interpreter.copy_procedure(to_id_, from_id_);
        }
};

class ASTProcCall : public ASTNode {
    private:
        int id_;
    public:
        ASTProcCall(int id) : id_(id) {}
        void execute(Interpreter& interpreter) override {
            if (interpreter.proc_id_empty()) {
                interpreter.push_proc_id(id_);
                interpreter.call_procedure(id_);
                interpreter.pop_proc_id();
            }
            else {
                if (id_ != interpreter.top_proc_id()) {
                    interpreter.push_proc_id(id_);
                    interpreter.call_procedure(id_);
                    interpreter.pop_proc_id();
                }
                else {
                    interpreter.push_proc_id(id_);
                    interpreter.call_recurcive_procedure(id_);
                    interpreter.pop_proc_id();
                }
            }
        }
};

class ASTLabel : public ASTNode {
    private:
        int label_id_;
    public:
        ASTLabel(int label_id) : label_id_(label_id) {}
        int get_id() const {
            return label_id_;
        }
        void execute(Interpreter& interpreter) override {}
};

class ASTBlock : public ASTNode {
    private:
        std::vector<std::shared_ptr<ASTNode>> statements_;
        std::map<int, int> local_labels_;
    public:
        void add_statement(ASTNode* statement) {
            if (statement) statements_.push_back(std::shared_ptr<ASTNode>(statement));
        }
        void execute(Interpreter& interpreter) override {
            local_labels_.clear();
            for (size_t i = 0; i < statements_.size(); ++i) {
                if (auto label_node = std::dynamic_pointer_cast<ASTLabel>(statements_[i])) {
                    local_labels_[label_node->get_id()] = i;
                }
            }

            int pc = 0;
            while (pc < statements_.size()) {
                statements_[pc]->execute(interpreter);
                if (interpreter.is_jump_requested()) {
                    int target = interpreter.get_jump_target();
                    if (local_labels_.find(target) != local_labels_.end()) {
                        pc = local_labels_[target];
                        interpreter.clear_jump();
                    }
                    else return;
                }
                else pc++;
            }
        }
};

class ASTGotoStatement : public ASTNode {
    private:
        std::shared_ptr<ASTExpression<bool>> condition_;
        int target_label_;
        bool is_please_;
    public:
        ASTGotoStatement(ASTExpression<bool>* cond, int label, bool please = false) : condition_(cond), target_label_(label), is_please_(please) {}

        void execute(Interpreter& interpreter) override {
            if (condition_->evaluate(interpreter)) {
                interpreter.request_jump(target_label_);
                std::cout << "переход по метке: " << target_label_ << "\n";
            }
        }
};

class ASTIntArrayAccess : public ASTExpression<int> {
    private:
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
    public:
        ASTIntArrayAccess(int id, const std::vector<ASTExpression<int>*>& exprs) : array_id_(id) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }  
        
        int evaluate(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);
            return interpreter.get_int_array(array_id_, resolved_indices);
        }
};

class ASTIntArrayAssign : public ASTNode {
    private:
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
        std::shared_ptr<ASTExpression<int>> value_expr_;
    public:
        ASTIntArrayAssign(int id, const std::vector<ASTExpression<int>*>& exprs, ASTExpression<int>* value) : array_id_(id), value_expr_(value) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }
            
        void execute(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);
            interpreter.set_int_array(array_id_, resolved_indices, value_expr_->evaluate(interpreter));
        }
};

class ASTBoolArrayAccess : public ASTExpression<bool> {
    private:
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
    public:
        ASTBoolArrayAccess(int id, const std::vector<ASTExpression<int>*>& exprs) : array_id_(id) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }
            
        bool evaluate(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);
            return interpreter.get_bool_array(array_id_, resolved_indices);
        }
};

class ASTBoolArrayAssign : public ASTNode {
    private:
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
        std::shared_ptr<ASTExpression<bool>> value_expr_;
    public:
        ASTBoolArrayAssign(int id, const std::vector<ASTExpression<int>*>& exprs, ASTExpression<bool>* value) : array_id_(id), value_expr_(value) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }
            
        void execute(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);
            interpreter.set_bool_array(array_id_, resolved_indices, value_expr_->evaluate(interpreter));
        }
};

class ASTProcArrayCall : public ASTNode {
    private:
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
    public:
        ASTProcArrayCall(int id, const std::vector<ASTExpression<int>*>& exprs) : array_id_(id) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }
            
        void execute(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);
            auto body = interpreter.get_proc_array(array_id_, resolved_indices);
            if (body) body->execute(interpreter);
        }
};

class ASTProcArrayDefine : public ASTNode {
    private:
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
        std::shared_ptr<ASTNode> body_;
    public:
        ASTProcArrayDefine(int id, const std::vector<ASTExpression<int>*>& exprs, ASTNode* body) : array_id_(id), body_(body) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }

        void execute(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);
            interpreter.set_proc_array(array_id_, resolved_indices, body_);
        }
};

class ASTProcArrayAssign : public ASTNode {
    private:
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
        int from_proc_id_;
    public:
        ASTProcArrayAssign(int id, const std::vector<ASTExpression<int>*>& exprs, int src_proc_id) : array_id_(id), from_proc_id_(src_proc_id) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }

        void execute(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);
            auto call_node = std::make_shared<ASTProcCall>(from_proc_id_);
            interpreter.set_proc_array(array_id_, resolved_indices, call_node);
        }
};

class ASTProcAssign : public ASTNode { // запись в proc <- ячейки массива процедур
    private:
        int to_proc_id_;
        int array_id_;
        std::vector<std::shared_ptr<ASTExpression<int>>> index_exprs_;
    public:
        ASTProcAssign(int to_proc_id, int array_id, const std::vector<ASTExpression<int>*>& exprs) : to_proc_id_(to_proc_id), array_id_(array_id) {
            for(auto expr : exprs) {
                index_exprs_.push_back(std::shared_ptr<ASTExpression<int>>(expr));
            }
        }

        void execute(Interpreter& interpreter) override {
            std::vector<int> resolved_indices;
            for (auto& expr : index_exprs_) {
                resolved_indices.push_back(expr->evaluate(interpreter));
            }
            if (resolved_indices.empty()) resolved_indices.push_back(0);

            auto body = interpreter.get_proc_array(array_id_, resolved_indices);
            if (body) interpreter.define_procedure(to_proc_id_, body);
            else interpreter.define_procedure(to_proc_id_, std::make_shared<ASTNoCommand>());
        }
};

class ASTBindOperator : public ASTExpression<bool> {
    private:
        std::string var_type_;
        int var_id_;
        int proc_id_;
    public:
        ASTBindOperator(const std::string& var_type, int var_id, int proc_id) : var_type_(var_type), var_id_(var_id), proc_id_(proc_id) {}

        bool evaluate(Interpreter& interpreter) override {
            return interpreter.bind_identifiers(var_type_, var_id_, proc_id_);
        }
};

class ASTUnbindOperator : public ASTExpression<bool> {
    private:
        std::string var_type_;
        int var_id_;
        int proc_id_;
    public:
        ASTUnbindOperator(const std::string& var_type, int var_id, int proc_id) : var_type_(var_type), var_id_(var_id), proc_id_(proc_id) {}

        bool evaluate(Interpreter& interpreter) override {
            return interpreter.unbind_identifiers(var_type_, var_id_, proc_id_);
        }
};

class ASTNotExpression : public ASTExpression<bool> {
    private:
        std::shared_ptr<ASTExpression<bool>> expr_;
    public:
        ASTNotExpression(ASTExpression<bool>* expr) : expr_(expr) {}
        bool evaluate(Interpreter& interpreter) override {
            return !expr_->evaluate(interpreter);
        }
};