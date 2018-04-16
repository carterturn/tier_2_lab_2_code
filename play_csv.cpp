#include <pulse/simple.h>
#include <pulse/error.h>

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

	int fft_r_min = floor(double(data.size()) * 1200.0 / 4800.0);
	int fft_r_max = ceil(double(data.size()) * 1600.0 / 4800.0);

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


	vector<int16_t> int_data(data.size());
	for(int i = 0; i < data.size(); i++){
		int_data[i] = reconstructed[i] * 6500.0 / ((double) data.size());
	}

	pa_simple * write;

	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16NE;
	ss.channels = 1;
	ss.rate = 4800;
	int error;
	write = pa_simple_new(NULL, "CSV AUDIO", PA_STREAM_PLAYBACK, NULL, "Playback", &ss, NULL, NULL, &error);

	pa_simple_write(write, int_data.data(), int_data.size(), &error);

	return 0;
}
