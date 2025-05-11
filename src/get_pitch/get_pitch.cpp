/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <cmath> 
#include <algorithm>
#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    --llindar-rmax FLOAT  llindar de decisio sonor o sord per al maxim secundari de la autocorrelacio [default: 0.41]
    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

void CenterClipping(vector<float>& signal) {
    float max = fabs(signal[0]);
    for (size_t i = 1; i < signal.size(); ++i) {
        max = std::max(max, fabs(signal[i]));
    }

    float th = max * 0.08;
    for (size_t i = 0; i < signal.size(); ++i) {
        if (fabs(signal[i]) <= th) {
            signal[i] = 0.0f;
        }
    }
}

void ordenar(std::vector<float>& window) {
    for (size_t i = 0; i < window.size() - 1; ++i) {
        for (size_t j = 0; j < window.size() - 1 - i; ++j) {
            if (window[j] < window[j + 1]) {
                std::swap(window[j], window[j + 1]);
            }
        }
    }
}

void MedianFilter(std::vector<float>& signal) {
    int w_size = 5;
    int half_w = (w_size + 1) / 2;

    for (size_t i = 0; i <= signal.size() - w_size; i += w_size) {
        std::vector<float> window(signal.begin() + i, signal.begin() + i + w_size);
        ordenar(window);
        float mediana = window[half_w - 1];
        for (int j = 0; j < w_size; ++j) {
            signal[i + j] = mediana;
        }
    }
}

int main(int argc, const char *argv[]) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},
        true,
        "2.0");

    std::string input_wav = args["<input-wav>"].asString();
    std::string output_txt = args["<output-txt>"].asString();
    float llindar_rmax = stof(args["--llindar-rmax"].asString());

    unsigned int rate;
    vector<float> x;
    if (readwav_mono(input_wav, rate, x) != 0) { 
        cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
        return -2;
    }

    int n_len = rate * FRAME_LEN;
    int n_shift = rate * FRAME_SHIFT;

    PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::RECT, 50, 500, llindar_rmax);

    CenterClipping(x);

    vector<float>::iterator iX;
    vector<float> f0;
    for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
        float f = analyzer(iX, iX + n_len);
        f0.push_back(f);
    }

    MedianFilter(x);

    std::ofstream file("mediana.txt");
    for (const float& value : x) {
        file << value << std::endl;
    }
    file.close();

    ofstream os(output_txt);
    if (!os.good()) {
        cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
        return -3;
    }

    os << 0 << '\n';
    for (const float& val : f0) 
        os << val << '\n';
    os << 0 << '\n';

    return 0;
}
