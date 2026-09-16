#include "View.hpp"

View::View(const std::vector<std::vector<char>>& map, int start_x, int start_y) : labyrinth(map), robot_x(start_x), robot_y(start_y) {
    visited.resize(map.size(), std::vector<bool>(map[0].size(), false));
    visited[start_y][start_x] = true;
    
    int window_width = map[0].size() * CELL_SIZE;
    int window_height = map.size() * CELL_SIZE;
    
    window.create(sf::VideoMode(window_width, window_height), "Labyrinth Robot Simulator");
    
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "Error loading font" << std::endl;
    }
}

void View::set_steps_queue(std::queue<RobotStep> queue) {
    steps_queue = std::move(queue);
}

void View::draw_labyrinth() {
    for (size_t y = 0; y < labyrinth.size(); ++y) {
        for (size_t x = 0; x < labyrinth[y].size(); ++x) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
            
            if (labyrinth[y][x] == '#') cell.setFillColor(WALL_COLOR);
            else if (visited[y][x]) cell.setFillColor(VISITED_COLOR);
            else cell.setFillColor(PATH_COLOR);
            
            cell.setOutlineThickness(WALL_THICKNESS);
            cell.setOutlineColor(sf::Color(100, 100, 100));
            
            window.draw(cell);
        }
    }
}

void View::draw_robot() {
    float center_x = robot_x * CELL_SIZE + CELL_SIZE / 2.0f;
    float center_y = robot_y * CELL_SIZE + CELL_SIZE / 2.0f;
    float radius = CELL_SIZE / 3.0f;
    
    sf::CircleShape body(radius);
    body.setFillColor(ROBOT_COLOR);
    body.setOrigin(radius, radius);
    body.setPosition(center_x, center_y);
    window.draw(body);
    
    sf::ConvexShape direction;
    direction.setPointCount(3);

    int robot_dir = 1;
    
    float dir_x = 0, dir_y = 0;
    switch(robot_dir) {
        case 0: {
            dir_y = -1;
            break;
        }
        case 1: {
            dir_x = 1;
            break;
        }
        case 2: {
            dir_y = 1;
            break;
        }
        case 3: {
            dir_x = -1;
            break;
        }
    }
    
    sf::Vector2f tip(center_x + dir_x * radius * 1.5f, center_y + dir_y * radius * 1.5f);
    sf::Vector2f left(center_x + (-dir_y * radius * 0.5f) - dir_x * radius * 0.5f, center_y + (dir_x * radius * 0.5f) - dir_y * radius * 0.5f);
    sf::Vector2f right(center_x + (dir_y * radius * 0.5f) - dir_x * radius * 0.5f, center_y + (-dir_x * radius * 0.5f) - dir_y * radius * 0.5f);
    
    direction.setPoint(0, tip);
    direction.setPoint(1, left);
    direction.setPoint(2, right);
    direction.setFillColor(sf::Color::White);
    
    window.draw(direction);
    
    if (font.getInfo().family != "") {
        sf::Text coords;
        coords.setFont(font);
        coords.setString("(" + std::to_string(robot_x) + "," + std::to_string(robot_y) + ")");
        coords.setCharacterSize(12);
        coords.setFillColor(TEXT_COLOR);
        coords.setPosition(robot_x * CELL_SIZE + 2, robot_y * CELL_SIZE + 2);
        window.draw(coords);
    }
}

void View::run_simulation() {
    bool paused = false;
    bool step_by_step = true;
    float animation_speed = 0.5f;
    
    sf::Clock clock;
    float time_since_last_step = 0;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                switch(event.key.code) {
                    case sf::Keyboard::Space: {
                        paused = !paused;
                        break;
                    }
                    case sf::Keyboard::Right: {
                        if (step_by_step && !steps_queue.empty()) execute_next_step();
                        break;
                    }
                    case sf::Keyboard::S: {
                        step_by_step = !step_by_step;
                        break;
                    }
                    case sf::Keyboard::Up: {
                        animation_speed = std::max(0.1f, animation_speed - 0.1f);
                        break;
                    }
                    case sf::Keyboard::Down: {
                        animation_speed = std::min(2.0f, animation_speed + 0.1f);
                        break;
                    }
                    case sf::Keyboard::R:
                        reset_simulation();
                        break;
                }
            }
        }
        
        if (!paused && !step_by_step && !steps_queue.empty()) {
            time_since_last_step += clock.restart().asSeconds();
            
            while (time_since_last_step >= animation_speed && !steps_queue.empty()) {
                execute_next_step();
                time_since_last_step -= animation_speed;
            }
        }
        
        window.clear(sf::Color(30, 30, 30));
        
        draw_labyrinth();
        draw_robot();
        draw_ui_info(animation_speed, step_by_step, paused);
        
        window.display();
    }
}

void View::execute_next_step() {
    if (steps_queue.empty()) return;
    
    RobotStep step = steps_queue.front();
    steps_queue.pop();
    
    if (step.result_) {
        robot_x = step.x_;
        robot_y = step.y_;
        // robot_dir = step.direction_;
        visited[robot_y][robot_x] = true;
    }
}

void View::reset_simulation() {}

void View::draw_ui_info(float speed, bool step_mode, bool paused) {
    if (font.getInfo().family == "") return;
    
    sf::Text info;
    info.setFont(font);
    info.setCharacterSize(16);
    info.setFillColor(TEXT_COLOR);
    
    std::string status = "Steps left: " + std::to_string(steps_queue.size());
    status += " | Speed: " + std::to_string(speed).substr(0, 3) + "s";
    status += " | Mode: " + std::string(step_mode ? "Step-by-step" : "Auto");
    status += " | " + std::string(paused ? "PAUSED" : "RUNNING");
    
    info.setString(status);
    info.setPosition(10, labyrinth.size() * CELL_SIZE - 25);
    
    sf::RectangleShape bg(sf::Vector2f(info.getLocalBounds().width + 20, 25));
    bg.setFillColor(sf::Color(0, 0, 0, 180));
    bg.setPosition(5, labyrinth.size() * CELL_SIZE - 25);
    
    window.draw(bg);
    window.draw(info);
}