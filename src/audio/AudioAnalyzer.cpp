#include "AudioAnalyzer.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace fs = std::filesystem;

namespace {
struct WavData {
    int sampleRate = 0;
    int channels = 0;
    std::vector<float> samples;
};

bool ReadChunkHeader(std::ifstream& file, char id[4], std::uint32_t& size) {
    if (!file.read(id, 4)) {
        return false;
    }
    if (!file.read(reinterpret_cast<char*>(&size), sizeof(size))) {
        return false;
    }
    return true;
}

bool LoadWavMono(const fs::path& path, WavData& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    char riff[4] = {};
    std::uint32_t riffSize = 0;
    char wave[4] = {};
    if (!file.read(riff, 4) || !file.read(reinterpret_cast<char*>(&riffSize), sizeof(riffSize)) ||
        !file.read(wave, 4)) {
        return false;
    }
    if (std::string(riff, 4) != "RIFF" || std::string(wave, 4) != "WAVE") {
        return false;
    }

    std::uint16_t audioFormat = 0;
    std::uint16_t numChannels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;
    std::uint32_t dataSize = 0;
    std::streampos dataPos = 0;
    bool foundFmt = false;
    bool foundData = false;

    while (file && (!foundFmt || !foundData)) {
        char chunkId[4] = {};
        std::uint32_t chunkSize = 0;
        if (!ReadChunkHeader(file, chunkId, chunkSize)) {
            break;
        }

        std::string id(chunkId, 4);
        if (id == "fmt ") {
            foundFmt = true;
            std::uint16_t blockAlign = 0;
            std::uint32_t byteRate = 0;
            file.read(reinterpret_cast<char*>(&audioFormat), sizeof(audioFormat));
            file.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));
            file.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
            file.read(reinterpret_cast<char*>(&byteRate), sizeof(byteRate));
            file.read(reinterpret_cast<char*>(&blockAlign), sizeof(blockAlign));
            file.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));
            if (chunkSize > 16) {
                file.seekg(chunkSize - 16, std::ios::cur);
            }
        } else if (id == "data") {
            foundData = true;
            dataSize = chunkSize;
            dataPos = file.tellg();
            file.seekg(chunkSize, std::ios::cur);
        } else {
            file.seekg(chunkSize, std::ios::cur);
        }

        if (chunkSize % 2 == 1) {
            file.seekg(1, std::ios::cur);
        }
    }

    bool pcm = (audioFormat == 1);
    bool floatPcm = (audioFormat == 3);
    bool supportedBits = (bitsPerSample == 8 || bitsPerSample == 16 || bitsPerSample == 24 || bitsPerSample == 32);
    if (!foundFmt || !foundData || numChannels == 0 || sampleRate == 0 || dataSize == 0 ||
        (!pcm && !floatPcm) || (pcm && !supportedBits) || (floatPcm && bitsPerSample != 32)) {
        std::cout << "Unsupported WAV format: fmt=" << audioFormat
                  << " bits=" << bitsPerSample
                  << " channels=" << numChannels
                  << " sampleRate=" << sampleRate
                  << std::endl;
        return false;
    }

    file.clear();
    file.seekg(dataPos);

    const std::size_t bytesPerSample = bitsPerSample / 8;
    const std::size_t frameSize = bytesPerSample * numChannels;
    const std::size_t totalFrames = dataSize / frameSize;

    out.samples.resize(totalFrames);
    out.sampleRate = static_cast<int>(sampleRate);
    out.channels = static_cast<int>(numChannels);

    for (std::size_t i = 0; i < totalFrames; ++i) {
        double sum = 0.0;
        for (std::size_t ch = 0; ch < numChannels; ++ch) {
            float sample = 0.0f;
            if (pcm) {
                if (bitsPerSample == 8) {
                    std::uint8_t raw = 0;
                    file.read(reinterpret_cast<char*>(&raw), sizeof(raw));
                    sample = (static_cast<int>(raw) - 128) / 128.0f;
                } else if (bitsPerSample == 16) {
                    std::int16_t raw = 0;
                    file.read(reinterpret_cast<char*>(&raw), sizeof(raw));
                    sample = static_cast<float>(raw) / 32768.0f;
                } else if (bitsPerSample == 24) {
                    unsigned char bytes[3] = {};
                    file.read(reinterpret_cast<char*>(bytes), 3);
                    std::int32_t raw = (static_cast<std::int32_t>(bytes[2]) << 16) |
                                       (static_cast<std::int32_t>(bytes[1]) << 8) |
                                       static_cast<std::int32_t>(bytes[0]);
                    if (raw & 0x800000) {
                        raw |= 0xFF000000;
                    }
                    sample = static_cast<float>(raw) / 8388608.0f;
                } else if (bitsPerSample == 32) {
                    std::int32_t raw = 0;
                    file.read(reinterpret_cast<char*>(&raw), sizeof(raw));
                    sample = static_cast<float>(raw) / 2147483648.0f;
                }
            } else {
                float raw = 0.0f;
                file.read(reinterpret_cast<char*>(&raw), sizeof(raw));
                sample = std::clamp(raw, -1.0f, 1.0f);
            }
            sum += sample;
        }
        out.samples[i] = static_cast<float>(sum / static_cast<double>(numChannels));
    }

    return true;
}

float ComputeMedian(std::vector<float> values) {
    if (values.empty()) {
        return 0.0f;
    }
    std::size_t mid = values.size() / 2;
    std::nth_element(values.begin(), values.begin() + mid, values.end());
    float median = values[mid];
    if (values.size() % 2 == 0) {
        std::nth_element(values.begin(), values.begin() + mid - 1, values.end());
        median = 0.5f * (median + values[mid - 1]);
    }
    return median;
}

std::string ResolveMusicFolder(const std::string& folderName) {
    fs::path base = fs::current_path();
#if defined(_WIN32)
    char buffer[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, buffer, MAX_PATH) > 0) {
        base = fs::path(buffer).parent_path();
    }
#endif
    fs::path direct = base / folderName;
    if (fs::exists(direct)) {
        return direct.string();
    }
    fs::path parent = base.parent_path() / folderName;
    if (fs::exists(parent)) {
        return parent.string();
    }
    return direct.string();
}
} // namespace

AudioAnalyzer::AudioAnalyzer()
    : m_useSimulation(true)
    , m_simulatedBPM(120.0f)
    , m_simulationTime(0.0f)
    , m_lastBeatTime(0.0f)
    , m_musicFolder("music")
    , m_selectedTrackIndex(static_cast<size_t>(-1))
    , m_playbackActive(false)
    , m_playbackTime(0.0f)
    , m_prevPlaybackTime(0.0f)
    , m_nextBeatIndex(0)
    , m_nextJumpIndex(0)
    , m_holdIndex(0)
    , m_sectionIndex(0) {
    m_currentBeatInfo.currentTime = 0.0f;
    m_currentBeatInfo.bpm = m_simulatedBPM;
    m_currentBeatInfo.isBeat = false;
    m_currentBeatInfo.energy = 0.5f;
    m_currentBeatInfo.intensity = 0.5f;
    m_currentBeatInfo.isStrongBeat = false;
    m_currentBeatInfo.isJumpBeat = false;
    m_currentBeatInfo.isLongNote = false;
    m_currentBeatInfo.longNoteDuration = 0.0f;
    m_currentBeatInfo.jumpInterval = 0.0f;
    m_currentBeatInfo.lastJumpTime = 0.0f;
    m_currentBeatInfo.nextJumpTime = 0.0f;
    m_currentBeatInfo.sectionIndex = 0;
}

AudioAnalyzer::~AudioAnalyzer() {
    Cleanup();
}

bool AudioAnalyzer::Initialize() {
    Reset();
    RefreshTracks();
    std::cout << "AudioAnalyzer initialized in simulation mode" << std::endl;
    if (!m_trackNames.empty()) {
        std::cout << "Found " << m_trackNames.size() << " track(s) in '" << m_musicFolder << "'" << std::endl;
    } else {
        std::cout << "No tracks found in '" << m_musicFolder << "'" << std::endl;
    }
    return true;
}

bool AudioAnalyzer::RefreshTracks() {
    StopPlayback();
    ResetPlayback();
    m_musicFolder = ResolveMusicFolder("music");
    bool found = ScanMusicFolder();
    m_selectedTrackIndex = static_cast<size_t>(-1);
    return found;
}

void AudioAnalyzer::Update(float deltaTime) {
    if (m_useSimulation) {
        m_simulationTime += deltaTime;
        m_currentBeatInfo.currentTime = m_simulationTime;
        UpdateSimulation(deltaTime);
        return;
    }

    UpdateFromTrack(deltaTime);
}

void AudioAnalyzer::UpdateSimulation(float deltaTime) {
    (void)deltaTime;

    float beatInterval = 60.0f / m_simulatedBPM;
    m_currentBeatInfo.isBeat = false;
    m_currentBeatInfo.isStrongBeat = false;
    m_currentBeatInfo.isJumpBeat = false;
    m_currentBeatInfo.isLongNote = false;
    m_currentBeatInfo.longNoteDuration = 0.0f;
    m_currentBeatInfo.jumpInterval = beatInterval;
    m_currentBeatInfo.lastJumpTime = m_lastBeatTime;
    m_currentBeatInfo.nextJumpTime = m_lastBeatTime + beatInterval;
    m_currentBeatInfo.sectionIndex = 0;

    if (m_simulationTime - m_lastBeatTime >= beatInterval) {
        m_currentBeatInfo.isBeat = true;
        m_currentBeatInfo.isStrongBeat = true;
        m_currentBeatInfo.isJumpBeat = true;
        m_lastBeatTime = m_simulationTime;
        m_currentBeatInfo.lastJumpTime = m_lastBeatTime;
        m_currentBeatInfo.nextJumpTime = m_lastBeatTime + beatInterval;
    }

    float timeInMeasure = fmod(m_simulationTime, beatInterval * 4.0f);
    float normalizedTime = timeInMeasure / (beatInterval * 4.0f);

    float primaryWave = sin(normalizedTime * M_PI * 2.0f) * 0.5f + 0.5f;
    float secondaryWave = sin(normalizedTime * M_PI * 8.0f) * 0.2f;
    m_currentBeatInfo.energy = 0.3f + 0.7f * (primaryWave + secondaryWave);

    float songProgress = fmod(m_simulationTime, 60.0f) / 60.0f;

    if (songProgress < 0.1f) {
        m_currentBeatInfo.intensity = 0.2f + songProgress * 2.0f;
    } else if (songProgress < 0.3f) {
        float sectionProgress = (songProgress - 0.1f) / 0.2f;
        m_currentBeatInfo.intensity = 0.4f + sectionProgress * 0.3f;
    } else if (songProgress < 0.6f) {
        float sectionProgress = (songProgress - 0.3f) / 0.3f;
        m_currentBeatInfo.intensity = 0.7f + sectionProgress * 0.2f;
    } else if (songProgress < 0.8f) {
        float sectionProgress = (songProgress - 0.6f) / 0.2f;
        float climaxWave = sin(sectionProgress * M_PI * 4.0f) * 0.1f;
        m_currentBeatInfo.intensity = 0.9f + climaxWave;
    } else {
        float sectionProgress = (songProgress - 0.8f) / 0.2f;
        m_currentBeatInfo.intensity = 0.9f - sectionProgress * 0.7f;
    }

    m_currentBeatInfo.intensity = std::clamp(m_currentBeatInfo.intensity, 0.1f, 1.0f);
    m_currentBeatInfo.bpm = m_simulatedBPM + sin(m_simulationTime * 0.1f) * 10.0f;
}

void AudioAnalyzer::AnalyzeRealAudio(float deltaTime) {
    UpdateFromTrack(deltaTime);
}

void AudioAnalyzer::Cleanup() {
    StopPlayback();
}

void AudioAnalyzer::Reset() {
    m_simulationTime = 0.0f;
    m_lastBeatTime = 0.0f;
    m_currentBeatInfo.currentTime = 0.0f;
    m_currentBeatInfo.bpm = m_simulatedBPM;
    m_currentBeatInfo.isBeat = false;
    m_currentBeatInfo.energy = 0.5f;
    m_currentBeatInfo.intensity = 0.5f;
    m_currentBeatInfo.isStrongBeat = false;
    m_currentBeatInfo.isJumpBeat = false;
    m_currentBeatInfo.isLongNote = false;
    m_currentBeatInfo.longNoteDuration = 0.0f;
    m_currentBeatInfo.jumpInterval = 0.0f;
    m_currentBeatInfo.lastJumpTime = 0.0f;
    m_currentBeatInfo.nextJumpTime = 0.0f;
    m_currentBeatInfo.sectionIndex = 0;
    ResetPlayback();
}

void AudioAnalyzer::ResetPlayback() {
    m_playbackTime = 0.0f;
    m_prevPlaybackTime = 0.0f;
    m_nextBeatIndex = 0;
    m_nextJumpIndex = 0;
    m_holdIndex = 0;
    m_sectionIndex = 0;
    m_playbackActive = false;
}

bool AudioAnalyzer::HasSelectedTrack() const {
    return m_selectedTrackIndex < m_tracks.size();
}

const RhythmSequence* AudioAnalyzer::GetRhythmSequence() const {
    if (!HasSelectedTrack()) {
        return nullptr;
    }
    return &m_tracks[m_selectedTrackIndex].analysis.sequence;
}

float AudioAnalyzer::GetFirstJumpTime() const {
    const RhythmSequence* sequence = GetRhythmSequence();
    if (!sequence || sequence->notes.empty()) {
        return 0.0f;
    }
    return sequence->notes.front().time;
}

void AudioAnalyzer::StartPlayback() {
    if (!HasSelectedTrack()) {
        return;
    }
    ResetPlayback();
    m_playbackActive = true;
#if defined(_WIN32)
    std::wstring widePath = m_tracks[m_selectedTrackIndex].path.wstring();
    if (!widePath.empty()) {
        PlaySoundW(widePath.c_str(), nullptr, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
    }
#endif
}

void AudioAnalyzer::StopPlayback() {
    m_playbackActive = false;
#if defined(_WIN32)
    PlaySoundW(nullptr, nullptr, 0);
#endif
}

bool AudioAnalyzer::ScanMusicFolder() {
    m_tracks.clear();
    m_trackNames.clear();

    fs::path folderPath = fs::absolute(m_musicFolder);
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        return false;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        fs::path path = entry.path();
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext != ".wav") {
            continue;
        }
        TrackEntry track;
        track.name = path.stem().u8string();
        track.path = path;
        m_tracks.push_back(std::move(track));
    }

    std::sort(m_tracks.begin(), m_tracks.end(), [](const TrackEntry& a, const TrackEntry& b) {
        return a.name < b.name;
    });

    for (const auto& track : m_tracks) {
        m_trackNames.push_back(track.name);
    }

    return !m_tracks.empty();
}

bool AudioAnalyzer::SelectTrack(size_t index) {
    if (index >= m_tracks.size()) {
        return false;
    }
    m_selectedTrackIndex = index;
    if (!m_tracks[index].analyzed) {
        if (!AnalyzeTrack(m_tracks[index])) {
            return false;
        }
        m_tracks[index].analyzed = true;
    }
    ResetPlayback();
    return true;
}

bool AudioAnalyzer::AnalyzeTrack(TrackEntry& entry) {
    WavData data;
    if (!LoadWavMono(entry.path, data)) {
        std::cout << "Failed to load WAV: " << entry.path.u8string() << std::endl;
        return false;
    }

    const int windowSize = 1024;
    const int hopSize = 512;
    const float frameTime = static_cast<float>(hopSize) / static_cast<float>(data.sampleRate);
    const std::size_t frameCount = (data.samples.size() < static_cast<size_t>(windowSize))
        ? 0
        : (data.samples.size() - windowSize) / hopSize + 1;
    if (frameCount == 0) {
        return false;
    }

    std::vector<float> energy(frameCount, 0.0f);
    for (std::size_t i = 0; i < frameCount; ++i) {
        std::size_t start = i * hopSize;
        double sum = 0.0;
        for (int j = 0; j < windowSize; ++j) {
            float sample = data.samples[start + j];
            sum += sample * sample;
        }
        energy[i] = static_cast<float>(std::sqrt(sum / windowSize));
    }

    float maxEnergy = *std::max_element(energy.begin(), energy.end());
    if (maxEnergy > 0.0f) {
        for (float& e : energy) {
            e /= maxEnergy;
        }
    }

    std::vector<float> flux(frameCount, 0.0f);
    for (std::size_t i = 1; i < frameCount; ++i) {
        float diff = energy[i] - energy[i - 1];
        flux[i] = std::max(0.0f, diff);
    }

    float meanFlux = 0.0f;
    if (!flux.empty()) {
        meanFlux = std::accumulate(flux.begin(), flux.end(), 0.0f) / static_cast<float>(flux.size());
    }
    float variance = 0.0f;
    for (float value : flux) {
        float delta = value - meanFlux;
        variance += delta * delta;
    }
    variance /= std::max(1.0f, static_cast<float>(flux.size()));
    float stdFlux = std::sqrt(variance);
    float threshold = meanFlux + stdFlux * 0.7f;

    std::vector<float> beatTimes;
    std::vector<float> beatEnergy;
    std::size_t lastBeatIndex = 0;
    int minBeatFrames = static_cast<int>(0.28f / frameTime);
    for (std::size_t i = 1; i + 1 < frameCount; ++i) {
        if (flux[i] > threshold && flux[i] > flux[i - 1] && flux[i] >= flux[i + 1]) {
            if (beatTimes.empty() || static_cast<int>(i - lastBeatIndex) > minBeatFrames) {
                float time = static_cast<float>(i) * frameTime;
                beatTimes.push_back(time);
                beatEnergy.push_back(energy[i]);
                lastBeatIndex = i;
            }
        }
    }

    float duration = static_cast<float>(data.samples.size()) / static_cast<float>(data.sampleRate);
    if (beatTimes.size() < 4) {
        beatTimes.clear();
        beatEnergy.clear();
        float beatInterval = 0.5f;
        for (float t = beatInterval; t < duration; t += beatInterval) {
            beatTimes.push_back(t);
            beatEnergy.push_back(0.5f);
        }
    }

    std::vector<float> intervals;
    for (std::size_t i = 1; i < beatTimes.size(); ++i) {
        intervals.push_back(beatTimes[i] - beatTimes[i - 1]);
    }
    float medianInterval = ComputeMedian(intervals);
    if (medianInterval <= 0.01f) {
        medianInterval = 0.5f;
    }
    float bpm = 60.0f / medianInterval;

    std::vector<float> intensity(frameCount, 0.0f);
    int smoothFrames = std::max(1, static_cast<int>(1.0f / frameTime));
    std::vector<float> prefix(frameCount + 1, 0.0f);
    for (std::size_t i = 0; i < frameCount; ++i) {
        prefix[i + 1] = prefix[i] + energy[i];
    }
    for (std::size_t i = 0; i < frameCount; ++i) {
        std::size_t start = (i > static_cast<std::size_t>(smoothFrames)) ? (i - smoothFrames) : 0;
        std::size_t count = i - start + 1;
        float avg = (prefix[i + 1] - prefix[start]) / static_cast<float>(count);
        intensity[i] = avg;
    }
    float maxIntensity = *std::max_element(intensity.begin(), intensity.end());
    if (maxIntensity > 0.0f) {
        for (float& value : intensity) {
            value /= maxIntensity;
        }
    }

    float avgBeatEnergy = 0.0f;
    if (!beatEnergy.empty()) {
        avgBeatEnergy = std::accumulate(beatEnergy.begin(), beatEnergy.end(), 0.0f) /
            static_cast<float>(beatEnergy.size());
    }
    std::vector<bool> strongBeats(beatTimes.size(), false);
    for (std::size_t i = 0; i < beatTimes.size(); ++i) {
        bool energetic = beatEnergy[i] > avgBeatEnergy * 1.15f;
        bool downbeat = (i % 4 == 0);
        strongBeats[i] = energetic || downbeat;
    }

    std::vector<HoldSegment> holds;
    float sustainThreshold = 0.55f;
    float fluxThreshold = 0.02f;
    int minHoldFrames = static_cast<int>(0.7f / frameTime);
    bool inHold = false;
    std::size_t holdStart = 0;
    for (std::size_t i = 1; i < frameCount; ++i) {
        bool sustain = energy[i] > sustainThreshold && flux[i] < fluxThreshold && intensity[i] > 0.4f;
        if (sustain && !inHold) {
            inHold = true;
            holdStart = i;
        } else if (!sustain && inHold) {
            std::size_t holdEnd = i;
            if (static_cast<int>(holdEnd - holdStart) >= minHoldFrames) {
                float startTime = static_cast<float>(holdStart) * frameTime;
                float endTime = static_cast<float>(holdEnd) * frameTime;
                holds.push_back({startTime, endTime});
            }
            inHold = false;
        }
    }
    if (inHold) {
        std::size_t holdEnd = frameCount - 1;
        if (static_cast<int>(holdEnd - holdStart) >= minHoldFrames) {
            float startTime = static_cast<float>(holdStart) * frameTime;
            float endTime = static_cast<float>(holdEnd) * frameTime;
            holds.push_back({startTime, endTime});
        }
    }

    std::vector<RhythmSection> sections;
    float sectionWindow = 4.0f;
    int sectionFrames = std::max(1, static_cast<int>(sectionWindow / frameTime));
    float lastSectionStart = 0.0f;
    float lastSectionIntensity = intensity.empty() ? 0.5f : intensity.front();
    for (std::size_t i = 0; i + sectionFrames < frameCount; i += sectionFrames) {
        float avg = 0.0f;
        for (std::size_t j = 0; j < static_cast<std::size_t>(sectionFrames); ++j) {
            avg += intensity[i + j];
        }
        avg /= static_cast<float>(sectionFrames);
        float time = static_cast<float>(i) * frameTime;
        if (std::abs(avg - lastSectionIntensity) > 0.18f && time - lastSectionStart > 6.0f) {
            sections.push_back({lastSectionStart, time, lastSectionIntensity});
            lastSectionStart = time;
            lastSectionIntensity = avg;
        }
    }
    if (duration > lastSectionStart + 1.0f) {
        sections.push_back({lastSectionStart, duration, lastSectionIntensity});
    }

    std::vector<RhythmNote> notes;
    const float targetJumpTime = 0.7f;
    int beatsPerJump = std::clamp(static_cast<int>(std::round(targetJumpTime / medianInterval)), 1, 4);
    std::size_t beatIdx = 0;
    std::size_t holdIdx = 0;
    while (beatIdx < beatTimes.size()) {
        float beatTime = beatTimes[beatIdx];
        if (holdIdx < holds.size() && beatTime >= holds[holdIdx].start && beatTime <= holds[holdIdx].end) {
            RhythmNote note{};
            note.time = beatTime;
            note.energy = SampleEnvelope(energy, beatTime, frameTime);
            note.intensity = SampleEnvelope(intensity, beatTime, frameTime);
            note.holdDuration = std::max(0.0f, holds[holdIdx].end - beatTime);
            note.strongBeat = strongBeats[beatIdx];
            notes.push_back(note);

            while (beatIdx < beatTimes.size() && beatTimes[beatIdx] <= holds[holdIdx].end) {
                ++beatIdx;
            }
            ++holdIdx;
            continue;
        }

        bool shouldJump = (beatIdx % static_cast<std::size_t>(beatsPerJump) == 0) || strongBeats[beatIdx];
        if (shouldJump) {
            RhythmNote note{};
            note.time = beatTime;
            note.energy = SampleEnvelope(energy, beatTime, frameTime);
            note.intensity = SampleEnvelope(intensity, beatTime, frameTime);
            note.holdDuration = 0.0f;
            note.strongBeat = strongBeats[beatIdx];
            notes.push_back(note);
        }
        ++beatIdx;
    }

    for (std::size_t i = 0; i + 1 < notes.size(); ++i) {
        notes[i].jumpInterval = notes[i + 1].time - notes[i].time;
    }
    if (!notes.empty()) {
        notes.back().jumpInterval = medianInterval * static_cast<float>(beatsPerJump);
    }

    entry.analysis.sequence.name = entry.name;
    entry.analysis.sequence.bpm = bpm;
    entry.analysis.sequence.duration = duration;
    entry.analysis.sequence.beatInterval = medianInterval;
    entry.analysis.sequence.notes = std::move(notes);
    entry.analysis.sequence.sections = std::move(sections);
    entry.analysis.beatTimes = std::move(beatTimes);
    entry.analysis.strongBeats = std::move(strongBeats);
    entry.analysis.energyEnvelope = std::move(energy);
    entry.analysis.intensityEnvelope = std::move(intensity);
    entry.analysis.envelopeStep = frameTime;
    entry.analysis.holds = std::move(holds);

    std::cout << "Analyzed track '" << entry.name << "' BPM=" << bpm
              << " beats=" << entry.analysis.beatTimes.size()
              << " notes=" << entry.analysis.sequence.notes.size()
              << std::endl;

    return true;
}

void AudioAnalyzer::UpdateFromTrack(float deltaTime) {
    m_currentBeatInfo.isBeat = false;
    m_currentBeatInfo.isStrongBeat = false;
    m_currentBeatInfo.isJumpBeat = false;
    m_currentBeatInfo.isLongNote = false;
    m_currentBeatInfo.longNoteDuration = 0.0f;
    m_currentBeatInfo.jumpInterval = 0.0f;
    m_currentBeatInfo.sectionIndex = 0;

    if (!HasSelectedTrack()) {
        UpdateSimulation(deltaTime);
        return;
    }

    TrackAnalysis& analysis = m_tracks[m_selectedTrackIndex].analysis;
    if (m_playbackActive) {
        m_prevPlaybackTime = m_playbackTime;
        m_playbackTime += deltaTime;
    }

    m_currentBeatInfo.currentTime = m_playbackTime;
    m_currentBeatInfo.bpm = analysis.sequence.bpm;
    m_currentBeatInfo.energy = SampleEnvelope(analysis.energyEnvelope, m_playbackTime, analysis.envelopeStep);
    m_currentBeatInfo.intensity = SampleEnvelope(analysis.intensityEnvelope, m_playbackTime, analysis.envelopeStep);

    while (m_sectionIndex + 1 < analysis.sequence.sections.size() &&
           m_playbackTime >= analysis.sequence.sections[m_sectionIndex].endTime) {
        ++m_sectionIndex;
    }
    m_currentBeatInfo.sectionIndex = static_cast<int>(m_sectionIndex);

    while (m_nextBeatIndex < analysis.beatTimes.size() &&
           analysis.beatTimes[m_nextBeatIndex] <= m_playbackTime) {
        if (analysis.beatTimes[m_nextBeatIndex] > m_prevPlaybackTime) {
            m_currentBeatInfo.isBeat = true;
            m_currentBeatInfo.isStrongBeat = analysis.strongBeats[m_nextBeatIndex];
        }
        ++m_nextBeatIndex;
    }

    while (m_nextJumpIndex < analysis.sequence.notes.size() &&
           analysis.sequence.notes[m_nextJumpIndex].time <= m_playbackTime) {
        if (analysis.sequence.notes[m_nextJumpIndex].time > m_prevPlaybackTime) {
            m_currentBeatInfo.isJumpBeat = true;
            m_currentBeatInfo.jumpInterval = analysis.sequence.notes[m_nextJumpIndex].jumpInterval;
            m_currentBeatInfo.longNoteDuration = analysis.sequence.notes[m_nextJumpIndex].holdDuration;
        }
        ++m_nextJumpIndex;
    }

    float lastJump = 0.0f;
    float nextJump = m_playbackTime + analysis.sequence.beatInterval;
    if (!analysis.sequence.notes.empty()) {
        if (m_nextJumpIndex == 0) {
            nextJump = analysis.sequence.notes.front().time;
            lastJump = nextJump - analysis.sequence.notes.front().jumpInterval;
            m_currentBeatInfo.jumpInterval = analysis.sequence.notes.front().jumpInterval;
        } else if (m_nextJumpIndex < analysis.sequence.notes.size()) {
            lastJump = analysis.sequence.notes[m_nextJumpIndex - 1].time;
            nextJump = analysis.sequence.notes[m_nextJumpIndex].time;
            m_currentBeatInfo.jumpInterval = analysis.sequence.notes[m_nextJumpIndex - 1].jumpInterval;
        } else {
            lastJump = analysis.sequence.notes.back().time;
            nextJump = lastJump + analysis.sequence.notes.back().jumpInterval;
            m_currentBeatInfo.jumpInterval = analysis.sequence.notes.back().jumpInterval;
        }
    }
    m_currentBeatInfo.lastJumpTime = lastJump;
    m_currentBeatInfo.nextJumpTime = nextJump;

    while (m_holdIndex < analysis.holds.size() && analysis.holds[m_holdIndex].end <= m_playbackTime) {
        ++m_holdIndex;
    }
    if (m_holdIndex < analysis.holds.size()) {
        const HoldSegment& hold = analysis.holds[m_holdIndex];
        if (m_playbackTime >= hold.start && m_playbackTime <= hold.end) {
            m_currentBeatInfo.isLongNote = true;
            m_currentBeatInfo.longNoteDuration = std::max(m_currentBeatInfo.longNoteDuration,
                                                         hold.end - m_playbackTime);
        }
    }
}

float AudioAnalyzer::SampleEnvelope(const std::vector<float>& envelope, float time, float step) const {
    if (envelope.empty() || step <= 0.0f) {
        return 0.5f;
    }
    std::size_t index = static_cast<std::size_t>(time / step);
    if (index >= envelope.size()) {
        return envelope.back();
    }
    return envelope[index];
}
