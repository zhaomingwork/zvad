/**
 * Description:
 * Copyright 2024. All Rights Reserved.
 * Author: Zhao Ming (zhaomingwork@qq.com)
 */
#include "pybind11/numpy.h"
#include <pybind11/pybind11.h>
#include <iostream>
#include "../include/zvad.h"
namespace py = pybind11;

PYBIND11_MODULE(py_zvad, m)
{
	m.doc() = "pybind for zvad";

	m.def("vad_init", &vad_init, py::return_value_policy::reference);
	m.def("vad_destroy", &vad_destroy);
	m.def("vad_feed", [](ZVAD_OBJ *vad, char *data, int data_len)
		  {
			  
			  ZVAD_OBJ_STATE state = vad_feed(vad, data, data_len);
			  return state;
		  });
	py::class_<ZVAD_OBJ>(m, "ZVAD_OBJ")
		.def(py::init<>())
		.def_readwrite("vad_engine", &ZVAD_OBJ::vad_engine)
		.def_readwrite("data_len", &ZVAD_OBJ::data_len);
	py::enum_<ZVAD_OBJ_STATE>(m, "ZVAD_OBJ_STATE")
		.value("ZVAD_OBJ_SPEAKING", ZVAD_OBJ_STATE::ZVAD_OBJ_SPEAKING)
		.value("ZVAD_OBJ_SILENCE", ZVAD_OBJ_STATE::ZVAD_OBJ_SILENCE)
		.export_values();
}