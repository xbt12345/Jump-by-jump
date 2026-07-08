#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <filesystem>
#include "../game/LevelGenerator.h"

class AudioAnalyzer {
public:
    AudioAnalyzer();
    ~AudioAnalyzer();
    
    bool Initialize();
    bool RefreshTracks();
    void Update(float deltaTime);
    void Cleanup();
    void Reset();
    void ResetPlayback();
    
    // 音频分析结果
    BeatInfo GetBeatInfo() const { return m_currentBeatInfo; }
    
    // 模拟音频数据（用于测试）
    void SetSimulatedBPM(float bpm) { m_simulatedBPM = bpm; }
    void EnableSimulation(bool enable) { m_useSimulation = enable; }

    const std::vector<std::string>& GetTrackNames() const { return m_trackNames; }
    bool SelectTrack(size_t index);
    bool HasSelectedTrack() const;
    const RhythmSequence* GetRhythmSequence() const;
    float GetFirstJumpTime() const;
    void StartPlayback();
    void StopPlayback();
    bool IsPlaybackActive() const { return m_playbackActive; }

private:
    struct HoldSegment {
        float start;
        float end;
    };

    struct TrackAnalysis {
        RhythmSequence sequence;
        std::vector<float> beatTimes;
        std::vector<bool> strongBeats;
        std::vector<float> energyEnvelope;
        std::vector<float> intensityEnvelope;
        float envelopeStep = 0.0f;
        std::vector<HoldSegment> holds;
    };

    struct TrackEntry {
        std::string name;
        std::filesystem::path path;
        TrackAnalysis analysis;
        bool analyzed = false;
    };

    BeatInfo m_currentBeatInfo;
    
    // 模拟参数
    bool m_useSimulation;
    float m_simulatedBPM;
    float m_simulationTime;
    float m_lastBeatTime;

    std::string m_musicFolder;
    std::vector<TrackEntry> m_tracks;
    std::vector<std::string> m_trackNames;
    size_t m_selectedTrackIndex;
    bool m_playbackActive;
    float m_playbackTime;
    float m_prevPlaybackTime;
    size_t m_nextBeatIndex;
    size_t m_nextJumpIndex;
    size_t m_holdIndex;
    size_t m_sectionIndex;
    
    void UpdateSimulation(float deltaTime);
    void AnalyzeRealAudio(float deltaTime);
    bool ScanMusicFolder();
    bool AnalyzeTrack(TrackEntry& entry);
    void UpdateFromTrack(float deltaTime);
    float SampleEnvelope(const std::vector<float>& envelope, float time, float step) const;
};
