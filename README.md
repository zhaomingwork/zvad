<!--
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-09-05 21:20:59
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-09-05 21:55:28
 * @FilePath: /zvad/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
# VAD Wrapper in C for most popular vad models, such as Silero

C Wrapper for most popular vad models (silero, maybe in future webrtc, fsmnvad...).
You can use it in FREESWITCH/UNIMRCP or other environments(unimrcp vad example see ./c_example/mpf_activity_detector.c), and replace the orginal vad module.  

## Interface
zvad.h  provide an uniform vad inferace.
zmodel.h provide a unified interface for different vad models.

## Simple c example in pseudocode

include header #include "../zvad.h" in your code, put libzvad.so in your PATH, and link this library. 
detail example see ./c_example/example.c or ./c_example/mpf_activity_detector.c

### STATE defined for the vad
```bash
	typedef enum
	{
		ZVAD_OBJ_NOINPUT = 0,  // not input
		ZVAD_OBJ_SPEAKING = 1, // now is speaking
		ZVAD_OBJ_SILENCE = 2,  // now is silence
		ZVAD_OBJ_ERROR = 3,    // in error
		ZVAD_OBJ_BEGIN_SPEAKING = 4, // begin to speak
		ZVAD_OBJ_BEGIN_SILENCE = 5,  // begin to silence
	} ZVAD_OBJ_STATE;
```

### pseudocode code for example

```bash
   float buff_float[chunk_size];
   ZVAD_OBJ* new_vad=vad_init(YOUR_MODEL_PATH,16000, 1,0);
   ZVAD_OBJ_STATE state=vad_feed(new_vad,(float*)buff_float,chunk_size);
   printf("now state is %d\n",state);
   vad_destroy(new_vad);
```

## Requirements

Code are tested in the environments

- Ubuntu 20.04
- onnxruntime-linux-x64-1.14.1



 

## Build with gcc and run

   ```bash
   # Build
   cd zvad
   mkdir build && cd build
   cmake .. -DONNXRUNTIME_ROOTDIR=<PATH TO THE ONNXRUNTIME>
   make

   # Run example
   ./c_example/example ../models/silero_vad.onnx ../models/vad_test.wav
   
   ```

# License

This project is licensed under The MIT License.