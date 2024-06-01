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

int mcp_pessimistic(const vector<vector<int>> &matrix, int rows, int cols, int pos_x, int pos_y)
{
    int sum = 0;
    for (int i = pos_x; i < rows; i++)
    {
        for (int j = pos_y; j < cols; j++)
        {
            sum += matrix[i][j];
        }
    }
    return sum;
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
    int steps_remaining = (rows - pos_x) + (cols - pos_y);
    return steps_remaining * min_val;
}

int mcp_bb(const vector<vector<int>> &matrix, int rows, int cols)
{
    struct Node
    {
        int x, y;
        int cost;

        Node(int x, int y, int cost)
            : x(x), y(y), cost(cost) {}
    };

    struct CompareCost
    {
        bool operator()(Node const &n1, Node const &n2)
        {
            return n1.cost > n2.cost;
        }
    };

    unordered_set<string> visited;

    priority_queue<Node, vector<Node>, CompareCost> liveNodes;

    int min_val = find_min_val(matrix, rows, cols);
    Node root(0, 0, matrix[0][0]);
    int solution = mcp_pessimistic(matrix, rows, cols, 0, 0);
    liveNodes.push(root);

    while (!liveNodes.empty())
    {
        Node node = liveNodes.top();
        liveNodes.pop();

        if (node.cost + mcp_optimistic(matrix, node.x, node.y, rows, cols, min_val) > solution)
        {
            npromising_but_discarded++;
            continue;
        }

        if (node.x == rows - 1 && node.y == cols - 1)
        {
            nleaf++;
            if (node.cost < solution)
            {
                nbest_solution_updated_from_leafs++;
                solution = node.cost;
            }
            continue;
        }

        vector<pair<int, int>> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
        for (auto &direction : directions)
        {
            nvisit++;
            int new_x = node.x + direction.first;
            int new_y = node.y + direction.second;
            string pos = to_string(new_x) + "," + to_string(new_y);

            if (new_x < rows && new_y < cols && new_x >= 0 && new_y >= 0 && visited.find(pos) == visited.end())
            {
                int new_cost = node.cost + matrix[new_x][new_y];
                int new_optimistic_bound = mcp_optimistic(matrix, new_x, new_y, rows, cols, min_val);
                int new_pessimistic_bound = mcp_pessimistic(matrix, rows, cols, new_x, new_y);

                if (node.cost + new_pessimistic_bound < solution)
                {
                    nbest_solution_updated_from_pessimistic_bound++;
                    solution = node.cost + new_pessimistic_bound;
                }
                if (node.cost + new_optimistic_bound < solution)
                {
                    nexplored++;
                    liveNodes.push(Node(new_x, new_y, new_cost));
                    visited.insert(pos);
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

    auto start = chrono::high_resolution_clock::now();
    int best_v = mcp_bb(matrix, rows, cols);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    double duration_ms = duration.count() / 1000.0;
    cout << best_v << endl;
    cout << nvisit << " " << nexplored << " " << nleaf << " " << nunfeasible << " " << nnot_promising << " " << npromising_but_discarded << " " << nbest_solution_updated_from_leafs << " " << nbest_solution_updated_from_pessimistic_bound << endl;
    cout << fixed << setprecision(3) << duration_ms << endl;

    if (p2D)
    {
    }

    if (p)
    {
    }

    file.close();
    return 0;
}
