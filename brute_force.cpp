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

int numberOfTiles;
int numberOfColors;
int maximumSize;

// Если вдруг maximumSize будет >= 123 - нужно поставить N = maximumSize + 1
const int N = 123;    

void inputParameters() {
    cout << "input number of tiles, number of colors (on both sides), maximum size for check" << endl;
    cin >> numberOfTiles >> numberOfColors >> maximumSize;
    assert(maximumSize < N);
}

int numberOfAllSets;

int numberOfAllTilingSets;
int numberOfAllNonTilingSets;

int numberOfTilingSets[N][N];
int numberOfNonTilingSets[N][N];

vector<vector<int>> sampleTilingSet[N][N];
vector<vector<int>> sampleTilingRectangle[N][N];

vector<vector<int>> sampleNonTilingSet[N][N];
vector<vector<int>> sampleNonTilingRectangle[N][N];

// Инициализирует все массивы и счётчики
void initData() {
    numberOfAllSets = 0;

    numberOfAllTilingSets = 0;
    numberOfAllNonTilingSets = 0;

    for (int h = 0; h < N; ++h) {
        for (int w = 0; w < N; ++w) {
            numberOfTilingSets[h][w] = 0;
            numberOfNonTilingSets[h][w] = 0;

            sampleTilingSet[h][w] = {};
            sampleTilingRectangle[h][w] = {};

            sampleNonTilingSet[h][w] = {};
            sampleNonTilingRectangle[h][w] = {};
        }
    }
}

const int numberOfSides = 4;
vector<vector<int>> allTiles;

// Генерация всех тайлов с заданными цветами
void genAllTiles(vector<int> tile) {
    if (tile.size() == numberOfSides) {
        allTiles.push_back(tile);
    } else {
        tile.push_back(0);
        for (int i = 0; i < numberOfColors; ++i) {
            tile[tile.size() - 1] = i;
            genAllTiles(tile);
        } 
    }
}

const int OUTPUT_EVERY_CONST_ITERATIONS = 10000;

// Обновить ответы заданным набором тайлов
void relaxAnswers(const vector<vector<int>>& tiles) {
    bool foundPeriod = false;
    vector<vector<int>> minimumTilingRectangle;
    vector<vector<int>> maximumTiledRectangle;
    Solver::solve(numberOfTiles, maximumSize, tiles, foundPeriod, minimumTilingRectangle, maximumTiledRectangle);
    if (foundPeriod) {
        ++numberOfAllTilingSets;
        assert(!minimumTilingRectangle.empty());
        int h = minimumTilingRectangle.size();
        assert(!minimumTilingRectangle[0].empty());
        int w = minimumTilingRectangle[0].size();
        ++numberOfTilingSets[h][w];
        if (numberOfTilingSets[h][w] == 1) {
            sampleTilingSet[h][w] = tiles;
            sampleTilingRectangle[h][w] = minimumTilingRectangle;
        }
    } else {
        ++numberOfAllNonTilingSets;
        assert(!maximumTiledRectangle.empty());
        int h = maximumTiledRectangle.size();
        assert(!maximumTiledRectangle[0].empty());
        int w = maximumTiledRectangle[0].size();
        ++numberOfNonTilingSets[h][w];
        if (numberOfNonTilingSets[h][w] == 1) {
            sampleNonTilingSet[h][w] = tiles;
            sampleNonTilingRectangle[h][w] = maximumTiledRectangle;
        }
    }
    ++numberOfAllSets;
    if (numberOfAllSets % OUTPUT_EVERY_CONST_ITERATIONS == 0) {
        cout << "Checked first " << numberOfAllSets << " sets" << endl;
        cout << "Number of tiling sets = " << numberOfAllTilingSets << endl;
        cout << "Number of non tiling sets = " << numberOfAllNonTilingSets << endl;
        cout << endl;
    }
}

// Первый тайл в наборе (0 0 <=1 <=1) (1 - нет, 2 - да)
const int FIRST_OPTIMUM_TILE_OPTIMIZATION = 2; 

// Перебор всех наборов тайлов
void recAllSetOfTiles(vector<vector<int>>& tiles, int sizeOfSet, int pos) {
    if (sizeOfSet == numberOfTiles) {
        relaxAnswers(tiles);    
    } else if (pos < allTiles.size()) {
        recAllSetOfTiles(tiles, sizeOfSet, pos + 1);
        if (FIRST_OPTIMUM_TILE_OPTIMIZATION == 2) {
            if (sizeOfSet == 0) {
                if (allTiles[pos][0] != 0 || allTiles[pos][1] != 0 || allTiles[pos][2] > 1 || allTiles[pos][3] > 1) {
                    return;
                }
            }
        }
        tiles[sizeOfSet] = allTiles[pos];
        recAllSetOfTiles(tiles, sizeOfSet + 1, pos + 1);
    }
}

// main перебор
void generate() {
    genAllTiles({});
    cout << "genAllTiles: ok, numer of tiles = " << allTiles.size() << endl;
    cout << endl;
    vector<vector<int>> tiles(numberOfTiles);
    recAllSetOfTiles(tiles, 0, 0);
    cout << "recAllSetOfTiles: ok" << endl;
    cout << endl;
}

// Вывод статистики
void outputResults() {
    cout << "number of checked sets = " << numberOfAllSets << endl;
    cout << "number of tiling sets = " << numberOfAllTilingSets << endl;
    cout << "number of non tiling sets = " << numberOfAllNonTilingSets << endl;
    cout << endl;

    cout << "statistics on the number of tiling sets for a given minimum period" << endl;
    for (int h = 1; h < N; ++h) {
        for (int w = 1; w < N; ++w) {
            if (numberOfTilingSets[h][w] == 0) {
                continue;
            } else {
                cout << "h = " << h << " w = " << w << " number of tiling sets = " << numberOfTilingSets[h][w] << endl;
            }
        }
    }
    cout << endl;

    cout << "statistics on the number of non tiling sets for a given maximum tiled rectangle" << endl;
    for (int h = 1; h < N; ++h) {
        for (int w = 1; w < N; ++w) {
            if (numberOfNonTilingSets[h][w] == 0) {
                continue;
            } else {
                cout << "h = " << h << " w = " << w << " number of non tiling sets = " << numberOfNonTilingSets[h][w] << endl;
            }
        }
    }
    cout << endl;
}

// Вывод конкретных примеров
void answerForQueries() {
    while (true) {
        cout << "input type of query: 0 - tiling set, 1 - non tiling set, -1 - stop program" << endl;
        int type;
        cin >> type;
        assert(type >= -1 && type <= 1);
        if (type == -1) {
            break;
        } else if (type == 0) {
            cout << "input size of minimum tiling rectangle: (height width)" << endl;
            int h, w;
            cin >> h >> w;
            if (numberOfTilingSets[h][w] == 0) {
                cout << "fail: there aren't relevant sets for your's query" << endl;
            } else {
                cout << "sample set" << endl;
                for (const auto& tile : sampleTilingSet[h][w]) {
                    for (const auto& side : tile) {
                        cout << side << " ";
                    }
                    cout << endl;
                }
                cout << "minimum period for sample set" << endl;
                for (int x = 0; x < h; ++x) {
                    for (int y = 0; y < w; ++y) {
                        cout << static_cast<char>('A' + sampleTilingRectangle[h][w][x][y]);
                    }
                    cout << endl;
                }
            }
        } else if (type == 1) {
            cout << "input size of maximum tiled rectangle: (height width)" << endl;
            int h, w;
            cin >> h >> w;
            if (numberOfNonTilingSets[h][w] == 0) {
                cout << "fail: there aren't relevant sets for your's query" << endl;
            } else {
                cout << "sample set" << endl;
                for (const auto& tile : sampleNonTilingSet[h][w]) {
                    for (const auto& side : tile) {
                        cout << side << " ";
                    }
                    cout << endl;
                }
                cout << "maximum tiled rectangle for sample set" << endl;
                for (int x = 0; x < h; ++x) {
                    for (int y = 0; y < w; ++y) {
                        cout << static_cast<char>('A' + sampleNonTilingRectangle[h][w][x][y]);
                    }
                    cout << endl;
                }
            }
        }
        cout << endl;
    }
}

int main() {
    inputParameters();
    initData();
    generate();
    outputResults();
    answerForQueries();
    return 0;
}