#include <lv2/core/lv2.h>
#include <lv2/urid/urid.h>
#include <cmath>
#include <cstring>
#include <stdlib.h>

#define CHAOS_AMEN_URI "http://github.com/yourname/chaos-amen"

enum PortIndex {
    OUTPUT      = 0,
    BPM         = 1,
    CHAOS_K     = 2,
    CHAOS_INTENSITY = 3,
    KICK_LEVEL  = 4,
    SNARE_LEVEL = 5,
    HIHAT_LEVEL = 6,
    COWBELL_LEVEL = 7
};

// Drum voice classes
class KickSynth {
private:
    double phase;
    double decay_env;
    double sample_rate;
    
public:
    KickSynth() : phase(0.0), decay_env(0.0), sample_rate(44100.0) {}
    
    void setSampleRate(double sr) { sample_rate = sr; }
    
    void trigger() {
        phase = 0.0;
        decay_env = 1.0;
    }
    
    float process() {
        if (decay_env <= 1e-4) return 0.0f;
        
        // Frequency modulation for punch
        double freq = 60.0 * exp(-phase * 8.0);
        phase += freq / sample_rate;
        if (phase > 1.0) phase -= 1.0;
        
        // Sine wave with exponential decay
        float output = sin(2.0 * M_PI * phase) * decay_env;
        decay_env *= 0.9995; // Decay rate
        
        return output;
    }
};

class SnareSynth {
private:
    double noise_amp;
    double tone_phase;
    double sample_rate;
    
public:
    SnareSynth() : noise_amp(0.0), tone_phase(0.0), sample_rate(44100.0) {}
    
    void setSampleRate(double sr) { sample_rate = sr; }
    
    void trigger() {
        noise_amp = 1.0;
        tone_phase = 0.0;
    }
    
    float process() {
        if (noise_amp <= 1e-4) return 0.0f;
        
        // White noise component
        float noise = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        
        // Tonal component at ~200Hz
        tone_phase += 200.0 / sample_rate;
        if (tone_phase > 1.0) tone_phase -= 1.0;
        float tone = sin(2.0 * M_PI * tone_phase) * 0.3;
        
        float output = (noise * 0.7 + tone) * noise_amp;
        noise_amp *= 0.998; // Fast decay
        
        return output;
    }
};

class HihatSynth {
private:
    double noise_amp;
    double filter_state;
    double sample_rate;
    
public:
    HihatSynth() : noise_amp(0.0), filter_state(0.0), sample_rate(44100.0) {}
    
    void setSampleRate(double sr) { sample_rate = sr; }
    
    void trigger() {
        noise_amp = 1.0;
    }
    
    float process() {
        if (noise_amp <= 1e-4) return 0.0f;
        
        // High-frequency filtered noise
        float noise = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        
        // Simple high-pass filter
        filter_state = filter_state * 0.9 + noise * 0.1;
        float output = (noise - filter_state) * noise_amp;
        
        noise_amp *= 0.999; // Very fast decay
        
        return output * 0.5f;
    }
};

class CowbellSynth {
private:
    double phase1, phase2;
    double decay_env;
    double sample_rate;
    
public:
    CowbellSynth() : phase1(0.0), phase2(0.0), decay_env(0.0), sample_rate(44100.0) {}
    
    void setSampleRate(double sr) { sample_rate = sr; }
    
    void trigger() {
        phase1 = 0.0;
        phase2 = 0.0;
        decay_env = 1.0;
    }
    
    float process() {
        if (decay_env <= 1e-4) return 0.0f;
        
        // Two sine waves for metallic timbre
        phase1 += 800.0 / sample_rate;
        phase2 += 540.0 / sample_rate;
        
        if (phase1 > 1.0) phase1 -= 1.0;
        if (phase2 > 1.0) phase2 -= 1.0;
        
        float osc1 = sin(2.0 * M_PI * phase1);
        float osc2 = sin(2.0 * M_PI * phase2);
        
        float output = (osc1 + osc2 * 0.7) * decay_env;
        decay_env *= 0.9998;
        
        return output * 0.6f;
    }
};

// Main plugin class
class ChaosAmen {
private:
    double sample_rate;
    uint32_t frame_count;
    uint32_t step_frames;
    uint32_t current_step;
    
    // Chaos variables
    double chaos_x;
    
    // Base Amen pattern
    bool base_pattern[4][16] = {
        {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0}, // kick
        {0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0}, // snare
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // hihat
        {0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,0}  // cowbell
    };
    
    // Current chaotic pattern
    bool current_pattern[4][16];
    
    // Synthesizers
    KickSynth kick;
    SnareSynth snare;
    HihatSynth hihat;
    CowbellSynth cowbell;
    
    // Ports
    float* output;
    const float* bpm;
    const float* chaos_k;
    const float* chaos_intensity;
    const float* kick_level;
    const float* snare_level;
    const float* hihat_level;
    const float* cowbell_level;
    
    // Default values for safety
    float default_bpm;
    float default_chaos_k;
    float default_chaos_intensity;
    float default_kick_level;
    float default_snare_level;
    float default_hihat_level;
    float default_cowbell_level;
    
    void generateChaoticPattern() {
        double k = getChaosK();
        double intensity = getChaosIntensity();
        
        // Clamp values for safety
        k = fmax(1.0, fmin(4.0, k));
        intensity = fmax(0.0, fmin(1.0, intensity));
        
        // Copy base pattern
        memcpy(current_pattern, base_pattern, sizeof(base_pattern));
        
        for (int i = 0; i < 16; i++) {
            // Generate chaos values with bounds checking
            chaos_x = k * chaos_x * (1.0 - chaos_x);
            if (chaos_x < 0.0 || chaos_x > 1.0) chaos_x = 0.5; // Reset if out of bounds
            double chaos1 = chaos_x;
            
            chaos_x = k * chaos_x * (1.0 - chaos_x);
            if (chaos_x < 0.0 || chaos_x > 1.0) chaos_x = 0.5;
            double chaos2 = chaos_x;
            
            chaos_x = k * chaos_x * (1.0 - chaos_x);
            if (chaos_x < 0.0 || chaos_x > 1.0) chaos_x = 0.5;
            double chaos3 = chaos_x;
            
            chaos_x = k * chaos_x * (1.0 - chaos_x);
            if (chaos_x < 0.0 || chaos_x > 1.0) chaos_x = 0.5;
            double chaos4 = chaos_x;
            
            // Apply chaos to kick
            if (!base_pattern[0][i] && chaos1 < intensity * 0.3) {
                current_pattern[0][i] = true;
            } else if (base_pattern[0][i] && chaos1 > (1.0 - intensity * 0.2)) {
                current_pattern[0][i] = false;
            }
            
            // Apply chaos to snare
            if (!base_pattern[1][i] && chaos2 < intensity * 0.4) {
                current_pattern[1][i] = true;
            } else if (base_pattern[1][i] && chaos2 > (1.0 - intensity * 0.1)) {
                current_pattern[1][i] = false;
            }
            
            // Apply chaos to hihat
            if (chaos3 > (1.0 - intensity * 0.8)) {
                current_pattern[2][i] = false;
            }
            
            // Apply chaos to cowbell
            if (chaos4 < intensity * 0.6) {
                current_pattern[3][i] = true;
            } else if (base_pattern[3][i] && chaos4 > (1.0 - intensity * 0.3)) {
                current_pattern[3][i] = false;
            }
        }
    }
    
public:
    ChaosAmen(double rate) : sample_rate(rate), frame_count(0), current_step(0), chaos_x(0.5) {
        kick.setSampleRate(rate);
        snare.setSampleRate(rate);
        hihat.setSampleRate(rate);
        cowbell.setSampleRate(rate);
        
        // Initialize all ports to null
        output = nullptr;
        bpm = nullptr;
        chaos_k = nullptr;
        chaos_intensity = nullptr;
        kick_level = nullptr;
        snare_level = nullptr;
        hihat_level = nullptr;
        cowbell_level = nullptr;
        
        // Set default values
        default_bpm = 136.0f;
        default_chaos_k = 3.8f;
        default_chaos_intensity = 0.3f;
        default_kick_level = 0.8f;
        default_snare_level = 0.7f;
        default_hihat_level = 0.5f;
        default_cowbell_level = 0.6f;
        
        // Initialize with base pattern
        memcpy(current_pattern, base_pattern, sizeof(base_pattern));
        
        step_frames = (uint32_t)(sample_rate * 60.0 / (default_bpm * 4.0));
    }
    
    void updateStepTime() {
        // 16th notes at current BPM with safety check
        float current_bpm = getBpm();
        if (current_bpm < 1.0f) current_bpm = 136.0f;
        step_frames = (uint32_t)(sample_rate * 60.0 / (current_bpm * 4.0));
    }
    
    // Safe parameter getters
    float getBpm() { return bpm ? *bpm : default_bpm; }
    float getChaosK() { return chaos_k ? *chaos_k : default_chaos_k; }
    float getChaosIntensity() { return chaos_intensity ? *chaos_intensity : default_chaos_intensity; }
    float getKickLevel() { return kick_level ? *kick_level : default_kick_level; }
    float getSnareLevel() { return snare_level ? *snare_level : default_snare_level; }
    float getHihatLevel() { return hihat_level ? *hihat_level : default_hihat_level; }
    float getCowbellLevel() { return cowbell_level ? *cowbell_level : default_cowbell_level; }
    
    void connectPort(uint32_t port, void* data) {
        switch (port) {
            case OUTPUT: output = (float*)data; break;
            case BPM: bpm = (const float*)data; break;
            case CHAOS_K: chaos_k = (const float*)data; break;
            case CHAOS_INTENSITY: chaos_intensity = (const float*)data; break;
            case KICK_LEVEL: kick_level = (const float*)data; break;
            case SNARE_LEVEL: snare_level = (const float*)data; break;
            case HIHAT_LEVEL: hihat_level = (const float*)data; break;
            case COWBELL_LEVEL: cowbell_level = (const float*)data; break;
        }
    }
    
    void run(uint32_t n_samples) {
        if (!output) return; // Safety check
        
        updateStepTime();
        
        for (uint32_t i = 0; i < n_samples; i++) {
            // Check if we need to trigger new step
            if (frame_count % step_frames == 0) {
                if (current_pattern[0][current_step]) kick.trigger();
                if (current_pattern[1][current_step]) snare.trigger();
                if (current_pattern[2][current_step]) hihat.trigger();
                if (current_pattern[3][current_step]) cowbell.trigger();
                
                current_step = (current_step + 1) % 16;
                
                // Generate new pattern occasionally
                if (current_step == 0 && (rand() % 4) == 0) {
                    generateChaoticPattern();
                }
            }
            
            // Process synthesizers and mix
            float sample = 0.0f;
            sample += kick.process() * getKickLevel();
            sample += snare.process() * getSnareLevel();
            sample += hihat.process() * getHihatLevel();
            sample += cowbell.process() * getCowbellLevel();
            
            // Clamp output to prevent distortion
            sample = fmaxf(-1.0f, fminf(1.0f, sample * 0.5f));
            output[i] = sample;
            frame_count++;
        }
    }
};

// LV2 C interface
extern "C" {

static LV2_Handle instantiate(const LV2_Descriptor* descriptor,
                             double rate,
                             const char* bundle_path,
                             const LV2_Feature* const* features) {
    return new ChaosAmen(rate);
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
    ChaosAmen* plugin = (ChaosAmen*)instance;
    plugin->connectPort(port, data);
}

static void activate(LV2_Handle instance) {
    // Nothing to do
}

static void run(LV2_Handle instance, uint32_t n_samples) {
    ChaosAmen* plugin = (ChaosAmen*)instance;
    plugin->run(n_samples);
}

static void deactivate(LV2_Handle instance) {
    // Nothing to do
}

static void cleanup(LV2_Handle instance) {
    delete (ChaosAmen*)instance;
}

static const void* extension_data(const char* uri) {
    return NULL;
}

static const LV2_Descriptor descriptor = {
    CHAOS_AMEN_URI,
    instantiate,
    connect_port,
    activate,
    run,
    deactivate,
    cleanup,
    extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
    switch (index) {
        case 0: return &descriptor;
        default: return NULL;
    }
}

} // extern "C"
