#include <iostream>
#include <fstream>
#include <cstring>   // for memset()
#include <algorithm> // for sort()
using namespace std;

#define DEBUG

#define CELL_UNSET  0 // must be zero (memset() can only fill with 0 or -1)
#define CELL_FILLED 1
#define CELL_EMPTY  2

#define SYMBOL_UNKNOWN "  "
#define SYMBOL_FILL    "██" // FULL BLOCK (U+2588)
#define SYMBOL_EMPTY   "··"
// #define SYMBOL_FILL  "●"
// #define SYMBOL_EMPTY "○"

#define LINE_ROW 1
#define LINE_COL 2

#define MAX_SIZE 100

class Nonogram {
private:
    int width;
    int height;
    int remaining_cells;
    char map[MAX_SIZE][MAX_SIZE];
    int row_hints[MAX_SIZE][MAX_SIZE];
    int col_hints[MAX_SIZE][MAX_SIZE];
    int row_hint_cnt[MAX_SIZE];
    int col_hint_cnt[MAX_SIZE];
    bool is_row_filled[MAX_SIZE] = { false, };
    bool is_col_filled[MAX_SIZE] = { false, };
    bool row_need_to_check[MAX_SIZE];
    bool col_need_to_check[MAX_SIZE];

    // Check that hints are used correctly.
    bool is_line_right(const char line[], const int line_len, const int hints[], const int hint_cnt) const {
        int i, hint_now = 0, cnt = 0;
        for (i = 0; i <= line_len; i++) {
            if (i != line_len && line[i] == CELL_FILLED) {
                cnt++;
            } else {
                if (cnt == 0) continue;
                if (hint_now == hint_cnt || hints[hint_now] != cnt) {
                    return false;
                }
                hint_now++;
                cnt = 0;
            }
        }
        return (hint_now == hint_cnt);
    }

    // Calculates the maximum duplicate count when the hint is used in all possible ways.
    int calc_line_dup_count(const char line[], const int line_len, const int pos, const int hints[], const int hint_cnt, const int hint_now, int dup_cnt[], int recur_cnt) const {
        int i, j, k, r = 0, remaining = 0;
        char temp[MAX_SIZE];
        // cout << "pos=" << pos << ", hint_now=" << hint_now << ", recur_cnt=" << recur_cnt << "\n";

        if (hint_now == hint_cnt) {
            return is_line_right(line, line_len, hints, hint_cnt);
        }
        for (i = hint_now; i < hint_cnt; i++) {
            remaining += hints[i];
        }
        remaining += (hint_cnt - hint_now) - 1;

        for (i = 0; i < line_len; i++) {
            temp[i] = line[i];
        }

        for (i = pos; i <= line_len - remaining; i++) {
            for (j = 0; j < hints[hint_now]; j++) {
                if (line[i + j] == CELL_EMPTY) {
                    break;
                }
            }
            if (j != hints[hint_now]) continue;
            if (i + j != line_len && line[i + j] == CELL_FILLED) continue;

            for (j = 0; j < hints[hint_now]; j++) {
                temp[i + j] = CELL_FILLED; // fill
            }
            
            recur_cnt++;
            k = calc_line_dup_count(temp, line_len, i + hints[hint_now] + 1, hints, hint_cnt, hint_now + 1, dup_cnt, recur_cnt);
            if (k == -1) return -1;

            for (j = 0; j < hints[hint_now]; j++) {
                temp[i + j] = line[i + j]; // restore
                dup_cnt[i + j] += k; // count
            }

            r += k;
        }
        return r;
    }

    // Fill in the lines based on hints.
    bool fill_line(char line[], const int line_type, const int hints[], const int hint_cnt) {
        int i, line_len, dup_max, dup_cnt[MAX_SIZE] = { 0, };
        bool* need_to_check, *curr;
        if (line_type == LINE_ROW) {
            line_len = width;
            need_to_check = col_need_to_check;
            curr = row_need_to_check;
        } else {
            line_len = height;
            need_to_check = row_need_to_check;
            curr = col_need_to_check;
        }
        #define FILL(i, c) if (line[i] == CELL_UNSET) { line[i] = c; need_to_check[i] = true; remaining_cells--;  }//curr[i] = true;

        // Check if all cells are empty or filled.
        // In other words, if there is only one hint and it's 0 or the length of the line.
        if (hint_cnt == 1) { 
            if (hints[0] == 0) {
                for (i = 0; i < line_len; i++) {
                    FILL(i, CELL_EMPTY);
                }
                return true;
            } else if (hints[0] == line_len) {
                for (i = 0; i < line_len; i++) {
                    FILL(i, CELL_FILLED);
                }
                return true;
            }
        }
 
        // CORE LOGIC
        // 1) Try every possible case that can be filled with hints by backtracking.
        // 2) Find out which cells are always filled or always empty.
        dup_max = calc_line_dup_count(line, line_len, 0, hints, hint_cnt, 0, dup_cnt, 0);
        if (dup_max != -1) {
            for (i = 0; i < line_len; i++) {
                if (dup_cnt[i] == 0) {
                    // Empty any cells that will always be empty. (dup_max can be 0, so check empty first.)
                    FILL(i, CELL_EMPTY);
                } else if (dup_cnt[i] == dup_max) {
                    // Fill any cells that will always be filled.
                    FILL(i, CELL_FILLED);
                }
            }
        }

        // If all hints are used correctly and fill all possible CELL_FILLED's, fill remaining cells with CELL_EMPTY.
        if (is_line_right(line, line_len, hints, hint_cnt)) { 
            for (i = 0; i < line_len; i++) {
                FILL(i, CELL_EMPTY);
            }
            return true;
        }

        // Returns false if there are still unfilled cells.
        return false;
    }

public:
    Nonogram(int width, int height) : width(width), height(height) {
        if (width > MAX_SIZE || height > MAX_SIZE) { 
            throw "Too large map size";
        }
        remaining_cells = width * height;
        memset(map, 0, sizeof(map)); // CELL_UNSET
        for (int i = 0; i < MAX_SIZE; i++) {
            row_need_to_check[i] = true;
            col_need_to_check[i] = true;
        }
    }

    // Fill row array.
    bool fill_row(int idx) {
        // Row array is stored contiguously in memory, so there is no need to copy it to a temporary array.
        if (is_row_filled[idx]) return true;
        if (fill_line(&map[idx][0], LINE_ROW, row_hints[idx], row_hint_cnt[idx])) {
            is_row_filled[idx] = true;
            return true;
        }
        return false;
    }

    // Fill column array.
    // Column array is stored discontinuous in memory, so they should be copied to a temporary array.
    bool fill_col(int idx) {
        char temp[MAX_SIZE];
        int i;
        if (is_col_filled[idx]) return true;
        for (i = 0; i < height; i++) { // Copy to the temp array.
            temp[i] = map[i][idx];
        }
        if (fill_line(temp, LINE_COL, col_hints[idx], col_hint_cnt[idx])) {
            is_col_filled[idx] = true;
        }
        for (i = 0; i < height; i++) { // Copy from the temp array.
            map[i][idx] = temp[i];
        }
        return is_col_filled[idx];
    }

    void input() {
        int i, j;
        for (i = 0; i < height; i++) {
            cin >> row_hint_cnt[i];
            for (j = 0; j < row_hint_cnt[i]; j++) {
                cin >> row_hints[i][j];
            }
        }
        for (i = 0; i < width; i++) {
            cin >> col_hint_cnt[i];
            for (j = 0; j < col_hint_cnt[i]; j++) {
                cin >> col_hints[i][j];
            }
        }
    }

    bool solve() {
        int i, j, k;

        // The more hints are given, the easier the line can be filled.
        // Therefore, sort the array in descending order by the sum of hints.
        struct order {
            int line_idx;
            int line_type;
            int sum_hint;
            bool operator<(const order& a) const { return sum_hint < a.sum_hint; }
            bool operator>(const order& a) const { return sum_hint > a.sum_hint; }
        };
        struct order ord[MAX_SIZE * 2];
        for (i = 0; i < height; i++) {
            for (j = 0, k = 0; j < row_hint_cnt[i]; j++) {
                k += row_hints[i][j];
            }
            ord[i] = { i, LINE_ROW, k };
        }
        for (i = 0; i < width; i++) {
            for (j = 0, k = 0; j < col_hint_cnt[i]; j++) {
                k += col_hints[i][j];
            }
            ord[height + i] = { i, LINE_COL, k };
        }
        sort(ord, ord + width + height, greater<order>());
#ifdef DEBUG
        cout << "Results sorted by the sum of hints: \n";
        for (i = 0; i < width + height; i++) {
            cout << (ord[i].line_type == LINE_ROW ? "Row" : "Col") << " "; cout.width(2);
            cout << ord[i].line_idx << " (i="; cout.width(2); cout << i << ") = " << ord[i].sum_hint << "\n";
        }
#endif

        cout << "\nStart solving...\n";

        int step = 1, remaining_old = remaining_cells;
        const int total_cnt = width * height;

        while (remaining_cells > 0) {
            for (i = 0; i < width + height; i++) {
                int idx = ord[i].line_idx;
                if (ord[i].line_type == LINE_ROW) {
                    if (is_row_filled[idx])      continue;
                    if (!row_need_to_check[idx]) continue;
                    row_need_to_check[idx] = false;
                    this->fill_row(idx);
                } else {
                    if (is_col_filled[idx])      continue;
                    if (!col_need_to_check[idx]) continue;
                    col_need_to_check[idx] = false;
                    this->fill_col(idx);
                }

                if (remaining_old != remaining_cells) { // Cells have been updated.
                    remaining_old = remaining_cells;
#ifdef DEBUG
                    cout << (ord[i].line_type == LINE_ROW ? "Row" : "Col") << " "; cout.width(2);
                    cout << ord[i].line_idx << " (i="; cout.width(2); cout << i << ") // ";
                    // this->print();
#endif
                    break;
                }
            }

            if (i == width + height) {
                cout << "\nFAILED TO SOLVE.\n";
                return false;
            }
#ifdef DEBUG
            cout << "STEP #" << step++ << ": " << (total_cnt - remaining_cells) << " ("; cout.precision(1);
            cout << fixed << ((float)(total_cnt - remaining_cells) / total_cnt * 100) << "%)" << "\n";
#endif
        }
        cout << "\nSOLVED!!\n";
        return true;
    }

    void print() const {
        int i, j, k, row_max = 0, col_max = 0;
        
        // Get the maximum of row_hints and col_hints.
        for (i = 0; i < height; i++) { // The length of row_hints is height.
            row_max = max(row_max, row_hint_cnt[i]);
        }
        for (i = 0; i < width; i++) { // The length of col_hints is width.
            col_max = max(col_max, col_hint_cnt[i]);
        }

        // // Print col_hints.
        for (i = 0; i < col_max; i++) {
            for (j = 0; j < row_max; j++) cout << "   "; cout << " | ";
            for (j = 0; j < width; j++) {
                k = i - (col_max - col_hint_cnt[j]);
                if (k >= 0) {
                    cout.width(2);
                    cout << col_hints[j][k];
                } else {
                    cout << "  ";
                }
            }
            cout << "\n";
        }
        
        // Print delimiter.
        for (i = 0; i < row_max; i++) cout << "---"; cout << "-+-";
        for (i = 0; i < width;   i++) cout << "--";  cout << "\n";
        
        // Print row_hints and map.
        for (i = 0; i < height; i++) {
            for (j = 0; j < row_max - row_hint_cnt[i]; j++) {
                cout << "   ";
            }
            for (j = 0; j < row_hint_cnt[i]; j++) {
                cout.width(2);
                cout << row_hints[i][j] << " ";
            }
            cout << " | ";
            for (int j = 0; j < width; j++) {
                if (map[i][j] == CELL_EMPTY) {
                    cout << SYMBOL_EMPTY;
                } else if (map[i][j] == CELL_FILLED) {
                    cout << SYMBOL_FILL;
                } else {
                    cout << SYMBOL_UNKNOWN;
                }
            }
            cout << "\n";
        }
    }

    static Nonogram load_file(const char* file_name) {
        ifstream ifs(file_name);
        if (!ifs) throw "failed to read the file.";

        int width, height;
        ifs >> width >> height;
        Nonogram ng(width, height);

        streambuf* backup;
        backup = cin.rdbuf();
        cin.rdbuf(ifs.rdbuf());
        ng.input();
        cin.rdbuf(backup);

        return ng;
    }

    bool save_file(const char* file_name) const {
        int i, j;
        char buffer[MAX_SIZE * 2 + 2] = {0,};
        ofstream ofs(file_name);
        if (!ofs) return false;

        for (i = 0; i < width * 2; i++) buffer[i] = ' ';
        buffer[i-1] = '\n';
        buffer[i]   = '\0';

        for (i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (map[i][j] == CELL_EMPTY) {
                    buffer[j * 2] = '0';
                } else if (map[i][j] == CELL_FILLED) {
                    buffer[j * 2] = '1';
                } else {
                    buffer[j * 2] = '2';
                }
            }
            ofs << buffer;
        }
        ofs.close();
        return true;
    }
};


int main() {
    int width, height;
    cin >> width >> height;

    Nonogram ng(width, height);
    ng.input();
    ng.solve();
    ng.print();

    return 0;
}
