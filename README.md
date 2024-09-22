
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
   ZVAD_OBJ* new_vad=vad_init(YOUR_MODEL_PATH,sample_rate, 1,0,voice_threshold);
   ZVAD_OBJ_STATE state=vad_feed(new_vad,(float*)buff_float,chunk_size);
   printf("now state is %d\n",state);
   vad_destroy(new_vad);
```

### python example
export python lib by pybind11

```bash
import py_zvad
ZVAD_OBJ=py_zvad.vad_init("PATH/silero_vad.onnx",16000,1,0,0.8)
 
test_chunk=3200
with open("../models/vad_test.wav","rb") as f:
   date_byte=f.read()
   for i in range(0,len(date_byte),test_chunk):
     state=py_zvad.vad_feed(ZVAD_OBJ,date_byte[i:i+test_chunk],test_chunk)
     if state==1: # when detect voice print the time and state
	     print("time ",ZVAD_OBJ.data_len,str(ZVAD_OBJ.data_len/16000),", detected voice")
     else:
        print("time ",ZVAD_OBJ.data_len,str((ZVAD_OBJ.data_len)/16000)+", not voice")

py_zvad.vad_destroy(ZVAD_OBJ)
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
   cmake .. -DONNXRUNTIME_ROOTDIR=<PATH TO THE ONNXRUNTIME> -DPYTHON_EXECUTABLE=$(which python)
   make

   # Run example
   ./c_example/example ../models/silero_vad.onnx ../models/vad_test.wav

   # Run python example
   python ../python_example/silence_trim.py
   
   ```

# License

This project is licensed under The MIT License.