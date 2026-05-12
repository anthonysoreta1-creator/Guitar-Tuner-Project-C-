#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

#include <SFML/Graphics.hpp>

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

static const vector<string> ALL_NOTES =
{
    "A","A#","B","C","C#","D",
    "D#","E","F","F#","G","G#"
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
    if(state == "high")
        return "SHARP";

    if(state == "low")
        return "FLAT";

    if(state == "tuned")
        return "IN TUNE";

    return "Listening...";
}

void processAudio(const vector<float>& indata)
{
    double avg = 0.0;

    for(float v : indata)
        avg += abs(v);

    avg /= indata.size();

    if(avg < POWER_THRESH)
    {
        lock_guard<mutex> lock(stateMutex);

        currentState =
        {
            "silence",
            ""
        };

        return;
    }

    double fakeFreq = 329.63;

    string detectedString =
        detectString(fakeFreq);

    double target =
        STRING_FREQS.at(detectedString);

    string state;

    if(fakeFreq > target + 2)
        state = "high";

    else if(fakeFreq < target - 2)
        state = "low";

    else
        state = "tuned";

    lock_guard<mutex> lock(stateMutex);

    currentState =
    {
        state,
        detectedString
    };
}

static int paCallback(
    const void* inputBuffer,
    void*,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo*,
    PaStreamCallbackFlags,
    void* userData
)
{
    const float* in = (const float*)inputBuffer;

    CallbackData* cbd =
        (CallbackData*)userData;

    if(!in)
        return paContinue;

    lock_guard<mutex> lock(cbd->mtx);

    cbd->pending.insert(
        cbd->pending.end(),
        in,
        in + framesPerBuffer
    );

    return paContinue;
}

void renderUI(
    sf::RenderWindow& window,
    const TunerState& state
)
{
    window.clear(sf::Color(5,5,15));

    static sf::Font font;
    static bool fontLoaded = false;

    if(!fontLoaded)
    {
        font.loadFromFile("arial.ttf");
        fontLoaded = true;
    }

    static sf::Texture guitarTexture;
    static bool textureLoaded = false;

    if(!textureLoaded)
    {
        guitarTexture.loadFromFile("Guitar.png");
        textureLoaded = true;
    }

    sf::Sprite guitarSprite;

    guitarSprite.setTexture(guitarTexture);

    guitarSprite.setScale(0.5f,0.5f);

    guitarSprite.setPosition(300,180);

    sf::Text title;

    title.setFont(font);

    title.setString("Guitar Tuner");

    title.setCharacterSize(35);

    title.setFillColor(sf::Color::Green);

    title.setPosition(320,40);

    sf::Text status;

    status.setFont(font);

    status.setString(
        statusText(state.status)
    );

    status.setCharacterSize(40);

    if(state.status == "high")
        status.setFillColor(sf::Color::Red);

    else if(state.status == "low")
        status.setFillColor(sf::Color::Yellow);

    else
        status.setFillColor(sf::Color::Green);

    status.setPosition(320,600);

    string leftLabels[3] =
    {
        "D","A","E"
    };

    string rightLabels[3] =
    {
        "G","B","E"
    };

    string leftKeys[3] =
    {
        "D","A","E_low"
    };

    string rightKeys[3] =
    {
        "G","B","E_high"
    };

    float leftY[3] =
    {
        180,320,460
    };

    float rightY[3] =
    {
        180,320,460
    };

    for(int i=0;i<3;i++)
    {
        sf::CircleShape leftCircle(45);

        leftCircle.setPosition(
            80,
            leftY[i]
        );

        leftCircle.setFillColor(
            sf::Color(220,220,220)
        );

        if(state.activeString == leftKeys[i])
        {
            leftCircle.setOutlineThickness(5);

            leftCircle.setOutlineColor(
                sf::Color::Yellow
            );
        }

        sf::CircleShape rightCircle(45);

        rightCircle.setPosition(
            720,
            rightY[i]
        );

        rightCircle.setFillColor(
            sf::Color(220,220,220)
        );

        if(state.activeString == rightKeys[i])
        {
            rightCircle.setOutlineThickness(5);

            rightCircle.setOutlineColor(
                sf::Color::Yellow
            );
        }

        sf::Text leftText;

        leftText.setFont(font);

        leftText.setString(
            leftLabels[i]
        );

        leftText.setCharacterSize(35);

        leftText.setFillColor(
            sf::Color::White
        );

        leftText.setPosition(
            112,
            leftY[i] + 18
        );

        sf::Text rightText;

        rightText.setFont(font);

        rightText.setString(
            rightLabels[i]
        );

        rightText.setCharacterSize(35);

        rightText.setFillColor(
            sf::Color::White
        );

        rightText.setPosition(
            752,
            rightY[i] + 18
        );

        window.draw(leftCircle);

        window.draw(rightCircle);

        window.draw(leftText);

        window.draw(rightText);
    }

    window.draw(title);

    window.draw(guitarSprite);

    window.draw(status);

    window.display();
}

int main()
{
    sf::RenderWindow window(
        sf::VideoMode(900,700),
        "Guitar Tuner"
    );

    Pa_Initialize();

    PaStreamParameters inputParams;

    inputParams.device =
        Pa_GetDefaultInputDevice();

    inputParams.channelCount = 1;

    inputParams.sampleFormat =
        paFloat32;

    inputParams.suggestedLatency =
        Pa_GetDeviceInfo(
            inputParams.device
        )->defaultLowInputLatency;

    inputParams.hostApiSpecificStreamInfo =
        nullptr;

    PaStream* stream = nullptr;

    Pa_OpenStream(
        &stream,
        &inputParams,
        nullptr,
        SAMPLE_FREQ,
        WINDOW_STEP,
        paClipOff,
        paCallback,
        &cbData
    );

    Pa_StartStream(stream);

    while(window.isOpen())
    {
        sf::Event event;

        while(window.pollEvent(event))
        {
            if(event.type ==
               sf::Event::Closed)
            {
                window.close();

                running = false;
            }
        }

        vector<float> chunk;

        {
            lock_guard<mutex> lock(
                cbData.mtx
            );

            if(
                cbData.pending.size()
                >= WINDOW_STEP
            )
            {
                chunk.assign(
                    cbData.pending.begin(),
                    cbData.pending.begin()
                    + WINDOW_STEP
                );

                cbData.pending.erase(
                    cbData.pending.begin(),
                    cbData.pending.begin()
                    + WINDOW_STEP
                );
            }
        }

        if(!chunk.empty())
        {
            processAudio(chunk);
        }

        TunerState snap;

        {
            lock_guard<mutex> lock(
                stateMutex
            );

            snap = currentState;
        }

        renderUI(window,snap);

        this_thread::sleep_for(
            chrono::milliseconds(30)
        );
    }

    Pa_StopStream(stream);

    Pa_CloseStream(stream);

    Pa_Terminate();

    return 0;
}
