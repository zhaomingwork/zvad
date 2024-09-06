
/**
 * Description:
 * Copyright 2024. All Rights Reserved.
 * Author: Zhao Ming (zhaomingwork@qq.com)
 */

#ifndef ZVAD_H
#define ZVAD_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
	typedef enum
	{
		ZVAD_OBJ_NOINPUT = 0,
		ZVAD_OBJ_SPEAKING = 1,
		ZVAD_OBJ_SILENCE = 2,
		ZVAD_OBJ_ERROR = 3,
		ZVAD_OBJ_BEGIN_SPEAKING = 4,
		ZVAD_OBJ_BEGIN_SILENCE = 5,
	} ZVAD_OBJ_STATE;
	typedef struct ZVAD_OBJ_T
	{
		int last_speaking_len;
		int last_silence_len;
		ZVAD_OBJ_STATE last_state;
		void *vad_engine;
		void *vector_samples; // buffer for feed data
		float new_state_ave;
		int data_len; // total buffer size that was feeded to engine
		int sample_rate;
		int channels;
		int mode;

	} ZVAD_OBJ;

	/**
	 * @description: init engine for ZVAD
	 * @param {int} sample_rate
	 * @param {int} channels
	 * @param {int} mode
	 * @return {*}
	 */
	ZVAD_OBJ *vad_init(char *model_path, int sample_rate, int channels, int mode);

	/**
	 * @description: reset the state of the ZVAD
	 * @param {ZVAD_OBJ} *vad
	 * @return {*}
	 */
	void vad_reset(ZVAD_OBJ *vad);
	/**
	 * @description: release the ZVAD engine
	 * @param {ZVAD_OBJ} *vad
	 * @return {*}
	 */
	void vad_destroy(ZVAD_OBJ *vad);
	/**
	 * @description:
	 * @param {ZVAD_OBJ} *vad
	 * @param {float} *data
	 * @param {int} data_len
	 * @return {*}
	 */
	ZVAD_OBJ_STATE vad_feed(ZVAD_OBJ *vad, float *data, int data_len);
	ZVAD_OBJ_STATE vad_get_state(ZVAD_OBJ *vad);
	// int sil_vad_state(ZVAD_OBJ* vad);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // ZVAD_H
