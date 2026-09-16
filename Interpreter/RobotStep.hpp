#pragma once
#include <string>

struct RobotStep {
    std::string command_;
    int x_, y_;
    bool result_;

    RobotStep(const std::string& cmd, int x, int y, bool res) : command_(cmd), x_(x), y_(y), result_(res) {}
};