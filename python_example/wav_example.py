##
## Description:
## Copyright 2024. All Rights Reserved.
## Author: Zhao Ming (zhaomingwork@qq.com)
##
import sys
sys.path.append("./pybind/")
print("sys",sys.path)
import py_zvad

ZVAD_OBJ=py_zvad.vad_init("../models/silero_vad.onnx",16000,1,0,0.8)
 
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
 

