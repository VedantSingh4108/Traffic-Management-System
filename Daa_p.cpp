#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

using namespace std;

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}



        int x, y;
        int queue;
        int green_duration;
        int green_timer;
        bool is_green;
        int wait_time;
        int max_wait;
    };

    struct Car {
        int from, to;
        int progress;
        char direction;
    };

    unordered_map<int, Intersection> intersections;
    vector<Car> cars;
    int grid_size = 20;
    int simulation_speed = 3;
    int time_since_last_switch = 0;
    const int MIN_GREEN_TIME = 5;
    const int MAX_WAIT_THRESHOLD = 15;

    void delay(int milliseconds) {
        clock_t start_time = clock();
        while (clock() < start_time + milliseconds * CLOCKS_PER_SEC / 1000) {
            if (_kbhit()) return;
        }
    }

    void drawRoad(int x1, int y1, int x2, int y2, vector<string>& grid) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int steps = max(abs(dx), abs(dy));

        for (int i = 0; i <= steps; i++) {
            int x = x1 + (dx * i) / steps;
            int y = y1 + (dy * i) / steps;
            if (x >= 0 && x < grid_size && y >= 0 && y < grid_size) {
                if (dx != 0 && dy != 0) {
                    grid[y][x] = (dx*dy > 0) ? '\\' : '/';
                }
                else if (dx != 0) {
                    if (grid[y][x] == ' ') grid[y][x] = '-';
                    else if (grid[y][x] == '|') grid[y][x] = '+';
                }
                else if (dy != 0) {
                    if (grid[y][x] == ' ') grid[y][x] = '|';
                    else if (grid[y][x] == '-') grid[y][x] = '+';
                }
            }
        }
    }

    void updateWaitTimes() {
        for (auto& pair : intersections) {
            auto& inter = pair.second;
            if (!inter.is_green) {
                inter.wait_time++;
                if (inter.wait_time > inter.max_wait) {
                    inter.max_wait = inter.wait_time;
                }
            } else {
                inter.wait_time = 0;
            }
        }
    }

    int selectIntersectionToGreen() {
        for (auto& pair : intersections) {
            if (pair.second.wait_time >= MAX_WAIT_THRESHOLD) {
                return pair.first;
            }
        }
        int best_id = 1;
        float best_score = -1;

        for (auto& pair : intersections) {
            const auto& inter = pair.second;
            float score = inter.queue + (inter.wait_time * 0.5f);
            if (score > best_score) {
                best_score = score;
                best_id = pair.first;
            }
        }

        return best_id;
    }

public:
    void addIntersection(int id, int x, int y) {
        intersections[id] = {x, y, 0, MIN_GREEN_TIME, 0, false, 0, 0};
    }

    void addRoad(int from, int to) {
        if (rand() % 3 == 0) {
            char dir;
            auto from_inter = intersections[from];
            auto to_inter = intersections[to];
            
            if (to_inter.x > from_inter.x) dir = '>';
            else if (to_inter.x < from_inter.x) dir = '<';
            else if (to_inter.y < from_inter.y) dir = '^';
            else dir = 'v';
            
            cars.push_back({from, to, 0, dir});
            intersections[from].queue++;
        }
    }

    void update() {
        updateWaitTimes();
        bool needs_switch = true;
        for (auto& pair : intersections) {
            if (pair.second.is_green && pair.second.green_timer > 0) {
                needs_switch = false;
                break;
            }
        }

        if (needs_switch) {
            int next_green = selectIntersectionToGreen();
            for (auto& pair : intersections) {
                pair.second.is_green = false;
                pair.second.green_timer = 0;
            }
            intersections[next_green].is_green = true;
            intersections[next_green].green_timer = intersections[next_green].green_duration;
            intersections[next_green].wait_time = 0;
        }
        for (auto& pair : intersections) {
            if (pair.second.is_green && pair.second.green_timer > 0) {
                pair.second.green_timer--;
            }
        }
        for (auto& car : cars) {
            if (intersections[car.from].is_green) {
                car.progress += simulation_speed;
                if (car.progress >= 100) {
                    intersections[car.to].queue++;
                    car = cars.back();
                    cars.pop_back();
                }
            }
        }
        if (rand() % 3 == 0) {
            int from = rand() % intersections.size() + 1;
            int to = (from % intersections.size()) + 1;
            addRoad(from, to);
        }
    }
    void draw() {
        system("cls");
        vector<string> grid(grid_size, string(grid_size, ' '));
        for (const auto& car : cars) {
            auto from = intersections[car.from];
            auto to = intersections[car.to];
            drawRoad(from.x, from.y, to.x, to.y, grid);
        }
        for (const auto& pair : intersections) {
            const auto& inter = pair.second;
            if (inter.x >= 0 && inter.x < grid_size && inter.y >= 0 && inter.y < grid_size) {
                grid[inter.y][inter.x] = 'O';
            }
        }
        for (const auto& car : cars) {
            auto from = intersections[car.from];
            auto to = intersections[car.to];

            int x = from.x + (to.x - from.x) * car.progress / 100;
            int y = from.y + (to.y - from.y) * car.progress / 100;

            if (x >= 0 && x < grid_size && y >= 0 && y < grid_size) {
                grid[y][x] = car.direction;
            }
        }
        for (const auto& row : grid) {
            for (char c : row) {
                switch (c) {
                    case 'O': setColor(14); break;
                    case '>': case '<': case '^': case 'v': setColor(12); break;
                    case '-': case '|': case '/': case '\\': case '+': setColor(8); break;
                    default: setColor(7);
                }
                cout << c;
            }
            cout << endl;
        }
        setColor(7);
        cout << "\nTRAFFIC CONTROL SYSTEM (Press Enter to stop)\n";
        for (const auto& pair : intersections) {
            const auto& inter = pair.second;
            cout << "Intersection " << pair.first << ": "; 
            setColor(inter.is_green ? 10 : 12);
            cout << (inter.is_green ? "GREEN" : "RED");
            setColor(7);
            cout << " Queue: " << string(inter.queue, 219) 
                 << " (" << inter.queue << ") Wait: " << inter.wait_time << "\n";
        }
        cout << "Current green: ";
        for (const auto& pair : intersections) {
            if (pair.second.is_green) {
                setColor(10);
                cout << "Intersection " << pair.first;
                break;
            }
        }
        setColor(7);
        cout << endl;
    }

    void run() {
        cout << "Starting FAIR traffic simulation...\n";
        cout << "Press Enter to stop\n";
        delay(1000);

        while (true) {
            update();
            draw();
            
            if (_kbhit()) {
                if (cin.get() == '\n') {
                    break;
                }
            }
            
            delay(300);
        }
    }
};

int main() {
    srand(time(0));
    TrafficSystem city;
    city.addIntersection(1, 5, 5);
    city.addIntersection(2, 15, 5);
    city.addIntersection(3, 15, 15);
    city.addIntersection(4, 5, 15);
    city.addIntersection(5, 10, 10); 
    city.addRoad(1, 2);
    city.addRoad(2, 3);
    city.addRoad(3, 4);
    city.addRoad(4, 1);
    city.addRoad(1, 5);
    city.addRoad(2, 5);
    city.addRoad(3, 5);
    city.addRoad(4, 5);

    city.run();

    return 0;
}