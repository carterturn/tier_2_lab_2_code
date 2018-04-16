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

using namespace std; // I am a monster, I believe
using namespace std::chrono;
using std::this_thread::sleep_for;

int fft_r_min;
int fft_r_max;
int cursor;

void check_keys(GLFWwindow * window, int key, int scancode, int action, int mods){
	switch(key){
	case GLFW_KEY_Q:
		fft_r_max-=500;
		break;
	case GLFW_KEY_W:
		fft_r_max-=50;
		break;
	case GLFW_KEY_E:
		fft_r_max+=50;
		break;
	case GLFW_KEY_R:
		fft_r_max+=500;
		break;
	case GLFW_KEY_A:
		fft_r_min-=500;
		break;
	case GLFW_KEY_S:
		fft_r_min-=50;
		break;
	case GLFW_KEY_D:
		fft_r_min+=50;
		break;
	case GLFW_KEY_F:
		fft_r_min+=500;
		break;
	case GLFW_KEY_Z:
		cursor-=500;
		break;
	case GLFW_KEY_X:
		cursor-=50;
		break;
	case GLFW_KEY_C:
		cursor+=50;
		break;
	case GLFW_KEY_V:
		cursor+=500;
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

	double data_y_min;
	double data_y_max;
	double data_y_avg;
	{ // Remove DC offset (cause we don't even care)
		double raw_y_min = *min_element(data.begin(), data.end());
		double raw_y_max = *max_element(data.begin(), data.end());
		data_y_avg = (raw_y_max + raw_y_min) / 2.0;
		data_y_min = - (raw_y_max - raw_y_min) / 2.0;
		data_y_max = (raw_y_max - raw_y_min) / 2.0;
	}

	fft_r_min = floor(double(data.size()) * 1200.0 / 4800.0);
	fft_r_max = ceil(double(data.size()) * 1600.0 / 4800.0);

	cout << fft_r_min << "\t" << fft_r_max << "\t" << data.size() << "\n";

	glfwMakeContextCurrent(fft_window);
	glfwSetKeyCallback(fft_window, check_keys);
	glfwSetInputMode(fft_window, GLFW_STICKY_KEYS, 1);
	glfwMakeContextCurrent(data_window);
	glfwSetKeyCallback(data_window, check_keys);
	glfwSetInputMode(data_window, GLFW_STICKY_KEYS, 1);

	while(true){
		glfwMakeContextCurrent(fft_window);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, (end_idx - start_idx) / 2, fft_y_min*1000, fft_y_max*1000, -1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_POINTS);
		for(int i = 0; i < data.size()/2; i++){
			glVertex2i(i, fft[i]*1000.0);
		}
		glEnd();

		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex2i(fft_r_min, fft_y_min*1000);
		glVertex2i(fft_r_min, fft_y_max*1000);
		glVertex2i(fft_r_max, fft_y_min*1000);
		glVertex2i(fft_r_max, fft_y_max*1000);
		glEnd();

		glfwSwapBuffers(fft_window);

		double * partial_fft = new double[data.size()];
		fill(partial_fft, partial_fft+data.size(), 0);
		for(int i = max(0, fft_r_min); i < min((int) data.size() / 2, fft_r_max); i++){
			partial_fft[i] = fft[i];
			partial_fft[data.size() / 2 - i] = fft[data.size() / 2 - i];
		}
		
		double * reconstructed = new double[data.size()];
		fftw_plan rfft_plan = fftw_plan_r2r_1d(data.size(), partial_fft, reconstructed, FFTW_HC2R,
											   FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);

		fftw_execute(rfft_plan);

		fftw_destroy_plan(rfft_plan);

		glfwMakeContextCurrent(data_window);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, data.size(), data_y_min*1000, data_y_max*1000, -1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		double * reconstructed_intensity = new double[data.size()];
		for(int i = 2; i < data.size()-2; i++){
			reconstructed_intensity[i] = max(abs(reconstructed[i]),
											 max(max(abs(reconstructed[i-1]), abs(reconstructed[i+1])),
												 max(abs(reconstructed[i-2]), abs(reconstructed[i+2]))));
		}

		glBegin(GL_POINTS);
		for(int i = 0; i < data.size(); i++){
			//			glColor3f(0.0f, 1.0f, 0.0f);
			//			glVertex2i(i, (data[i] - data_y_avg) * 1000.0);
			//			glColor3f(1.0f, 1.0f, 1.0f);
			//			glVertex2i(i, (reconstructed[i]) * 1000.0 / ((double) data.size()));
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2i(i, (reconstructed_intensity[i]) * 1000.0 / ((double) data.size()));
		}
		glEnd();

		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES);
		glVertex2i(cursor, data_y_min*1000);
		glVertex2i(cursor, data_y_max*1000);
		glEnd();
		cout << double(cursor) * 0.0002 << "\t" << reconstructed_intensity[cursor] << "\n";

		delete[] partial_fft;
		delete[] reconstructed;
		delete[] reconstructed_intensity;

		glfwSwapBuffers(data_window);

		glfwPollEvents();

		sleep_for(milliseconds(25));
	}

	glfwTerminate();
}

