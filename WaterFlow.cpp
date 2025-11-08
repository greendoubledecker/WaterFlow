#include <iostream>
#include <vector>
#include <random>
#include <windows.h>
#include <utility>

using namespace std;

vector<int> water = {100, 100, 255};
vector<int> land = {100, 255, 100};
vector<int> valley = {230, 135, 15};
vector<int> peak = {140, 20, 210};

bool diagFlow = true;

int smoothing_passes = 3;
int bg_smoothing = 20;

bool slopeDir = true; //true for right, false for left;

void setPixel(int row, int col, int r, int g, int b) {
    std::cout << "\x1b[" << row + 1 << ";" << col * 2 + 1 << "H"      // Move cursor
              << "\x1b[48;2;" << r << ";" << g << ";" << b << "m  " // Draw block
              << "\x1b[0m" << std::flush;                // Reset and flush
}

void printColorGrid(vector<vector<int>> grid) { // ints: negative for water, positive for land, height is absolute value(out of 1000)
    // ANSI escape codes format:
    // \x1b[48;2;R;G;Bm to set background color (RGB)
    // \x1b[0m to reset colors

    for (int row = 0; row < grid.size(); ++row) {
        for (int col = 0; col < grid[0].size(); ++col) {
			float gs = grid[row][col] / 1000.0;
            if(grid[row][col] < 0){
				setPixel(row, col, - gs * water[0], - gs * water[1], - gs * water[2]);
			}else{
				setPixel(row, col, gs * land[0], gs * land[1], gs * land[2]);
			}
        }
        cout << "\x1b[0m\n"; // Reset at end of line
    }
}

vector<vector<int>> waterFlow(const vector<vector<int>> prevWater){
	vector<pair<int, int>> shore;
	for(int y = 0; y < prevWater.size(); ++y){
		for(int x = 0; x < prevWater[0].size(); ++x){ // find squares next to water
			if(prevWater[y][x] > 0){
				if(y != 0){
					if(prevWater[y - 1][x] < 0){
						shore.push_back({y, x});
						continue;
					}
					if(x != 0 && diagFlow){
						if(prevWater[y - 1][x - 1] < 0){
							shore.push_back({y, x});
							continue;
						}
					}
					if(x != prevWater[0].size() - 1 && diagFlow){
						if(prevWater[y - 1][x + 1] < 0){
							shore.push_back({y, x});
							continue;
						}
					}
				}
				if(x != 0){
					if(prevWater[y][x - 1] < 0){
						shore.push_back({y, x});
						continue;
					}
				}
				if(x != prevWater[0].size() - 1){
					if(prevWater[y][x + 1] < 0){
						shore.push_back({y, x});
						continue;
					}
				}
				if(y != prevWater.size() - 1){
					if(prevWater[y + 1][x] < 0){
						shore.push_back({y, x});
						continue;
					}
					if(x != 0 && diagFlow){
						if(prevWater[y + 1][x - 1] < 0){
							shore.push_back({y, x});
							continue;
						}
					}
					if(x != prevWater[0].size() - 1 && diagFlow){
						if(prevWater[y + 1][x + 1] < 0){
							shore.push_back({y, x});
							continue;
						}
					}
				}
			}
		}
	}
	
	vector<pair<int, int>> lowestShore;
	int lowest = 1001;
	for(pair<int, int> loc : shore){
		if(prevWater[loc.first][loc.second] < lowest){
			lowestShore.clear();
			lowestShore.push_back(loc);
			lowest = prevWater[loc.first][loc.second];
		}else if(prevWater[loc.first][loc.second] == lowest){
			lowestShore.push_back(loc);
		}
	}
	
	vector<vector<int>> ans = prevWater;
	for(pair<int, int> loc : lowestShore){
		ans[loc.first][loc.second] *= -1;
		float mul = prevWater[loc.first][loc.second] / 1000.0;
		setPixel(loc.first, loc.second, mul * water[0], mul * water[1], mul * water[2]);
	}
	return ans;
}

vector<vector<int>> addVec(vector<vector<int>> v1, vector<vector<int>> v2){
	vector<vector<int>> ans(v1.size(), vector<int>(v1[0].size()));
	for(int i = 0; i < v1.size(); ++i){
		for(int j = 0; j < v1[0].size(); ++j){
			//cout << "in loop" << i << ", " << j << endl;
			ans[i][j] = v1[i][j] + v2[i][j];
		}
	}
	return ans;
}

int main(){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    int width  = (csbi.srWindow.Right - csbi.srWindow.Left + 1) / 2;
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top - 1;
    
    random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(1, 1000);
    
    vector<vector<int>> terrain(height, vector<int>(width));
    vector<vector<int>> bg(height, vector<int>(width));
    vector<vector<int>> slope(height, vector<int>(width));
    vector<vector<int>> waterGrid(height, vector<int>(width)); // y, x, height(negative for water)
    
    for(int y = 0; y < height; ++y){
        for(int x = 0; x < width; ++x){
            terrain[y][x] = dist(rng) / 4;
            bg[y][x] = (dist(rng) - 300);
            float frac = static_cast<float>(x) / width;
            if(slopeDir){
				slope[y][x] = (1 - frac) * 500;
			}else{
				slope[y][x] = frac * 500;
			}
		}
	}
	
	std::cout << "\x1b[2J";
	
	for(int i = 0; i < bg_smoothing; ++i){
		vector<vector<int>> smoothGrid(height, vector<int>(width));
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				double sum = bg[y][x];
				int count = 1;
				if(y > 0){
					sum += bg[y-1][x];
					count++;
				}
				if(y < height - 1){
					sum += bg[y+1][x];
					count++;
				}
				if(x > 0){
					sum += bg[y][x-1];
					count++;
				}
				if(x < width - 1){
					sum += bg[y][x+1];
					count++; 
				}
				smoothGrid[y][x] = sum / count;
			}
		}
		bg = smoothGrid;
	}
	
	if(smoothing_passes > 0){
		for(int i = 0; i < smoothing_passes; ++i){
			vector<vector<int>> smoothGrid(height, vector<int>(width));
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					double sum = terrain[y][x];
					int count = 1;
					if(y > 0){
						sum += terrain[y-1][x];
						count++;
					}
					if(y < height - 1){
						sum += terrain[y+1][x];
						count++;
					}
					if(x > 0){
						sum += terrain[y][x-1];
						count++;
					}
					if(x < width - 1){
						sum += terrain[y][x+1];
						count++; 
					}
					smoothGrid[y][x] = sum / count;
				}
			}
			waterGrid = smoothGrid;
			terrain = smoothGrid;
		}
	}else{
		waterGrid = terrain;
	}
	
	waterGrid = addVec(waterGrid, bg);
	waterGrid = addVec(waterGrid, slope);
	//cout << "after" << endl;
	
	pair<int, int> top, bottom;
	int high = 0;
	int low = 1001;
	for(int y = 0; y < waterGrid.size(); ++y){
		for(int x = 0; x < waterGrid[0].size(); ++x){
			if(waterGrid[y][x] < low){
				low = waterGrid[y][x];
				bottom = {y, x};
			}if(waterGrid[y][x] > high){
				high = waterGrid[y][x];
				top = {y, x};
			}
		}
	}
	cout << high << " " << low << endl;
	waterGrid[top.first][top.second] *= -1;
	
	printColorGrid(waterGrid);
	while(waterGrid[bottom.first][bottom.second] > 0){
		setPixel(top.first, top.second, peak[0], peak[1], peak[2]);
		setPixel(bottom.first, bottom.second, valley[0], valley[1], valley[2]);
		waterGrid = waterFlow(waterGrid);
	}
	setPixel(top.first, top.second, peak[0], peak[1], peak[2]);
	setPixel(bottom.first, bottom.second, valley[0], valley[1], valley[2]);
	cout << "\x1b[0m";
	cout << "\x1b[9999;1H";
	return 0;
}
