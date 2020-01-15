#include <stdio.h>
#include <conio.h>
#include <iostream>
using namespace std;

#define MAP_YES 2
#define MAP_NO  1
#define MAP_UNK 0

int max(int a, int b) { return (a > b) ? a : b; }
void print_map(int* map, int rows, int cols, int* row_hints[], int* col_hints[], int row_hint_cnt[], int col_hint_cnt[]);


bool is_line_right(int line[], int line_len, int hints[], int hint_cnt)
{
	int i, hint_now = 0, cnt = 0;
	
	for (i = 0; i <= line_len; i++) { // 등호 넣은건 line 끝이 채워진 경우도 체크하기 위함 
		if (i != line_len && line[i] == MAP_YES) {
			cnt++;
		} else {
			if (cnt == 0)
				continue;
			if (hint_now == hint_cnt || hints[hint_now] != cnt)
				return false; // 더 이상 힌트가 없는데 채워져 있거나, 힌트와 맞지 않으면 false 
			hint_now++;
			cnt = 0;
		}	
	}
	
	if (hint_now == hint_cnt) // 힌트를 모두 사용한 경우 (올바르게 채워진 경우) 
		return true;

	return false;
}

int calc_line_dup_count(int line[], int line_len, int pos, int hints[], int hint_cnt, int hint_now, int dup_cnt[])
{
	int i, j, k, r = 0, remaining = 0, temp[line_len];
	
	if (hint_now == hint_cnt)
		return is_line_right(line, line_len, hints, hint_cnt);

	for (i = hint_now; i < hint_cnt; i++)
		remaining += hints[i];
	remaining += (hint_cnt - hint_now) - 1;

	for (i = 0; i < line_len; i++)
		temp[i] = line[i];
		
	for (i = pos; i <= line_len - remaining; i++) {
		for (j = 0; j < hints[hint_now]; j++)
			if (line[i + j] == MAP_NO)
				break;
				
		if (j != hints[hint_now])
			continue; // 채우려는 칸 중에 NO가 있으면 넘어감 
		if (i + j != line_len && line[i + j] == MAP_YES) // for optimization
			continue; // 다음 칸이 YES이면 힌트 길이보다 1 커지므로 볼 필요 없음 
		
		for (j = 0; j < hints[hint_now]; j++)
			temp[i + j] = MAP_YES; // 채우기 
		
		k = calc_line_dup_count(temp, line_len, i + hints[hint_now] + 1, hints, hint_cnt, hint_now + 1, dup_cnt);
		
		for (j = 0; j < hints[hint_now]; j++) {
			temp[i + j] = line[i + j]; // 복구 
			dup_cnt[i + j] += k; // 중복 횟수
		}
		
		r += k;
	}
	
	return r;
}


bool fill_line(int line[], int line_len, int hints[], int hint_cnt)
{
	int i, dup_max = 0, dup_cnt[line_len];
	
	// 모든 칸이 비워지거나 채워진경우 (힌트로 바로 알 수 있는 경우)
	// 힌트가 0 하나만 있는 경우는 체크해야 함(비워진 경우), 하지만 모두 채워진 경우는 필수 아님 
	if (hint_cnt == 1 && (hints[0] == 0 || hints[0] == line_len)) { 
		for (i = 0; i < line_len; i++)
			line[i] = (hints[0] == 0 ? MAP_NO : MAP_YES);
		return true;
	}
	
	for (i = 0; i < line_len; i++)
		dup_cnt[i] = 0; // 중복 횟수 초기화 
	
	// 모든 경우를 시도 -> 각 칸의 중복 횟수를 dup_cnt에 저장, 최대 중복 횟수를 반환 
	dup_max = calc_line_dup_count(line, line_len, 0, hints, hint_cnt, 0, dup_cnt);

	for (i = 0; i < line_len; i++) {
		if (dup_cnt[i] == 0) // 항상 비워지는 칸이 있다면 비움 (dup_max가 0일 수 있으므로 비워지는 것 부터 체크) 
			line[i] = MAP_NO;
		else if (dup_cnt[i] == dup_max) // 항상 채워지는 칸이 있다면 채움 
			line[i] = MAP_YES;
	}
	
	// 힌트를 모두 사용해서 모두 정확히 채운 경우 -> 아직 안 채워진 공간을 모두 NO로 채움 
	if (is_line_right(line, line_len, hints, hint_cnt)) { 
		for (i = 0; i < line_len; i++)
			line[i] = (line[i] == MAP_YES ? MAP_YES : MAP_NO);
		return true; // 빈 칸 없이 모두 채운 경우 
	}
	
	return false; // 빈 칸이 남은 경우 
}


void solve(int* map, int row_len, int col_len, int* row_hints[], int* col_hints[], int row_hint_cnt[], int col_hint_cnt[])
{
	int i, j, tmp[col_len], not_fill_cnt = row_len + col_len;
	bool is_row_filled[col_len], is_col_filled[row_len];
	
	for (i = 0; i < col_len; i++) is_row_filled[i] = false;
	for (i = 0; i < row_len; i++) is_col_filled[i] = false;
	
	while(not_fill_cnt) {
		for (i = 0; i < col_len; i++) { // fill rows
			if (is_row_filled[i]) continue;
			
			if (fill_line(&map[row_len * i], row_len, row_hints[i], row_hint_cnt[i]))
				is_row_filled[i] = true, not_fill_cnt--;
		}

		for (i = 0; i < row_len; i++) { // fill cols
			if (is_col_filled[i]) continue;
			
			for (j = 0; j < col_len; j++) // col -> line
				tmp[j] = map[row_len * j + i];
				
			if (fill_line(tmp, col_len, col_hints[i], col_hint_cnt[i]))
				is_col_filled[i] = true, not_fill_cnt--;
			
			for (j = 0; j < col_len; j++) // line -> col
				map[row_len * j + i] = tmp[j];
		}
	}
}



int main()
{
	int i, j, k;

	int row_len, col_len;
	cin >> row_len >> col_len;
	
	int map[col_len][row_len];
	for (i = 0; i < col_len; i++)
		for (j = 0; j < row_len; j++)
			map[i][j] = MAP_UNK;


	int* row_hints[col_len];
	int  row_hint_cnt[col_len];	
	for (i = 0; i < col_len; i++) {
		cin >> row_hint_cnt[i];
		row_hints[i] = new int[row_hint_cnt[i]];
		for (j = 0; j < row_hint_cnt[i]; j++)
			cin >> row_hints[i][j];
	}
	
	int* col_hints[row_len];
	int  col_hint_cnt[row_len];	
	for (i = 0; i < row_len; i++) {
		cin >> col_hint_cnt[i];
		col_hints[i] = new int[col_hint_cnt[i]];
		for (j = 0; j < col_hint_cnt[i]; j++)
			cin >> col_hints[i][j];
	}
	
	
	solve(    (int*)map, row_len, col_len, row_hints, col_hints, row_hint_cnt, col_hint_cnt);
	print_map((int*)map, row_len, col_len, row_hints, col_hints, row_hint_cnt, col_hint_cnt);
	
	
	for (i = 0; i < col_len; i ++) delete[] row_hints[i];
	for (i = 0; i < row_len; i ++) delete[] col_hints[i];

	
	return 0;
}




void print_map(int* map, int row_len, int col_len, int* row_hints[], int* col_hints[], int row_hint_cnt[], int col_hint_cnt[])
{
	int i, j, k, row_max = 0, col_max = 0;
	
	// row_hints와 col_hints의 최대 개수 얻기 
	for (i = 0; i < col_len; i++) // row_hints는 col_len개 만큼 있음 
		row_max = max(row_max, row_hint_cnt[i]);
	for (i = 0; i < row_len; i++)
		col_max = max(col_max, col_hint_cnt[i]);
	
	for (i = 0; i < col_max; i++) // print col_hints
	{
		for (j = 0; j < row_max; j++)
			printf("   ");
		printf(" | ");
			
		for (j = 0; j < row_len; j++) // col_hints는 row_len개 만큼 있음 
		{
			k = i - (col_max - col_hint_cnt[j]);
			if (k >= 0)
				printf("%2d", col_hints[j][k]);
			else
				printf("  ");
		}
		printf("\n");
	}
	
	for (i = 0; i < row_max; i++) printf("---");
	printf("-+-");
	for (i = 0; i < row_len; i++) printf("--");
	printf("\n");
	
	for (i = 0; i < col_len; i++) // print row_hints
	{
		for (j = 0; j < row_max - row_hint_cnt[i]; j++)
			printf("   ");
		for (j = 0; j < row_hint_cnt[i]; j++)
			printf("%2d ", row_hints[i][j]);
		printf(" | ");
		
		for (int j = 0; j < row_len; j++)
		{
			switch (map[i * row_len + j])
			{
			case MAP_YES:
				printf("●");
				break;
			case MAP_NO:
				printf("○");
				break;
			default:
				printf("？");
			}
		}
		printf("\n");
	}
}
