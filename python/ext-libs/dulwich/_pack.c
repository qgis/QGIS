/* 
 * Copyright (C) 2009 Jelmer Vernooij <jelmer@samba.org>
 *
 * Dulwich is dual-licensed under the Apache License, Version 2.0 and the GNU
 * General Public License as public by the Free Software Foundation; version 2.0
 * or (at your option) any later version. You can redistribute it and/or
 * modify it under the terms of either of these two licenses.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * You should have received a copy of the licenses; if not, see
 * <http://www.gnu.org/licenses/> for a copy of the GNU General Public License
 * and <http://www.apache.org/licenses/LICENSE-2.0> for a copy of the Apache
 * License, Version 2.0.
 */

#include <Python.h>
#include <stdint.h>

#if PY_MAJOR_VERSION >= 3
#define PyInt_FromLong PyLong_FromLong
#define PyString_AS_STRING PyBytes_AS_STRING
#define PyString_AsString PyBytes_AsString
#define PyString_Check PyBytes_Check
#define PyString_CheckExact PyBytes_CheckExact
#define PyString_FromStringAndSize PyBytes_FromStringAndSize
#define PyString_FromString PyBytes_FromString
#define PyString_GET_SIZE PyBytes_GET_SIZE
#define PyString_Size PyBytes_Size
#define _PyString_Join _PyBytes_Join
#endif

static PyObject *PyExc_ApplyDeltaError = NULL;

static int py_is_sha(PyObject *sha)
{
	if (!PyString_CheckExact(sha))
		return 0;

	if (PyString_Size(sha) != 20)
		return 0;

	return 1;
}


static size_t get_delta_header_size(uint8_t *delta, size_t *index, size_t length)
{
	size_t size = 0;
	size_t i = 0;
	while ((*index) < length) {
		uint8_t cmd = delta[*index];
		(*index)++;
		size |= (cmd & ~0x80) << i;
		i += 7;
		if (!(cmd & 0x80))
			break;
	}
	return size;
}

static PyObject *py_chunked_as_string(PyObject *py_buf)
{
	if (PyList_Check(py_buf)) {
		PyObject *sep = PyString_FromString("");
		if (sep == NULL) {
			PyErr_NoMemory();
			return NULL;
		}
		py_buf = _PyString_Join(sep, py_buf);
		Py_DECREF(sep);
		if (py_buf == NULL) {
			PyErr_NoMemory();
			return NULL;
		}
	} else if (PyString_Check(py_buf)) {
		Py_INCREF(py_buf);
	} else {
		PyErr_SetString(PyExc_TypeError,
			"src_buf is not a string or a list of chunks");
		return NULL;
	}
    return py_buf;
}

static PyObject *py_apply_delta(PyObject *self, PyObject *args)
{
	uint8_t *src_buf, *delta;
	size_t src_buf_len, delta_len;
	size_t src_size, dest_size;
	size_t outindex = 0;
	size_t index;
	uint8_t *out;
	PyObject *ret, *py_src_buf, *py_delta, *ret_list;

	if (!PyArg_ParseTuple(args, "OO", &py_src_buf, &py_delta))
		return NULL;

	py_src_buf = py_chunked_as_string(py_src_buf);
	if (py_src_buf == NULL)
		return NULL;

	py_delta = py_chunked_as_string(py_delta);
	if (py_delta == NULL) {
		Py_DECREF(py_src_buf);
		return NULL;
	}

	src_buf = (uint8_t *)PyString_AS_STRING(py_src_buf);
	src_buf_len = (size_t)PyString_GET_SIZE(py_src_buf);

	delta = (uint8_t *)PyString_AS_STRING(py_delta);
	delta_len = (size_t)PyString_GET_SIZE(py_delta);

	index = 0;
	src_size = get_delta_header_size(delta, &index, delta_len);
	if (src_size != src_buf_len) {
		PyErr_Format(PyExc_ApplyDeltaError,
					 "Unexpected source buffer size: %lu vs %ld", src_size, src_buf_len);
		Py_DECREF(py_src_buf);
		Py_DECREF(py_delta);
		return NULL;
	}
	dest_size = get_delta_header_size(delta, &index, delta_len);
	ret = PyString_FromStringAndSize(NULL, dest_size);
	if (ret == NULL) {
		PyErr_NoMemory();
		Py_DECREF(py_src_buf);
		Py_DECREF(py_delta);
		return NULL;
	}
	out = (uint8_t *)PyString_AsString(ret);
	while (index < delta_len) {
		uint8_t cmd = delta[index];
		index++;
		if (cmd & 0x80) {
			size_t cp_off = 0, cp_size = 0;
			int i;
			for (i = 0; i < 4; i++) {
				if (cmd & (1 << i)) {
					uint8_t x = delta[index];
					index++;
					cp_off |= x << (i * 8);
				}
			}
			for (i = 0; i < 3; i++) {
				if (cmd & (1 << (4+i))) {
					uint8_t x = delta[index];
					index++;
					cp_size |= x << (i * 8);
				}
			}
			if (cp_size == 0)
				cp_size = 0x10000;
			if (cp_off + cp_size < cp_size ||
				cp_off + cp_size > src_size ||
				cp_size > dest_size)
				break;
			memcpy(out+outindex, src_buf+cp_off, cp_size);
			outindex += cp_size;
			dest_size -= cp_size;
		} else if (cmd != 0) {
			if (cmd > dest_size)
				break;
			memcpy(out+outindex, delta+index, cmd);
			outindex += cmd;
			index += cmd;
			dest_size -= cmd;
		} else {
			PyErr_SetString(PyExc_ApplyDeltaError, "Invalid opcode 0");
			Py_DECREF(ret);
			Py_DECREF(py_delta);
			Py_DECREF(py_src_buf);
			return NULL;
		}
	}
	Py_DECREF(py_src_buf);
	Py_DECREF(py_delta);

	if (index != delta_len) {
		PyErr_SetString(PyExc_ApplyDeltaError, "delta not empty");
		Py_DECREF(ret);
		return NULL;
	}

	if (dest_size != 0) {
		PyErr_SetString(PyExc_ApplyDeltaError, "dest size incorrect");
		Py_DECREF(ret);
		return NULL;
	}

	ret_list = Py_BuildValue("[N]", ret);
	if (ret_list == NULL) {
		Py_DECREF(ret);
		return NULL;
	}
	return ret_list;
}

static PyObject *py_bisect_find_sha(PyObject *self, PyObject *args)
{
	PyObject *unpack_name;
	char *sha;
	int sha_len;
	int start, end;
#if PY_MAJOR_VERSION >= 3
	if (!PyArg_ParseTuple(args, "iiy#O", &start, &end,
			      &sha, &sha_len, &unpack_name))
#else
	if (!PyArg_ParseTuple(args, "iis#O", &start, &end,
			      &sha, &sha_len, &unpack_name))
#endif
		return NULL;

	if (sha_len != 20) {
		PyErr_SetString(PyExc_ValueError, "Sha is not 20 bytes long");
		return NULL;
	}
	if (start > end) {
		PyErr_SetString(PyExc_AssertionError, "start > end");
		return NULL;
	}

	while (start <= end) {
		PyObject *file_sha;
		int i = (start + end)/2;
		int cmp;
		file_sha = PyObject_CallFunction(unpack_name, "i", i);
		if (file_sha == NULL) {
			return NULL;
		}
		if (!py_is_sha(file_sha)) {
			PyErr_SetString(PyExc_TypeError, "unpack_name returned non-sha object");
			Py_DECREF(file_sha);
			return NULL;
		}
		cmp = memcmp(PyString_AsString(file_sha), sha, 20);
		Py_DECREF(file_sha);
		if (cmp < 0)
			start = i + 1;
		else if (cmp > 0)
			end = i - 1;
		else {
			return PyInt_FromLong(i);
		}
	}
	Py_RETURN_NONE;
}


static PyMethodDef py_pack_methods[] = {
	{ "apply_delta", (PyCFunction)py_apply_delta, METH_VARARGS, NULL },
	{ "bisect_find_sha", (PyCFunction)py_bisect_find_sha, METH_VARARGS, NULL },
	{ NULL, NULL, 0, NULL }
};

static PyObject *
moduleinit(void)
{
	PyObject *m;
	PyObject *errors_module;

	errors_module = PyImport_ImportModule("dulwich.errors");
	if (errors_module == NULL)
		return NULL;

	PyExc_ApplyDeltaError = PyObject_GetAttrString(errors_module, "ApplyDeltaError");
	Py_DECREF(errors_module);
	if (PyExc_ApplyDeltaError == NULL)
		return NULL;

#if PY_MAJOR_VERSION >= 3
	static struct PyModuleDef moduledef = {
	  PyModuleDef_HEAD_INIT,
	  "_pack",         /* m_name */
	  NULL,            /* m_doc */
	  -1,              /* m_size */
	  py_pack_methods, /* m_methods */
	  NULL,            /* m_reload */
	  NULL,            /* m_traverse */
	  NULL,            /* m_clear*/
	  NULL,            /* m_free */
	};
	m = PyModule_Create(&moduledef);
#else
	m = Py_InitModule3("_pack", py_pack_methods, NULL);
#endif
	if (m == NULL)
		return NULL;

	return m;
}

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC
PyInit__pack(void)
{
	return moduleinit();
}
#else
PyMODINIT_FUNC
init_pack(void)
{
	moduleinit();
}
#endif
