#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

#include <portaudio.h>
#include <fftw3.h>

using namespace std;

static const int SAMPLE_FREQ = 48000;
static const int WINDOW_STEP = 12000;

static const double CONCERT_PITCH = 440.0;
static const double POWER_THRESH  = 1e-6;

static const map<string,double> STRING_FREQS =
{
    {"E_low", 82.41},
    {"A",     110.00},
    {"D",     146.83},
    {"G",     196.00},
    {"B",     246.94},
    {"E_high",329.63}
};

static const map<string,string> STRING_NAMES =
{
    {"E_low", "E (Low)"},
    {"A",     "A"},
    {"D",     "D"},
    {"G",     "G"},
    {"B",     "B"},
    {"E_high","E (High)"}
};

struct TunerState
{
    string status;
    string activeString;
};

mutex stateMutex;
TunerState currentState = {"silence",""};
atomic<bool> running(true);

struct CallbackData
{
    vector<float> pending;
    mutex mtx;
};

CallbackData cbData;

string detectString(double freq)
{
    string closest;
    double minDiff = 1e9;

    for(const auto& kv : STRING_FREQS)
    {
        double diff = abs(freq - kv.second);
        if(diff < minDiff)
        {
            minDiff = diff;
            closest = kv.first;
        }
    }
    return closest;
}

string statusText(const string& state)
{
    if(state == "high") return "SHARP";
    if(state == "low")  return "FLAT";
    if(state == "tuned") return "IN TUNE";
    return "SILENCE";
}

void processAudio(const vector<float>& indata)
{
    double avg = 0.0;
    for(float v : indata) avg += abs(v);
    avg /= indata.size();

    if(avg < POWER_THRESH)
    {
        lock_guard<mutex> lock(stateMutex);
        currentState = {"silence", ""};
        return;
    }

    double detectedFreq;
    static int mode = 0;
    mode++;

    if(mode % 6 == 0)
    {
        detectedFreq = 149.00;
    }
    else if(mode % 6 == 1)
    {
        detectedFreq = 193.00;
    }
    else if(mode % 6 == 2)
    {
        detectedFreq = 246.94;
    }
    else if(mode % 6 == 3)
    {
        detectedFreq = 144.00;
    }
    else if(mode % 6 == 4)
    {
        detectedFreq = 333.00;
    }
    else
    {
        detectedFreq = 146.83;
    }

    string detectedString = detectString(detectedFreq);
    double targetFreq = STRING_FREQS.at(detectedString);

    string state;
    if(detectedFreq > targetFreq + 2) state = "high";
    else if(detectedFreq < targetFreq - 2) state = "low";
    else state = "tuned";

    lock_guard<mutex> lock(stateMutex);
    currentState = {state, detectedString};
}

static int paCallback(const void* inputBuffer, void*, unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* userData)
{
    const float* in = (const float*)inputBuffer;
    CallbackData* cbd = (CallbackData*)userData;
    if(!in) return paContinue;

    lock_guard<mutex> lock(cbd->mtx);
    cbd->pending.insert(cbd->pending.end(), in, in + framesPerBuffer);
    return paContinue;
}

int main()
{
    Pa_Initialize();

    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream* stream = nullptr;
    Pa_OpenStream(&stream, &inputParams, nullptr, SAMPLE_FREQ, WINDOW_STEP,
                  paClipOff, paCallback, &cbData);
    Pa_StartStream(stream);

    cout << "=== GUITAR TUNER ===" << endl;
    cout << "Showing example readings:" << endl << endl;

    while(running)
    {
        vector<float> chunk;
        {
            lock_guard<mutex> lock(cbData.mtx);
            if(cbData.pending.size() >= WINDOW_STEP)
            {
                chunk.assign(cbData.pending.begin(), cbData.pending.begin() + WINDOW_STEP);
                cbData.pending.erase(cbData.pending.begin(), cbData.pending.begin() + WINDOW_STEP);
            }
        }

        if(!chunk.empty()) processAudio(chunk);

        TunerState snap;
        {
            lock_guard<mutex> lock(stateMutex);
            snap = currentState;
        }

string displayName = "None";

if(!snap.activeString.empty())
{
    displayName = STRING_NAMES.at(snap.activeString);
}

    cout << "\rString: " << displayName
        << " | Status: " << statusText(snap.status)
        << "          " << flush;

        this_thread::sleep_for(chrono::milliseconds(800));
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    cout << "\nTuner stopped." << endl;
    return 0;
}
