#include<stdio.h>
#include<iostream>
#include<vector>
#include<string>
#include<string.h>
#include<algorithm>
using namespace std;
/*
bool solution(vector<vector<int>> grid) {
	bool result=true;
	//check if board contain valid value
	for (int i = 0; i<grid.size(); i++) {
		for (int j = 0; j<grid[i].size(); j++) {
			if (grid[i][j]<1 || grid[i][j]>9) result = false;
		}
	}

	//check if row is valid
	vector<int> uniqueCheckRow(9, 0);
	for (int i = 0; i<grid.size(); i++) {
		for (int j = 0; j<grid[i].size(); j++) {
			uniqueCheckRow[grid[i][j] - 1]++;
		}
	}
	for (int i = 0; i<uniqueCheckRow.size(); i++) {
		if (uniqueCheckRow[i] != 1) result = false;
	}

	//check if column is valid
	vector<int> uniqueCheckColumn(9, 0);
	for (int j = 0; j<grid[0].size(); j++) {
		for (int i = 0; i<grid.size(); i++) {
			uniqueCheckColumn[grid[i][j] - 1]++;
		}
	}
	for (int i = 0; i<uniqueCheckColumn.size(); i++) {
		if (uniqueCheckColumn[i] != 1) result = false;
	}

	//check if 3x3 is valid
	vector<int> uniqueCheck3x3(9, 0);
	//i,j stored leftmost corner of 3x3
	for (int i = 0; i<grid.size() - 2; i += 3) {
		for (int j = 0; j<grid[i].size(); j += 3) {
			//check the corresponding 3x3
			for (int k = i; k<i + 3; k++) {
				for (int l = j; l<j + 3; l++) {
					uniqueCheck3x3[grid[k][l] - 1]++;
				}
			}
		}
	}
	for (int i = 0; i<uniqueCheck3x3.size(); i++) {
		if (uniqueCheck3x3[i] != 1) result = false;
	}
	return true;
}
*/


int main() {
	vector<vector<int>> matrix = { {4,5,6},
	{1,2,3 }
};
	sort(matrix.begin(), matrix.end());
	//bool res = solution(matrix);
	return 0;
}