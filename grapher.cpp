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

using namespace std; // I am a monster, I believe
using namespace std::chrono;
using std::this_thread::sleep_for;

vector<double> data;
vector<double>::iterator data_start, data_stop;

void check_keys(GLFWwindow * window, int key, int scancode, int action, int mods){
	switch(key){
	case GLFW_KEY_Q:
		data_start-=100;
		break;
	case GLFW_KEY_W:
		data_start-=10;
		break;
	case GLFW_KEY_E:
		data_start+=10;
		break;
	case GLFW_KEY_R:
		data_start+=100;
		break;
	case GLFW_KEY_A:
		data_stop-=100;
		break;
	case GLFW_KEY_S:
		data_stop-=10;
		break;
	case GLFW_KEY_D:
		data_stop+=10;
		break;
	case GLFW_KEY_F:
		data_stop+=100;
		break;
	default:
		break;
	}
	cout << "DATA START: " << distance(data.begin(), data_start) << "\t"
		 << "DATA END: " << distance(data.begin(), data_stop) << "\n";
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
		cout << "Give me a file and a y axis column\n";
		return -1;
	}

	fstream data_file(argv[1]);

	string a;
	getline(data_file, a); // throw out first line

	int y_axis_col_idx = atoi(argv[2]);

	while(!data_file.eof()){
		string line;
		getline(data_file, line);
		vector<string> entries = split_string(line, ',');
		if(entries.size() < y_axis_col_idx){
			continue;
		}
		data.push_back(strtod(entries[y_axis_col_idx].c_str(), NULL));
	}

	data_file.close();

	data_start = data.begin();
	data_stop = data.end();

	glfwInit();
	GLFWwindow * window = glfwCreateWindow(1280, 480, "Grapher", NULL, NULL);

	if(!window){
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, check_keys);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	while(true){
		double y_min = *min_element(data_start, data_stop,
									[](double p, double q){return p < q;});
		double y_max = *max_element(data_start, data_stop,
									[](double p, double q){return p < q;});

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(distance(data.begin(), data_start), distance(data.begin(), data_stop),
				y_min*1000, y_max*1000, -1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_POINTS);
		for(vector<double>::iterator i = data_start; i != data_stop; i++){
			glVertex2i(distance(data.begin(), i), (*i)*1000.0);
		}
		glEnd();

		glfwSwapBuffers(window);
		glfwPollEvents();

		sleep_for(milliseconds(5));
	}

	glfwTerminate();
}
