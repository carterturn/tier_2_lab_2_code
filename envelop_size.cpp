/*
  Calculate the ratio of max/min for a beat envelop
 */

#include <GLFW/glfw3.h>
#include <FTGL/ftgl.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <thread>
#include <utility>
#include <vector>

#include <fftw3.h>

using namespace std;
using namespace std::chrono;
using std::this_thread::sleep_for;

int fft_r_min;
int fft_r_max;

void check_keys(GLFWwindow * window, int key, int scancode, int action, int mods){
	switch(key){
	case GLFW_KEY_Q:
		fft_r_max-=100;
		break;
	case GLFW_KEY_W:
		fft_r_max-=10;
		break;
	case GLFW_KEY_E:
		fft_r_max+=10;
		break;
	case GLFW_KEY_R:
		fft_r_max+=100;
		break;
	case GLFW_KEY_A:
		fft_r_min-=100;
		break;
	case GLFW_KEY_S:
		fft_r_min-=10;
		break;
	case GLFW_KEY_D:
		fft_r_min+=10;
		break;
	case GLFW_KEY_F:
		fft_r_min+=100;
		break;
	default:
		break;
	}
	return;
}

vector<string> split_string(string data, char splitter){
	vector<string> tokens = vector<string>();
	string tmp = "";
	for(int i = 0; i < data.length(); i++){
		if(data[i] == splitter){
			tokens.push_back(tmp);
			tmp = "";
		}
		else{
			tmp += data[i];
		}
	}
	tokens.push_back(tmp);
	return tokens;
}

int main(int argc, char * argv[]){
	if(argc < 3){
		cout << "I need a filename, start index, and end index\n";
		return -1;
	}
	fstream data_file(argv[1]);

	{
		string a;
		getline(data_file, a); // throw out first line
	}

	vector<double> data;

	int start_idx = atoi(argv[2]);
	int end_idx = atoi(argv[3]);

	int idx = 0;
	while(!data_file.eof()){
		string line;
		getline(data_file, line);
		idx++;
		if(idx < start_idx){
			continue;
		}
		if(idx > end_idx){
			break;
		}
		vector<string> entries = split_string(line, ',');
		if(entries.size() < 5){
			continue;
		}
		data.push_back(strtod(entries[5].c_str(), NULL));
	}

	data_file.close();

	// Preproccessing
	double average = 0.0;
	for(int i = 0; i < data.size(); i++){
		average += data[i];
	}
	average = average / data.size();
	for(int i = 0; i < data.size(); i++){
		data[i] -= average;
	}
	for(int i = 0; i < data.size(); i++){
		data[i] = abs(data[i]);
	}
	for(int k = 0; k < 10; k++){
		vector<double> new_data(data.size());
		for(int i = 1; i < data.size()-1; i++){
			new_data[i] = max(data[i-1], max(data[i], data[i+1]));
		}
		data = new_data;
	}
	sort(data.begin(), data.end());
	int percentile = 10;
	double min_average = 0.0;
	for(int i = 0; i < data.size() / percentile; i++){
		min_average += data[i];
	}
	min_average = min_average / (data.size() / percentile);
	double max_average = 0.0;
	for(int i = data.size() - data.size() / percentile; i < data.size(); i++){
		max_average += data[i];
	}
	max_average = max_average / (data.size() / percentile);
	cout << argv[1] << "\t" <<  "Envelop min: " << min_average << "\tEnvelop max: " << max_average << "\tRatio: " << max_average / min_average << "\n";

	return 0;
}
