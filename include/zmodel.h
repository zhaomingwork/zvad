/**
 * Description:
 * Copyright 2024. All Rights Reserved.
 * Author: Zhao Ming (zhaomingwork@qq.com)
 */

#ifndef ENGINE_INTERFACE_H
#define ENGINE_INTERFACE_H

#include <vector>
#include <cstring>
#include <string>
class VadEngine
{
public:
	virtual void init_model(const std::wstring &model_path) = 0;
	virtual void set_sample_rate(int sample_rate) = 0;
	virtual void reset_states() = 0;
	virtual float predict_possible(const std::vector<float> &data) = 0;
	virtual long get_chunk_size() = 0;
};

/**
 * @description: return instance of the vad model
 * @param {wstring} &model_path
 * @return {*}
 */
VadEngine *create_engine(const std::wstring &model_path);

#endif // ENGINE_INTERFACE_H
