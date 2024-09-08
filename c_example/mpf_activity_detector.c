/**
 * Description: modified from unimrcp's mpf_activity_detector.c
 * add zvad for the vad module
 * Copyright 2024. All Rights Reserved.
 * Author: Zhao Ming (zhaomingwork@qq.com) 
 */



#include "mpf_activity_detector.h"
#include "apt_log.h"
#include "webrtc/common_audio/vad/include/webrtc_vad.h"
#include "zvad.h"
#include <pthread.h>
#include <unistd.h>

/** Detector states */
typedef enum {
	DETECTOR_STATE_INACTIVITY,           /**< inactivity detected */
	DETECTOR_STATE_ACTIVITY_TRANSITION,  /**< activity detection is in-progress */
	DETECTOR_STATE_ACTIVITY,             /**< activity detected */
	DETECTOR_STATE_INACTIVITY_TRANSITION /**< inactivity detection is in-progress */
} mpf_detector_state_e;

/** Activity detector */
struct mpf_activity_detector_t {
	/* voice activity (silence) level threshold */
	apr_size_t           level_threshold;

	/* period of activity required to complete transition to active state */
	apr_size_t           speech_timeout;
	/* period of inactivity required to complete transition to inactive state */
	apr_size_t           silence_timeout;
	/* noinput timeout */
	apr_size_t           noinput_timeout;

	/* current state */
	mpf_detector_state_e state;
	/* duration spent in current state  */
	apr_size_t           duration;
	
	apr_pool_t *pool; // data buffer used for zvad
	
	ZVAD_OBJ * zvad; // the zvad engine handle
	
	pthread_t thread_id; // thread id
};
/**
 * @description: this thread used for clear zvad when finished
 * @param {void*} p
 * @return {*}
 */
void* destroy_thread(void* p){
   
   
  mpf_activity_detector_t * the_p= (mpf_activity_detector_t *)p;
  ZVAD_OBJ * p_vad=the_p->zvad;
  while(NULL!=the_p)
  {
	  printf("mpf_activity_detector_t is existed!!!!!");
	  sleep(0.1);
  }
  
  vad_destroy(p_vad);
  
 
}





/** Create activity detector */
MPF_DECLARE(mpf_activity_detector_t*) mpf_activity_detector_create(apr_pool_t *pool)
{
	mpf_activity_detector_t *detector = apr_palloc(pool,sizeof(mpf_activity_detector_t));
	detector->level_threshold = 30; /* 0 .. 255 */
	detector->speech_timeout = 200; /* 0.2 s */
	detector->silence_timeout = 200; /* 0.2 s */
	detector->noinput_timeout = 5000; /* 5 s */
	detector->duration = 0;
	detector->state = DETECTOR_STATE_INACTIVITY;
	detector->zvad = vad_init("silero_vad.onnx",16000, 1, 0 , 0.6); // init zvad
	detector->pool = pool;
 
	pthread_create(&detector->thread_id, NULL, destroy_thread, detector);
	return detector;
}

/** Reset activity detector */
MPF_DECLARE(void) mpf_activity_detector_reset(mpf_activity_detector_t *detector)
{
	detector->duration = 0;
	detector->state = DETECTOR_STATE_INACTIVITY;
	vad_reset(detector->zvad);
}

/** Set threshold of voice activity (silence) level */
MPF_DECLARE(void) mpf_activity_detector_level_set(mpf_activity_detector_t *detector, apr_size_t level_threshold)
{
	detector->level_threshold = level_threshold;
}

/** Set noinput timeout */
MPF_DECLARE(void) mpf_activity_detector_noinput_timeout_set(mpf_activity_detector_t *detector, apr_size_t noinput_timeout)
{
	detector->noinput_timeout = noinput_timeout;
}

/** Set timeout required to trigger speech (transition from inactive to active state) */
MPF_DECLARE(void) mpf_activity_detector_speech_timeout_set(mpf_activity_detector_t *detector, apr_size_t speech_timeout)
{
	detector->speech_timeout = speech_timeout;
}

/** Set timeout required to trigger silence (transition from active to inactive state) */
MPF_DECLARE(void) mpf_activity_detector_silence_timeout_set(mpf_activity_detector_t *detector, apr_size_t silence_timeout)
{
	detector->silence_timeout = silence_timeout;
}


static APR_INLINE void mpf_activity_detector_state_change(mpf_activity_detector_t *detector, mpf_detector_state_e state)
{
	detector->duration = 0;
	detector->state = state;
	apt_log(MPF_LOG_MARK,APT_PRIO_INFO,"Activity Detector state changed [%d]",state);
}

 

 

static ZVAD_OBJ_STATE zvad_process(mpf_activity_detector_t *detector, const mpf_frame_t *frame)
{
		apr_size_t samplesCount = frame->codec_frame.size/2;
		int per_ms_frames = 20;
		apr_size_t sampleRate = 8000;  // change to your environment
		size_t samples = sampleRate * per_ms_frames / 1000;
		if (samples == 0) return -1;
		size_t nTotal = (samplesCount / samples);
	    int16_t *input = frame->codec_frame.buffer;
		float * float_input = apr_palloc(detector->pool,sizeof(float)*nTotal);
		for(int i=0;i<samples;i++)
		{
			float_input[i]=((float)input[i])/32768.0;
		}
		ZVAD_OBJ_STATE state=vad_feed(detector->zvad,float_input,samples);
		return state;
}

MPF_DECLARE(mpf_detector_event_e) mpf_activity_detector_process(mpf_activity_detector_t *detector, const mpf_frame_t *frame)
{
    mpf_detector_event_e det_event = MPF_DETECTOR_EVENT_NONE;
    apr_size_t level = 0;
	ZVAD_OBJ_STATE now_state;
    if((frame->type & MEDIA_FRAME_TYPE_AUDIO) == MEDIA_FRAME_TYPE_AUDIO) {     

		  now_state=zvad_process(detector,frame);
 
#if 0
        apt_log(APT_LOG_MARK,APT_PRIO_INFO,"Activity Detector --------------------- %d",now_state);
#endif
    }
	
    if(now_state== ZVAD_OBJ_NOINPUT)
	{
		 
		det_event = MPF_DETECTOR_EVENT_NOINPUT;
	}
    if(now_state== ZVAD_OBJ_SPEAKING && detector->state != DETECTOR_STATE_ACTIVITY)
	{
		mpf_activity_detector_state_change(detector, DETECTOR_STATE_ACTIVITY);
		det_event = MPF_DETECTOR_EVENT_ACTIVITY;
	}
    if(now_state== ZVAD_OBJ_SILENCE && detector->state != DETECTOR_STATE_INACTIVITY)
	{
		mpf_activity_detector_state_change(detector, DETECTOR_STATE_INACTIVITY);
		det_event = MPF_DETECTOR_EVENT_INACTIVITY;
	}
	
	if(detector->state != DETECTOR_STATE_ACTIVITY){
		detector->duration += CODEC_FRAME_TIME_BASE;
		if(detector->duration >= detector->noinput_timeout && now_state== ZVAD_OBJ_NOINPUT) {
                det_event = MPF_DETECTOR_EVENT_NOINPUT;
		}
 
	}
	return det_event;
	
  
}

 
