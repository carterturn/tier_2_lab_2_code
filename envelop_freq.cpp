/*
  Calculate the frequency of a beat envelop
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

	glfwInit();
	GLFWwindow * fft_window = glfwCreateWindow(1280, 480, "FFT", NULL, NULL);
	GLFWwindow * data_window = glfwCreateWindow(1280, 480, "DATA", NULL, NULL);

	double * fft = new double[data.size()];
	fftw_plan ffft_plan = fftw_plan_r2r_1d(data.size(), data.data(), fft, FFTW_R2HC,
											  FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);

	fftw_execute(ffft_plan);

	fftw_destroy_plan(ffft_plan);

	vector<double> fft_vector;
	fft_vector.push_back(abs(fft[0]));
	for(int i = 1; i < data.size() / 2; i++){
		fft_vector.push_back(sqrt(fft[i]*fft[i] + fft[data.size()/2 - i]*fft[data.size()/2 - i]));
	}
	double fft_y_min = 0;
	double fft_y_max = 100.0;

	cout << 0 << "\t" << (end_idx - start_idx) / 2 << "\n";

	fft[0] = 0;

	for(int i = 0; i < data.size()/2; i+=4){
		double bin = 0.0;
		for(int j = 0; j < 4; j++){
			bin += abs(fft[i]);
		}
		cout << double(i) * 4800.0 / double(data.size()) << "\t" << bin << "\n";
	}
}

