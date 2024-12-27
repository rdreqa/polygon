#include <vector>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <stack>
#include <algorithm>
#include <chrono>
#include <cassert>


// Структура, представляющая узел графа
struct Node {
    double lon, lat; // Долгота и широта узла
    std::vector<std::pair<Node*, double>> edges;// Список рёбер (соседей) и их весов
};

// Хеш-функция для пар (пара долгот и широт), используется для хранения узлов в unordered_map
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);  // Хеширование первого элемента пары
        auto h2 = std::hash<T2>{}(p.second); // Хеширование второго элемента пары
        return h1 ^ (h2 << 1);  // Комбинирование хешей
    }
};

// Структура графа
struct Graph {
    std::unordered_map<std::pair<double, double>, std::unique_ptr<Node>, pair_hash> nodes; // Мапа узлов графа с координатами в качестве ключа

    // Метод для поиска ближайшего узла по координатам
    Node* find_closest_node(double lat, double lon) {
        double min_distance = std::numeric_limits<double>::max(); // Инициализация максимальным расстоянием
        Node* closest_node = nullptr; // Указатель на ближайший узел

        // Проходим по всем узлам и находим ближайший
        for (const auto& node : nodes) {
            double distance = std::sqrt(std::pow(node.second->lat - lat, 2) + std::pow(node.second->lon - lon, 2));
            if (distance < min_distance) {
                closest_node = node.second.get();  // Обновляем ближайший узел
                min_distance = distance;
            }
        }
        return closest_node;
    }

    // Метод для добавления нового узла в граф
    void add_node(double lon, double lat) {
        if (nodes.find({lat, lon}) == nodes.end()) {  // Проверка, есть ли уже узел с такими координатами
            auto node = std::make_unique<Node>();   // Создаём новый узел
            node->lon = lon;
            node->lat = lat;
            nodes[{lat, lon}] = std::move(node);  // Добавляем узел в граф
        }
    }

    // Метод для добавления ребра между двумя узлами с весом
    void add_edge(double lon1, double lat1, double lon2, double lat2, double weight) {
        // Находим узлы, между которыми будет добавлено ребро
        auto it1 = nodes.find({lat1, lon1});
        auto it2 = nodes.find({lat2, lon2});

        // Если оба узла существуют в графе, добавляем ребро от node1 к node2
        if (it1 != nodes.end() && it2 != nodes.end()) {
            Node* node1 = it1->second.get();
            Node* node2 = it2->second.get();


            node1->edges.emplace_back(node2, weight);
            node2->edges.emplace_back(node1, weight);
        }
    }

    // Метод для загрузки графа из файла
    void load_graph_from_file(const std::string& filename) {
        std::ifstream file(filename);

        if (!file.is_open()) {  // Если не удалось открыть файл
            std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line)) {  // Чтение построчно
            std::stringstream ss(line);
            std::string node_part, neighbor_part;

            // Чтение координат текущего узла
            std::getline(ss, node_part, ':');
            double lon1, lat1;
            if (sscanf(node_part.c_str(), "%lf,%lf", &lon1, &lat1) != 2) {
                continue;
            }
            add_node(lon1, lat1);  // Добавляем узел в граф

            // Чтение соседей текущего узла
            while (std::getline(ss, neighbor_part, ';')) {
                double lon2, lat2, weight;
                if (sscanf(neighbor_part.c_str(), "%lf,%lf,%lf", &lon2, &lat2, &weight) == 3) {
                    add_node(lon2, lat2);  // Добавляем соседа
                    add_edge(lon1, lat1, lon2, lat2, weight);  // Добавляем ребро
                }
            }
        }
    }

    // Метод для поиска пути с использованием алгоритма BFS (поиск в ширину)
    std::vector<Node*> bfs(Node* start, Node* goal) {
        std::queue<Node*> q;
        std::unordered_map<Node*, Node*> parent;
        std::unordered_set<Node*> visited;

        q.push(start);
        visited.insert(start);
        parent[start] = nullptr;

        while (!q.empty()) {
            Node* current = q.front();
            q.pop();

            if (current == goal) {  // Если достигли целевого узла
                std::vector<Node*> way;
                while (current) {  // Восстанавливаем путь от цели к началу
                    way.push_back(current);
                    current = parent[current];
                }
                std::reverse(way.begin(), way.end());  // Разворачиваем путь
                return way;
            }

            for (auto [neighbor, _] : current->edges) {  // Проходим по соседям текущего узла
                if (visited.find(neighbor) == visited.end()) {  // Если сосед ещё не посещён
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }

        return {};  // Если путь не найден
    }

    // Метод для поиска пути с использованием алгоритма DFS (поиск в глубину)
    std::vector<Node*> dfs(Node* start, Node* goal) {
        std::stack<Node*> stack;
        std::unordered_map<Node*, Node*> parent;
        std::unordered_set<Node*> visited;

        stack.push(start);
        parent[start] = nullptr;

        while (!stack.empty()) {
            Node* current = stack.top();
            stack.pop();

            if (visited.find(current) != visited.end()) {
                continue;  // Пропускаем уже посещённые узлы
            }
            visited.insert(current);

            if (current == goal) {  // Если достигли целевого узла
                std::vector<Node*> way;
                while (current) {
                    way.push_back(current);
                    current = parent[current];
                }
                std::reverse(way.begin(), way.end());  // Разворачиваем путь
                return way;
            }

            for (auto [neighbor, _] : current->edges) {
                // Проходим по соседям текущего узла
                if (visited.find(neighbor) == visited.end()) {  // Если сосед ещё не посещён
                    parent[neighbor] = current;
                    stack.push(neighbor);
                }
            }
        }

        return {};  // Если путь не найден
    }

    // Метод для поиска пути с использованием алгоритма Дейкстры
    std::vector<Node*> dijkstra(Node* start, Node* goal) {
        std::priority_queue<std::pair<double, Node*>, std::vector<std::pair<double, Node*>>, std::greater<>> pq;
        std::unordered_map<Node*, double> distances;
        std::unordered_map<Node*, Node*> parent;

        // Инициализация всех узлов с бесконечными расстояниями
        for (auto& node : nodes) {
            distances[node.second.get()] = std::numeric_limits<double>::infinity();
        }

        distances[start] = 0;
        pq.push({0, start});
        parent[start] = nullptr;

        while (!pq.empty()) {
            auto [current_dist, current] = pq.top();
            pq.pop();

            if (current == goal) {  // Если достигли целевого узла
                std::vector<Node*> way;
                while (current) {
                    way.push_back(current);
                    current = parent[current];
                }
                std::reverse(way.begin(), way.end());  // Разворачиваем путь
                return way;
            }

            for (auto [neighbor, edge_dist] : current->edges) {  // Проходим по соседям текущего узла
                double new_dist = current_dist + edge_dist;

                if (new_dist < distances[neighbor]) {  // Если нашли более короткий путь
                    distances[neighbor] = new_dist;
                    pq.push({new_dist, neighbor});
                    parent[neighbor] = current;
                }
            }
        }

        return {};  // Если путь не найден
    }

    // Эвристическая функция для A* (расстояние между двумя узлами)
    double heuristic(Node* a, Node* b) {
        return std::sqrt(std::pow(a->lat - b->lat, 2) + std::pow(a->lon - b->lon, 2));
    }

    // Метод для поиска пути с использованием алгоритма A* (поиск с учётом эвристики)
    std::vector<Node*> a_star(Node* start, Node* goal) {
        std::priority_queue<std::pair<double, Node*>, std::vector<std::pair<double, Node*>>, std::greater<>> open_set;
        std::unordered_map<Node*, double> g_score;
        std::unordered_map<Node*, double> f_score;
        std::unordered_map<Node*, Node*> came_from;

        // Инициализация всех узлов с бесконечными значениями для g и f
        for (const auto& node : nodes) {
            g_score[node.second.get()] = std::numeric_limits<double>::infinity();
            f_score[node.second.get()] = std::numeric_limits<double>::infinity();
        }

        g_score[start] = 0;
        f_score[start] = heuristic(start, goal);
        open_set.push({f_score[start], start});
        came_from[start] = nullptr;

        while (!open_set.empty()) {
            Node* current = open_set.top().second;
            open_set.pop();

            if (current == goal) {  // Если достигли целевого узла
                std::vector<Node*> way;
                while (current) {
                    way.push_back(current);
                    current = came_from[current];
                }
                std::reverse(way.begin(), way.end());  // Разворачиваем путь
                return way;
            }

            for (auto [neighbor, weight] : current->edges) {  // Проходим по соседям текущего узла
                double tentative_g_score = g_score[current] + weight;

                if (tentative_g_score < g_score[neighbor]) {  // Если нашли более короткий путь
                    came_from[neighbor] = current;
                    g_score[neighbor] = tentative_g_score;
                    f_score[neighbor] = g_score[neighbor] + heuristic(neighbor, goal);
                    open_set.push({f_score[neighbor], neighbor});
                }
            }
        }

        return {};  // Если путь не найден
    }
    // Функция для вычисления расстояния между двумя точками (по Пифагору)
    double calculate_distance(const Node* start, const Node* goal) {
        return std::sqrt(std::pow(goal->lat - start->lat, 2) + std::pow(goal->lon - start->lon, 2));
    }

    // Метод для вывода расстояния по пути
    double get_way_distance(const std::vector<Node*>& way) {
        double total_distance = 0.0;
        for (size_t i = 1; i < way.size(); ++i) {
            for (auto& [neighbor, weight] : way[i-1]->edges) {
                if (neighbor == way[i]) {
                    total_distance += weight;
                    break;
                }
            }
        }
        return total_distance;
    }

};


void test_add_node() {
    Graph graph;

    // Добавляем узел в граф
    graph.add_node(1.0, 2.0);
    Node* node = graph.find_closest_node(2.0, 1.0);

    // Проверяем, что узел был добавлен
    assert(node != nullptr);
    assert(node->lon == 1.0);
    assert(node->lat == 2.0);

    std::cout << "Test add_node passed!" << std::endl;
}

void test_add_edge() {
    Graph graph;

    // Добавляем два узла
    graph.add_node(1.0, 2.0);
    graph.add_node(3.0, 4.0);

    // Добавляем ребро между узлами
    graph.add_edge(1.0, 2.0, 3.0, 4.0, 10.0);

    Node* node1 = graph.find_closest_node(2.0, 1.0);
    Node* node2 = graph.find_closest_node(4.0, 3.0);

    // Проверяем, что ребро между узлами существует
    bool edge_exists = false;
    for (auto& [neighbor, weight] : node1->edges) {
        if (neighbor == node2 && weight == 10.0) {
            edge_exists = true;
            break;
        }
    }

    assert(edge_exists);
    std::cout << "Test add_edge passed!" << std::endl;
}

void test_bfs() {
    Graph graph;

    // Добавляем узлы
    graph.add_node(1.0, 2.0);
    graph.add_node(3.0, 4.0);
    graph.add_node(5.0, 6.0);

    // Добавляем рёбра
    graph.add_edge(1.0, 2.0, 3.0, 4.0, 1.0);
    graph.add_edge(3.0, 4.0, 5.0, 6.0, 1.0);

    Node* start = graph.find_closest_node(2.0, 1.0);
    Node* goal = graph.find_closest_node(6.0, 5.0);

    // Поиск пути с помощью BFS
    std::vector<Node*> path = graph.bfs(start, goal);

    // Проверяем, что путь найден
    assert(!path.empty());
    assert(path.front() == start);
    assert(path.back() == goal);

    std::cout << "Test BFS passed!" << std::endl;
}

void test_dfs() {
    Graph graph;

    // Добавляем узлы
    graph.add_node(1.0, 2.0);
    graph.add_node(3.0, 4.0);
    graph.add_node(5.0, 6.0);

    // Добавляем рёбра
    graph.add_edge(1.0, 2.0, 3.0, 4.0, 1.0);
    graph.add_edge(3.0, 4.0, 5.0, 6.0, 1.0);

    Node* start = graph.find_closest_node(2.0, 1.0);
    Node* goal = graph.find_closest_node(6.0, 5.0);

    // Поиск пути с помощью DFS
    std::vector<Node*> path = graph.dfs(start, goal);

    // Проверяем, что путь найден
    assert(!path.empty());
    assert(path.front() == start);
    assert(path.back() == goal);

    std::cout << "Test DFS passed!" << std::endl;
}

void test_dijkstra() {
    Graph graph;

    // Добавляем узлы
    graph.add_node(1.0, 2.0);
    graph.add_node(3.0, 4.0);
    graph.add_node(5.0, 6.0);

    // Добавляем рёбра с весами
    graph.add_edge(1.0, 2.0, 3.0, 4.0, 1.0);
    graph.add_edge(3.0, 4.0, 5.0, 6.0, 1.0);

    Node* start = graph.find_closest_node(2.0, 1.0);
    Node* goal = graph.find_closest_node(6.0, 5.0);

    // Поиск пути с помощью Дейкстры
    std::vector<Node*> path = graph.dijkstra(start, goal);

    // Проверяем, что путь найден
    assert(!path.empty());
    assert(path.front() == start);
    assert(path.back() == goal);

    std::cout << "Test Dijkstra passed!" << std::endl;
}

void test_a_star() {
    Graph graph;

    // Добавляем узлы
    graph.add_node(1.0, 2.0);
    graph.add_node(3.0, 4.0);
    graph.add_node(5.0, 6.0);

    // Добавляем рёбра с весами
    graph.add_edge(1.0, 2.0, 3.0, 4.0, 1.0);
    graph.add_edge(3.0, 4.0, 5.0, 6.0, 1.0);

    Node* start = graph.find_closest_node(2.0, 1.0);
    Node* goal = graph.find_closest_node(6.0, 5.0);

    // Поиск пути с помощью A*
    std::vector<Node*> path = graph.a_star(start, goal);

    // Проверяем, что путь найден
    assert(!path.empty());
    assert(path.front() == start);
    assert(path.back() == goal);

    std::cout << "Test A* passed!" << std::endl;
}


// Главная функция
int main() {
    test_add_node();
    test_add_edge();
    test_bfs();
    test_dfs();
    test_dijkstra();
    test_a_star();
    std::cout << "All tests passed!" << std::endl;
    Graph graph;


    // Загружаем граф из файла
    graph.load_graph_from_file("spb_graph.txt");

   // Стартовая и конечная вершина
    double start_lon = 30.453104, start_lat =  60.044417,  goal_lon = 30.32306, goal_lat = 59.85202;

    // Находим ближайшие узлы к начальной и целевой точке
    Node* start_node = graph.find_closest_node(start_lat, start_lon);
    Node* goal_node = graph.find_closest_node(goal_lat, goal_lon);

    if (!start_node && !goal_node) {
        std::cout << "node not found." << std::endl;
        return 0;
    }

    // Измеряем время для поиска пути с помощью различных алгоритмов
    auto start_time = std::chrono::high_resolution_clock::now();
    auto bfs_way = graph.bfs(start_node, goal_node);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "BFS: " << std::chrono::duration<double>(end_time - start_time).count() << " seconds." << std::endl;

    if (!bfs_way.empty()) {
        double bfs_distance = graph.get_way_distance(bfs_way);
        std::cout << "BFS way distance: " << bfs_distance << std::endl;
    }


    start_time = std::chrono::high_resolution_clock::now();
    auto dfs_way = graph.dfs(start_node, goal_node);
    end_time = std::chrono::high_resolution_clock::now();
    std::cout << "DFS: " << std::chrono::duration<double>(end_time - start_time).count() << " seconds." << std::endl;
    if (!dfs_way.empty()) {
        double dfs_distance = graph.get_way_distance(dfs_way);
        std::cout << "DFS way distance: " << dfs_distance << std::endl;
    }

    start_time = std::chrono::high_resolution_clock::now();
    auto dijkstra_way = graph.dijkstra(start_node, goal_node);
    end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Dijkstra: " << std::chrono::duration<double>(end_time - start_time).count() << " seconds." << std::endl;
    if (!dijkstra_way.empty()) {
        double dijkstra_distance = graph.get_way_distance(dijkstra_way);
        std::cout << "Dijkstra way distance: " << dijkstra_distance << std::endl;
    }

    start_time = std::chrono::high_resolution_clock::now();
    auto a_star_way = graph.a_star(start_node, goal_node);
    end_time = std::chrono::high_resolution_clock::now();
    std::cout << "A*: " << std::chrono::duration<double>(end_time - start_time).count() << " seconds." << std::endl;
    if (!a_star_way.empty()) {
        double a_star_distance = graph.get_way_distance(a_star_way);
        std::cout << "A* way distance: " << a_star_distance << std::endl;
    }

    // std::cout << "BFS Way: ";
    // for (Node* n : bfs_way) {
    //     std::cout << "(" << n->lon << "," << n->lat << ") ";
    // }
    //
    // std::cout << "DFS Way ";
    // for (Node* n : dfs_way) {
    //     std::cout << "(" << n->lon << "," << n->lat << ") ";
    // }
    //
    // std::cout << "Dijkstra Way: ";
    // for (Node* n : dijkstra_way) {
    //     std::cout << "(" << n->lon << "," << n->lat << ") ";
    // }
    //
    // std::cout << "A* Way: ";
    // for (Node* n : a_star_way) {
    //     std::cout << "(" << n->lon << "," << n->lat << ") ";
    // }

    return 0;
}


