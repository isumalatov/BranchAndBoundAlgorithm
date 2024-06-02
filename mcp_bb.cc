// Ilyas Umalatov X7278165E
#include <iostream>
#include <fstream>
#include <vector>
#include <climits>
#include <chrono>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <unordered_set>
#include <utility>

using namespace std;

int nvisit = 0;
int nexplored = 0;
int nleaf = 0;
int nunfeasible = 0;
int nnot_promising = 0;
int npromising_but_discarded = 0;
int nbest_solution_updated_from_leafs = 0;
int nbest_solution_updated_from_pessimistic_bound = 0;

int mcp_pessimistic(const vector<vector<int>> &matrix, vector<pair<int, int>> &best_path, int rows, int cols)
{
    int i = 0, j = 0;
    int coste_origen_destino = matrix[i][j];

    best_path.push_back({i, j});

    while (i != rows - 1 || j != cols - 1)
    {
        int abajo = (i < rows - 1) ? matrix[i + 1][j] : INT_MAX;
        int derecha = (j < cols - 1) ? matrix[i][j + 1] : INT_MAX;
        int diagonal = (i < rows - 1 && j < cols - 1) ? matrix[i + 1][j + 1] : INT_MAX;

        if (diagonal <= derecha && diagonal <= abajo)
        {
            i++;
            j++;
        }
        else if (derecha <= abajo)
        {
            j++;
        }
        else
        {
            i++;
        }

        best_path.push_back({i, j});
        coste_origen_destino += matrix[i][j];
    }

    return coste_origen_destino;
}

int find_min_val(const vector<vector<int>> &matrix, int rows, int cols)
{
    int min_val = INT_MAX;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            min_val = min(min_val, matrix[i][j]);
        }
    }
    return min_val;
}

int mcp_optimistic(const vector<vector<int>> &matrix, int pos_x, int pos_y, int rows, int cols, int min_val)
{
    int steps_remaining = (rows - 1 - pos_x) + (cols - 1 - pos_y);
    return matrix[pos_x][pos_y] + steps_remaining * min_val;
}

int mcp_bb(const vector<vector<int>> &matrix, vector<pair<int, int>> &best_path, int rows, int cols)
{
    struct Node
    {
        int x, y;
        int cost;
        vector<pair<int, int>> path;
        Node(int x, int y, int cost, vector<pair<int, int>> path) : x(x), y(y), cost(cost), path(path) {}
    };

    struct CompareCost
    {
        bool operator()(Node const &n1, Node const &n2)
        {
            return n1.cost > n2.cost;
        }
    };

    priority_queue<Node, vector<Node>, CompareCost> liveNodes;

    unordered_set<string> visited;

    int min_val = find_min_val(matrix, rows, cols);
    int solution = mcp_pessimistic(matrix, best_path, rows, cols);
    Node root(0, 0, matrix[0][0], {{0, 0}});
    liveNodes.push(root);

    while (!liveNodes.empty())
    {
        Node node = liveNodes.top();
        liveNodes.pop();

        if (node.x == rows - 1 && node.y == cols - 1)
        {
            nleaf++;
            if (node.cost < solution)
            {
                nbest_solution_updated_from_leafs++;
                solution = node.cost;
                best_path = node.path;
            }
            continue;
        }

        string pos = to_string(node.x) + "," + to_string(node.y);
        if (visited.find(pos) != visited.end())
        {
            continue;
        }
        visited.insert(pos);

        vector<pair<int, int>> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
        for (auto &direction : directions)
        {
            nvisit++;
            int new_x = node.x + direction.first;
            int new_y = node.y + direction.second;

            if (new_x < rows && new_y < cols && new_x >= 0 && new_y >= 0)
            {
                int new_cost = node.cost + matrix[new_x][new_y];
                int new_optimistic_bound = mcp_optimistic(matrix, new_x, new_y, rows, cols, min_val);

                if (new_cost + new_optimistic_bound < solution)
                {
                    nexplored++;
                    vector<pair<int, int>> new_path = node.path;
                    new_path.push_back({new_x, new_y});
                    liveNodes.push(Node(new_x, new_y, new_cost, new_path));
                }
                else
                {
                    nnot_promising++;
                }
            }
            else
            {
                nunfeasible++;
            }
        }
    }
    return solution;
}

int main(int argc, char *argv[])
{
    bool p2D = false;
    bool p = false;
    if (argc < 3)
    {
        cerr << "ERROR: missing filename.\nUsage:\nmcp [--p2D] [-p] -f file" << endl;
        return 1;
    }

    ifstream file;
    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i]; // Convert to std::string

        if (arg == "--p2D")
        {
            p2D = true;
        }
        else if (arg == "-p")
        {
            p = true;
        }
        else if (arg == "-f")
        {
            file.open(argv[i + 1]);
            i++;
        }
        else
        {
            cerr << "ERROR: unknown option " << arg << ".\nUsage:\nmcp [--p2D] [-p] -f file" << endl;
            return 1;
        }
    }

    if (!file)
    {
        cerr << "No se pudo abrir el archivo.\n";
        return 1;
    }

    int rows, cols;
    file >> rows >> cols;

    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            file >> matrix[i][j];
        }
    }

    vector<pair<int, int>> best_path;
    auto start = chrono::high_resolution_clock::now();
    int best_v = mcp_bb(matrix, best_path, rows, cols);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    double duration_ms = duration.count() / 1000.0;
    cout << best_v << endl;
    cout << nvisit << " " << nexplored << " " << nleaf << " " << nunfeasible << " " << nnot_promising << " " << npromising_but_discarded << " " << nbest_solution_updated_from_leafs << " " << nbest_solution_updated_from_pessimistic_bound << endl;
    cout << fixed << setprecision(3) << duration_ms << endl;

    if (p2D)
    {
        vector<vector<char>> pathMatrix(rows, vector<char>(cols, '.'));
        int pathDifficulty = 0;
        for (const auto &p : best_path)
        {
            pathMatrix[p.first][p.second] = 'x';
            pathDifficulty += matrix[p.first][p.second];
        }

        for (const auto &row : pathMatrix)
        {
            for (const auto &cell : row)
            {
                cout << cell;
            }
            cout << endl;
        }

        cout << pathDifficulty << endl;
    }

    if (p)
    {
        if (best_path.size() == 1)
        {
            cout << "<>" << endl;
        }
        else
        {
            string path = "<";
            for (size_t i = 0; i < best_path.size() - 1; ++i)
            {
                int dx = best_path[i + 1].first - best_path[i].first;
                int dy = best_path[i + 1].second - best_path[i].second;
                if (dx == -1 && dy == 0)
                    path += '1';
                else if (dx == -1 && dy == 1)
                    path += '2';
                else if (dx == 0 && dy == 1)
                    path += '3';
                else if (dx == 1 && dy == 1)
                    path += '4';
                else if (dx == 1 && dy == 0)
                    path += '5';
                else if (dx == 1 && dy == -1)
                    path += '6';
                else if (dx == 0 && dy == -1)
                    path += '7';
                else if (dx == -1 && dy == -1)
                    path += '8';
            }
            path += '>';
            cout << path << endl;
        }
    }

    file.close();
    return 0;
}
