/**
 * Description:
 * Copyright 2024. All Rights Reserved.
 * Author: Zhao Ming (zhaomingwork@qq.com)
 */

#include "include/zmodel.h"
#include "include/zvad.h"
#define CHUNK_SIZE 1024

/**
 * @description:
 * @param {ZVAD_OBJ} *vad
 * @return {*}
 */
void print_state(ZVAD_OBJ *vad)
{

	if (vad->last_state == ZVAD_OBJ_STATE::ZVAD_OBJ_NOINPUT)
	{
		printf("now state is ZVAD_OBJ_NOINPUT,ave val=%f\n", vad->new_state_ave);
	}
	if (vad->last_state == ZVAD_OBJ_STATE::ZVAD_OBJ_BEGIN_SPEAKING)
	{
		printf("now state is ZVAD_OBJ_BEGIN_SPEAKING,ave val=%f\n", vad->new_state_ave);
	}
	if (vad->last_state == ZVAD_OBJ_STATE::ZVAD_OBJ_SPEAKING)
	{
		printf("now state is ZVAD_OBJ_SPEAKING,ave val=%f\n", vad->new_state_ave);
	}
	if (vad->last_state == ZVAD_OBJ_STATE::ZVAD_OBJ_BEGIN_SILENCE)
	{
		printf("now state is ZVAD_OBJ_BEGIN_SILENCE,ave val=%f\n", vad->new_state_ave);
	}
	if (vad->last_state == ZVAD_OBJ_STATE::ZVAD_OBJ_SILENCE)
	{
		printf("now state is ZVAD_OBJ_SILENCE,ave val=%f\n", vad->new_state_ave);
	}
	printf("data len=%d,now time=%f\n", vad->data_len, (float)vad->data_len / 16000);
}
/**
 * @description: the logic for changing the vad state
 * @param {ZVAD_OBJ} *vad - the engine of vad
 * @param {float} poss - the possibility of this chunk is speech
 * @return {*}
 */
void update_vad_state(ZVAD_OBJ *vad, float poss)
{
    
	if (poss >= vad->act_threshold && vad->last_state != ZVAD_OBJ_STATE::ZVAD_OBJ_SPEAKING)
	{
		vad->last_state = ZVAD_OBJ_STATE::ZVAD_OBJ_BEGIN_SPEAKING;
	}

	if (poss >= vad->act_threshold - 0.15 && vad->last_state == ZVAD_OBJ_STATE::ZVAD_OBJ_BEGIN_SPEAKING)
	{

		vad->last_state = ZVAD_OBJ_STATE::ZVAD_OBJ_SPEAKING;
	}
	if (poss <= vad->act_threshold - 0.15 && vad->last_state != ZVAD_OBJ_STATE::ZVAD_OBJ_SILENCE)
	{
		vad->last_state = ZVAD_OBJ_STATE::ZVAD_OBJ_BEGIN_SILENCE;
	}
	if (poss <= vad->act_threshold && vad->last_state == ZVAD_OBJ_STATE::ZVAD_OBJ_BEGIN_SILENCE)
	{
		vad->last_state = ZVAD_OBJ_STATE::ZVAD_OBJ_SILENCE;
	}

	// print_state(vad);

	return;
}

/**
 * @description: return current state
 * @param {ZVAD_OBJ} *vad
 * @return {*} the current state value
 */
ZVAD_OBJ_STATE vad_get_state(ZVAD_OBJ *vad)
{
	return vad->last_state;
}

/**
 * @description: init the vad engine
 * @param {int} sample_rate
 * @param {int} channels
 * @param {int} mode
 * @return {*}
 */
ZVAD_OBJ *vad_init(char *model_path, int sample_rate, int channels, int mode, float act_threshold)
{
	// std::wstring path = L"silero_vad.onnx";
	std::wstring path(model_path, model_path + strlen(model_path));
	VadEngine *vad_engine = create_engine(path);
	vad_engine->set_sample_rate(sample_rate);
	ZVAD_OBJ *vad_obj = (ZVAD_OBJ *)malloc(sizeof(ZVAD_OBJ));
	vad_obj->vad_engine = (void *)vad_engine;
	std::vector<float> *samples = new std::vector<float>();
	vad_obj->vector_samples = (void *)samples;
	vad_obj->sample_rate = sample_rate;
	vad_obj->channels = channels;
	vad_obj->mode = mode;
	vad_obj->data_len = 0;
	vad_obj->act_threshold = act_threshold;

	return vad_obj;
}

/**
 * @description: feed data to the vad engine
 * @param {ZVAD_OBJ} *vad the engine point
 * @param {float} *data the data of pcm in float type
 * @param {int} data_len the data length
 * @return {*}
 */
ZVAD_OBJ_STATE vad_feed(ZVAD_OBJ *vad, float *data, int data_len)
{
	// get the engine buffer point
	std::vector<float> *samples = (std::vector<float> *)vad->vector_samples;
	samples->insert(samples->end(), data, data + data_len);
	VadEngine *vad_engine = (VadEngine *)vad->vad_engine;
	// loop to split the data in CHUNK_SIZE and feed to vad engine
	while (samples->size() > CHUNK_SIZE)
	{
		// copy chunk size data to a temp buffer
		std::vector<float> sample_chunk(samples->begin(), samples->begin() + CHUNK_SIZE);
		// remove already feeded buffer in engine buffer
		samples->erase(samples->begin(), samples->begin() + CHUNK_SIZE);
		vad->data_len = vad->data_len + sample_chunk.size();
		// the possiblity of the chunk is speech
		float poss = vad_engine->predict_possible(sample_chunk);

		update_vad_state(vad, poss);
		// printf("poss=%f,samples->size()=%ld\n", poss, samples->size());
	}
	return vad_get_state(vad);
}

void vad_destroy(ZVAD_OBJ *vad)
{
	delete (VadEngine *)vad->vad_engine;
	((std::vector<float> *)(vad->vector_samples))->clear();
	delete (std::vector<float> *)vad->vector_samples;
	free(vad);
}
