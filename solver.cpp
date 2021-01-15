#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

using namespace std;

namespace Solver{
    int numberOfTiles; // количество тайлов
    int maximumSize; // максимальный размер квадрата, который проверяем
    vector<vector<int>> tiles; // заданный набор тайлов 

    // Перебор направлений
    // x - столбцы, y - строки
    // противоположный - ^ 2
    const vector<int> dx = {-1, 0, 1, 0}; // up, right, down, left
    const vector<int> dy = {0, 1, 0, -1};  

    bool isSame(int type1, int type2, int dir1) {
        return tiles[type1][dir1] == tiles[type2][dir1 ^ 2];
    }

    // Проверка замощённого прямоугольника - является ли он периодом
    bool isTilingRectangle(const vector<vector<int>>& table) {
        assert(!table.empty());
        int n = table.size();
        assert(!table[0].empty());
        int m = table[0].size();

        for (int x = 0; x < n; ++x) {
            // table[x][0][3] != table[x][m - 1][1]
            if (!isSame(table[x][0], table[x][m - 1], 3)) {
                return false;
            }
        }

        for (int y = 0; y < m; ++y) {
            // table[0][y][0] != table[n - 1][y][2]
            if (!isSame(table[0][y], table[n - 1][y], 0)) {
                return false;
            }
        }

        return true;
    }

    // Проверка для точки на принадлежность bounding box
    bool isInTable(const vector<vector<int>>& table, int x, int y) {
        if (table.empty() || table[0].empty()) {
            return false;
        }

        return x >= 0 && x < (int)table.size() && y >= 0 && y < (int)table[0].size();
    }

    // Проверка для тайла можно ли его поставить в данную точку
    bool canPutTile(const vector<vector<int>>& table, int x, int y, int type) {
        //cout << "x = " << x << " y = " << y << endl;
        assert(isInTable(table, x, y));

        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (!isInTable(table, nx, ny)) continue;
            if (table[nx][ny] == -1) continue;
            if (!isSame(type, table[nx][ny], dir)) {
                return false;
            }
        }

        return true;
    }

    // конструкции вида: vector<vector<vector<vector<vector<int>>>>> - это смерть, 3 - тоже плохо, но не совсем смерть
    // Если вдруг maximumSize будет >= 123 - нужно поставить N = maximumSize + 1
    const int N = 123;
    vector<vector<vector<int>>> allTables[N][N];
    vector<vector<int>> minimumTilingRectangle;
    vector<vector<int>> maximumTiledRectangle;
    bool foundPeriod;

    // Обновить ответы замощённым прямоугольником
    void relaxAnswers(const vector<vector<int>>& table) {
        if (isTilingRectangle(table)) {
            minimumTilingRectangle = table;
            foundPeriod = true;
        }
        allTables[table.size()][table[0].size()].push_back(table);
    }

    // Рекурсивная штука пытается заполнить полоску от таблички (из -1)
    //  0  1 -1
    //  1  2 -1
    // -1 -1 -1
    void recTryToAdd(vector<vector<int>>& table, int x, int y) {
        if (x == -1) {
            relaxAnswers(table);
        } else {
            assert(!table.empty());
            assert(!table[0].empty());
            int m = table[0].size();

            int nx = x, ny = y;
            if (ny + 1 < m) {
                ++ny;
            } else {
                --nx;
            }

            for (int type = 0; type < numberOfTiles; ++type) {
                if (!canPutTile(table, x, y, type)) continue;
                
                table[x][y] = type;
                recTryToAdd(table, nx, ny);
                table[x][y] = -1;
            }
        }
    }    

    // Расширяет прямоугольник в нужную сторону и запускает рекурсию
    void tryToAdd(const vector<vector<int>>& baseTable, int n, int m) {
        auto table = baseTable;
        for (int x = 0; x < n; ++x) {
            if (table.size() == x) {
                table.push_back(vector<int>(m, -1));
            }
            while (table[x].size() < m) {
                table[x].push_back(-1);
            }
        }

        recTryToAdd(table, n - 1, 0);
    }

    // Перебирать или не перебирать периоды x * 1 (1 - да, 2 - нет)
    const int FIRST_SQUARE_OPTIMIZATION = 1;

    // Перебирать или не перебирать периоды n * m, n < m (1 - да, 2 - нет)
    const int LEXICOGRAPHIC_OPTIMIZATION = 1;

    // main
    void run() {
        for (int h = 0; h <= maximumSize; ++h) {
            for (int w = 0; w <= maximumSize; ++w) {
                allTables[h][w].clear();
                if (h == 0 || w == 0) {
                    allTables[h][w].push_back(vector<vector<int>>());
                }
            }
        } 

        foundPeriod = false;

        for (int h = 1; h <= maximumSize && !foundPeriod; ++h) {
            for (int w = min(h, FIRST_SQUARE_OPTIMIZATION); w <= (LEXICOGRAPHIC_OPTIMIZATION == 1 ? maximumSize : h) && !foundPeriod; ++w) {
                for (const auto& baseTable : allTables[h - 1][w - 1]) {
                    if (foundPeriod) {
                        break;
                    } else {
                        tryToAdd(baseTable, h, w);
                    }
                }
            }
        }

        for (int h = 1; h <= maximumSize; ++h) {
            for (int w = 1; w <= h; ++w) {
                for (const auto& table : allTables[h][w]) {
                    if (!isTilingRectangle(table)) {
                        maximumTiledRectangle = table;
                    }
                }
            }
        }
    }

    // Эта часть для внешнего доступа, запускает Solver на заданном наборе тайлов и выдаёт требуемые ответы
    void solve(
        const int _numberOfTiles,
        const int _maximumSize,
        const vector<vector<int>>& _tiles,
        bool& _foundPeriod,
        vector<vector<int>>& _minimumTilingRectangle,
        vector<vector<int>>& _maximumTiledRectangle
        ) {
        numberOfTiles = _numberOfTiles;
        maximumSize = _maximumSize;
        tiles = _tiles;
        run();
        _foundPeriod = foundPeriod;
        _minimumTilingRectangle = minimumTilingRectangle;
        _maximumTiledRectangle = maximumTiledRectangle;
    }
};

int main() {
    ios_base::sync_with_stdio(0), cin.tie(0), cout.tie(0), cout.precision(20), cout.setf(ios::fixed);
    int numberOfTiles;
    int maximumSize;
    vector<vector<int>> tiles;
    cout << "input number of tiles, maximum size for check, set of tiles (set by set, up-right-down-left colors)" << endl;
    cin >> numberOfTiles;
    cin >> maximumSize;
    tiles.assign(numberOfTiles, vector<int>(4, 0));
    for (auto& tile : tiles) {
        for (auto& side : tile) {
            cin >> side;
        }
    } 
    bool foundPeriod = false;
    vector<vector<int>> minimumTilingRectangle;
    vector<vector<int>> maximumTiledRectangle;
    Solver::solve(numberOfTiles, maximumSize, tiles, foundPeriod, minimumTilingRectangle, maximumTiledRectangle);
    if (foundPeriod) {
        cout << "found the period" << endl;
        assert(!minimumTilingRectangle.empty());
        int n = minimumTilingRectangle.size();
        assert(!minimumTilingRectangle[0].empty());
        int m = minimumTilingRectangle[0].size();
        cout << "rows = " << n << " columns = " << m << endl;
        for (int x = 0; x < n; ++x) {
            for (int y = 0; y < m; ++y) {
                cout << static_cast<char>(minimumTilingRectangle[x][y] + 'A');
            }
            cout << endl;
        }
    } else {
        cout << "didn't find a period" << endl;
        assert(!maximumTiledRectangle.empty());
        int n = maximumTiledRectangle.size();
        assert(!maximumTiledRectangle.empty());
        int m = maximumTiledRectangle[0].size();
        cout << "rows = " << n << " columns = " << m << endl;
        for (int x = 0; x < n; ++x) {
            for (int y = 0; y < m; ++y) {
                cout << static_cast<char>(maximumTiledRectangle[x][y] + 'A');
            }
            cout << endl;
        }
    }
    return 0;
}