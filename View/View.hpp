#include <SFML/Graphics.hpp>
#include <queue>
#include "../Interpreter/Interpreter.hpp"

const int CELL_SIZE = 40;
const int WALL_THICKNESS = 2;

const sf::Color WALL_COLOR(40, 40, 40);
const sf::Color PATH_COLOR(220, 220, 220);
const sf::Color ROBOT_COLOR(50, 150, 250);
const sf::Color VISITED_COLOR(180, 200, 180);
const sf::Color START_COLOR(100, 200, 100);
const sf::Color TEXT_COLOR(255, 255, 255);

class View {
    private:
        sf::RenderWindow window;
        std::vector<std::vector<char>> labyrinth;
        std::queue<RobotStep> steps_queue;
        
        int robot_x, robot_y;
        
        sf::Font font;
        
        std::vector<std::vector<bool>> visited;
        
    public:
        View(const std::vector<std::vector<char>>& map, int start_x, int start_y);
        void set_steps_queue(std::queue<RobotStep> queue);
        void draw_labyrinth();
        void draw_robot();
        void run_simulation();
        void execute_next_step();
        void reset_simulation();
        void draw_ui_info(float speed, bool step_mode, bool paused);
};