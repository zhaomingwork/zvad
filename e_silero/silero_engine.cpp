// most from https://github.com/snakers4/silero-vad
#include <iostream>
#include <vector>
#include <sstream>
#include <cstring>
#include <limits>
#include <chrono>
#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>
#include <string>
#include "onnxruntime_cxx_api.h"
#include "wav.h"
#include <cstdio>
#include <cstdarg>
#if __cplusplus < 201703L
#include <memory>
#endif

#include <thread>
#include <mutex>
#include "../include/zmodel.h"
class SilEngine: public VadEngine
{
private:
    // OnnxRuntime resources
    Ort::Env env;
    Ort::SessionOptions session_options;
    std::shared_ptr<Ort::Session> session = nullptr;
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);

private:
 
	void init_model(const std::wstring& model_path) override;
    void reset_states() override;
    float predict_possible(const std::vector<float> &data) override;

    int64_t window_size_samples;  // Assign when init, support 256 512 768 for 8k; 512 1024 1536 for 16k.
    int sample_rate;  //Assign when init support 16000 or 8000      
    int sr_per_ms;   // Assign when init, support 8 or 16
    float threshold; 
    int min_silence_samples; // sr_per_ms * #ms
    int min_silence_samples_at_max_speech; // sr_per_ms * #98
    int min_speech_samples; // sr_per_ms * #ms
    float max_speech_samples;
    int speech_pad_samples; // usually a 
    int audio_length_samples;

    // model states
    bool triggered = false;
    unsigned int temp_end = 0;
    unsigned int current_sample = 0;    
    // MAX 4294967295 samples / 8sample per ms / 1000 / 60 = 8947 minutes  
    int prev_end;
    int next_start = 0;

 


    // Onnx model
    // Inputs
    std::vector<Ort::Value> ort_inputs;
    
    std::vector<const char *> input_node_names = {"input", "state", "sr"};
    std::vector<float> input;
    unsigned int size_state = 2 * 1 * 128; // It's FIXED.
    std::vector<float> _state;
    std::vector<int64_t> sr;

    int64_t input_node_dims[2] = {};
    const int64_t state_node_dims[3] = {2, 1, 128}; 
    const int64_t sr_node_dims[1] = {1};

    // Outputs
    std::vector<Ort::Value> ort_outputs;
    std::vector<const char *> output_node_names = {"output", "stateN"};

};

/**
 * @description: make sure engine a single instance
 * @return {*}
 */
class Singleton {
public:
    // Returns a pointer to the single instance of the class
    static Singleton* getInstance(const std::wstring& model_path) {
        // Check if the instance is null before creating it
		std::lock_guard<std::mutex> my_guard(my_mutex);
        if (instance == nullptr) {
            // Create the instance and assign it to the pointer
            instance = new Singleton(model_path);
        }
         return instance;
    }

    // Some public methods or fields that you want to access from anywhere
    std::shared_ptr<Ort::Session> getSession() {
        return session;
    }

private:
    // Make the constructor private so that no one can create an instance
    Singleton(const std::wstring& model_path) {
		
		init_onnx_model(model_path);
        // ...
    }

    // Delete the copy constructor and the assignment operator
    // to prevent copying or assigning the instance
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // Declare a static pointer that holds the instance
    static Singleton* instance;
    Ort::Env env;
    Ort::SessionOptions session_options;
    std::shared_ptr<Ort::Session> session = nullptr;
    //Ort::AllocatorWithDefaultOptions allocator;
    //Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);
	static std::mutex my_mutex;
 
    /**
     * @description: 
     * @param {wstring&} model_path
     * @return {*}
     */    
    void init_onnx_model(const std::wstring& model_path)
    {
		int intra_threads=1;
		int inter_threads=1;
        session_options.SetIntraOpNumThreads(intra_threads);
        session_options.SetInterOpNumThreads(inter_threads);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        // Load model
		//const ORTCHAR_T* model_path1 = model_path; //ORT_TSTR("silero_vad.onnx");
        std::string path_string( model_path.begin(), model_path.end() );
        //Ort::Session session_tmp(env, string_.c_str(),session_options);
        session = std::make_shared<Ort::Session>(env,path_string.c_str(), session_options);
    };
};
 
Singleton* Singleton::instance = nullptr;
std::mutex Singleton::my_mutex;

 

void SilEngine::init_model(const std::wstring& model_path)
{
    Singleton* s = Singleton::getInstance(model_path);
		session=s->getSession();
        int Sample_rate = 16000;
		int windows_frame_size = 32;
        float Threshold = 0.5; int min_silence_duration_ms = 0;
        int speech_pad_ms = 32; int min_speech_duration_ms = 32;
        float max_speech_duration_s = std::numeric_limits<float>::infinity();
		
		
		
        threshold = Threshold;
        sample_rate = Sample_rate;
        sr_per_ms = sample_rate / 1000;

        window_size_samples = windows_frame_size * sr_per_ms;

        min_speech_samples = sr_per_ms * min_speech_duration_ms;
        speech_pad_samples = sr_per_ms * speech_pad_ms;

        max_speech_samples = (
            sample_rate * max_speech_duration_s
            - window_size_samples
            - 2 * speech_pad_samples
            );

        min_silence_samples = sr_per_ms * min_silence_duration_ms;
        min_silence_samples_at_max_speech = sr_per_ms * 98;

        input.resize(window_size_samples);
        input_node_dims[0] = 1;
        input_node_dims[1] = window_size_samples;

        _state.resize(size_state);
        sr.resize(1);
        sr[0] = sample_rate;
		printf("init model\n");
}
void SilEngine::reset_states(){
        std::memset(_state.data(), 0.0f, _state.size() * sizeof(float));
        triggered = false;
        temp_end = 0;
        current_sample = 0;

        prev_end = next_start = 0;

       // speeches.clear();
       // current_speech = timestamp_t();
}
float SilEngine::predict_possible(const std::vector<float> &data)
{
        input.assign(data.begin(), data.end());
        Ort::Value input_ort = Ort::Value::CreateTensor<float>(
            memory_info, input.data(), input.size(), input_node_dims, 2);
	    //printf("_state.size() =%ld\n",_state.size());
        Ort::Value state_ort = Ort::Value::CreateTensor<float>(
            memory_info, _state.data(), _state.size(), state_node_dims, 3);
        Ort::Value sr_ort = Ort::Value::CreateTensor<int64_t>(
            memory_info, sr.data(), sr.size(), sr_node_dims, 1);

        // Clear and add inputs
        ort_inputs.clear();
        ort_inputs.emplace_back(std::move(input_ort));
        ort_inputs.emplace_back(std::move(state_ort));
        ort_inputs.emplace_back(std::move(sr_ort));

        // Infer
        ort_outputs = session->Run(
            Ort::RunOptions{nullptr},
            input_node_names.data(), ort_inputs.data(), ort_inputs.size(),
            output_node_names.data(), output_node_names.size());

        // Output probability & update h,c recursively
        float speech_prob = ort_outputs[0].GetTensorMutableData<float>()[0];
        float *stateN = ort_outputs[1].GetTensorMutableData<float>();
        std::memcpy(_state.data(), stateN, size_state * sizeof(float));

        // Push forward sample index
        current_sample += window_size_samples;
        
		//printf("now speech_prob=%.3f,threshold=%.3f\n",speech_prob,threshold);
		return speech_prob;
}
/**
 * @description: create this engine class by its interface
 * @return {*}
 */
VadEngine* create_engine(const std::wstring &model_path)
{
	VadEngine * new_engine= new SilEngine();
	new_engine->init_model(model_path);
	return new_engine;
	 
}