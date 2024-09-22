##
## Description:
## Copyright 2024. All Rights Reserved.
## Author: Zhao Ming (zhaomingwork@qq.com)
##

import sys
sys.path.append("./pybind/")

import py_zvad
import soundfile as sf
import wave
import torch,torchaudio
import numpy as np
 
 
def trim_silence(wave_float):
   ZVAD_OBJ=py_zvad.vad_init("../models/silero_vad.onnx",16000,1,0,0.1)
   date_byte=b''.join((wave_float*32767).astype(np.int16))
   test_chunk=1024
   remove_silence=[]
   beforebuf=[]
   for i in range(0,len(date_byte),test_chunk):
     state=py_zvad.vad_feed(ZVAD_OBJ,date_byte[i:i+test_chunk],test_chunk)
     

     if state==1: # when detect voice print the time and state
          if len(beforebuf)>0:
              remove_silence.append(b''.join(beforebuf)[-1600*3:])
          remove_silence.append(date_byte[i:i+test_chunk])
         
          beforebuf=[]
     else:
          
          beforebuf.append(date_byte[i:i+test_chunk])
   removed_bytes=b''.join(remove_silence)
   the_array=np.fromstring(removed_bytes, dtype=np.int16).astype(np.float32)/32768
   py_zvad.vad_destroy(ZVAD_OBJ)
   return the_array

with open("../models/vad_test.wav", "rb") as wave_file:
    wave_file.read(42)
    tmpdata=wave_file.read()
 
    the_array=np.fromstring(tmpdata, dtype=np.int16)/32767
    
    the_array=trim_silence(the_array)
 
    the_array=torch.from_numpy(the_array).unsqueeze(0)
    torchaudio.save("../python_example/removed_example.wav", the_array, 16000)
    