/*
 * Copyright (C) 2010 Google, Inc.
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
#include <sys/stat.h>

#ifdef _MSC_VER
typedef unsigned short mode_t;
#endif

#if (PY_VERSION_HEX < 0x02050000)
typedef int Py_ssize_t;
#endif

#if (PY_VERSION_HEX < 0x02060000)
#define Py_SIZE(ob)             (((PyVarObject*)(ob))->ob_size)
#endif

#if PY_MAJOR_VERSION >= 3
#define PyInt_FromLong PyLong_FromLong
#define PyInt_AsLong PyLong_AsLong
#define PyInt_AS_LONG PyLong_AS_LONG
#define PyString_AS_STRING PyBytes_AS_STRING
#define PyString_AsString PyBytes_AsString
#define PyString_AsStringAndSize PyBytes_AsStringAndSize
#define PyString_Check PyBytes_Check
#define PyString_CheckExact PyBytes_CheckExact
#define PyString_FromStringAndSize PyBytes_FromStringAndSize
#define PyString_FromString PyBytes_FromString
#define PyString_GET_SIZE PyBytes_GET_SIZE
#define PyString_Size PyBytes_Size
#define _PyString_Join _PyBytes_Join
#endif

static PyObject *tree_entry_cls = NULL, *null_entry = NULL,
	*defaultdict_cls = NULL, *int_cls = NULL;
static int block_size;

/**
 * Free an array of PyObject pointers, decrementing any references.
 */
static void free_objects(PyObject **objs, Py_ssize_t n)
{
	Py_ssize_t i;
	for (i = 0; i < n; i++)
		Py_XDECREF(objs[i]);
	PyMem_Free(objs);
}

/**
 * Get the entries of a tree, prepending the given path.
 *
 * :param path: The path to prepend, without trailing slashes.
 * :param path_len: The length of path.
 * :param tree: The Tree object to iterate.
 * :param n: Set to the length of result.
 * :return: A (C) array of PyObject pointers to TreeEntry objects for each path
 *     in tree.
 */
static PyObject **tree_entries(char *path, Py_ssize_t path_len, PyObject *tree,
		Py_ssize_t *n)
{
	PyObject *iteritems, *items, **result = NULL;
	PyObject *old_entry, *name, *sha;
	Py_ssize_t i = 0, name_len, new_path_len;
	char *new_path;

	if (tree == Py_None) {
		*n = 0;
		result = PyMem_New(PyObject*, 0);
		if (!result) {
			PyErr_NoMemory();
			return NULL;
		}
		return result;
	}

	iteritems = PyObject_GetAttrString(tree, "iteritems");
	if (!iteritems)
		return NULL;
	items = PyObject_CallFunctionObjArgs(iteritems, Py_True, NULL);
	Py_DECREF(iteritems);
	if (items == NULL) {
		return NULL;
	}
	/* The C implementation of iteritems returns a list, so depend on that. */
	if (!PyList_Check(items)) {
		PyErr_SetString(PyExc_TypeError,
			"Tree.iteritems() did not return a list");
		return NULL;
	}

	*n = PyList_Size(items);
	result = PyMem_New(PyObject*, *n);
	if (!result) {
		PyErr_NoMemory();
		goto error;
	}
	for (i = 0; i < *n; i++) {
		old_entry = PyList_GetItem(items, i);
		if (!old_entry)
			goto error;
		sha = PyTuple_GetItem(old_entry, 2);
		if (!sha)
			goto error;
		name = PyTuple_GET_ITEM(old_entry, 0);
		name_len = PyString_Size(name);
		if (PyErr_Occurred())
			goto error;

		new_path_len = name_len;
		if (path_len)
			new_path_len += path_len + 1;
		new_path = PyMem_Malloc(new_path_len);
		if (!new_path) {
			PyErr_NoMemory();
			goto error;
		}
		if (path_len) {
			memcpy(new_path, path, path_len);
			new_path[path_len] = '/';
			memcpy(new_path + path_len + 1, PyString_AS_STRING(name), name_len);
		} else {
			memcpy(new_path, PyString_AS_STRING(name), name_len);
		}

#if PY_MAJOR_VERSION >= 3
		result[i] = PyObject_CallFunction(tree_entry_cls, "y#OO", new_path,
			new_path_len, PyTuple_GET_ITEM(old_entry, 1), sha);
#else
		result[i] = PyObject_CallFunction(tree_entry_cls, "s#OO", new_path,
			new_path_len, PyTuple_GET_ITEM(old_entry, 1), sha);
#endif
		PyMem_Free(new_path);
		if (!result[i]) {
			goto error;
		}
	}
	Py_DECREF(items);
	return result;

error:
	if (result)
		free_objects(result, i);
	Py_DECREF(items);
	return NULL;
}

/**
 * Use strcmp to compare the paths of two TreeEntry objects.
 */
static int entry_path_cmp(PyObject *entry1, PyObject *entry2)
{
	PyObject *path1 = NULL, *path2 = NULL;
	int result = 0;

	path1 = PyObject_GetAttrString(entry1, "path");
	if (!path1)
		goto done;

	if (!PyString_Check(path1)) {
		PyErr_SetString(PyExc_TypeError, "path is not a (byte)string");
		goto done;
	}

	path2 = PyObject_GetAttrString(entry2, "path");
	if (!path2)
		goto done;

	if (!PyString_Check(path2)) {
		PyErr_SetString(PyExc_TypeError, "path is not a (byte)string");
		goto done;
	}

	result = strcmp(PyString_AS_STRING(path1), PyString_AS_STRING(path2));

done:
	Py_XDECREF(path1);
	Py_XDECREF(path2);
	return result;
}

static PyObject *py_merge_entries(PyObject *self, PyObject *args)
{
	PyObject *tree1, *tree2, **entries1 = NULL, **entries2 = NULL;
	PyObject *e1, *e2, *pair, *result = NULL;
	Py_ssize_t n1 = 0, n2 = 0, i1 = 0, i2 = 0;
	int path_len;
	char *path_str;
	int cmp;

#if PY_MAJOR_VERSION >= 3
	if (!PyArg_ParseTuple(args, "y#OO", &path_str, &path_len, &tree1, &tree2))
#else
	if (!PyArg_ParseTuple(args, "s#OO", &path_str, &path_len, &tree1, &tree2))
#endif
		return NULL;

	entries1 = tree_entries(path_str, path_len, tree1, &n1);
	if (!entries1)
		goto error;

	entries2 = tree_entries(path_str, path_len, tree2, &n2);
	if (!entries2)
		goto error;

	result = PyList_New(0);
	if (!result)
		goto error;

	while (i1 < n1 && i2 < n2) {
		cmp = entry_path_cmp(entries1[i1], entries2[i2]);
		if (PyErr_Occurred())
			goto error;
		if (!cmp) {
			e1 = entries1[i1++];
			e2 = entries2[i2++];
		} else if (cmp < 0) {
			e1 = entries1[i1++];
			e2 = null_entry;
		} else {
			e1 = null_entry;
			e2 = entries2[i2++];
		}
		pair = PyTuple_Pack(2, e1, e2);
		if (!pair)
			goto error;
		PyList_Append(result, pair);
		Py_DECREF(pair);
	}

	while (i1 < n1) {
		pair = PyTuple_Pack(2, entries1[i1++], null_entry);
		if (!pair)
			goto error;
		PyList_Append(result, pair);
		Py_DECREF(pair);
	}
	while (i2 < n2) {
		pair = PyTuple_Pack(2, null_entry, entries2[i2++]);
		if (!pair)
			goto error;
		PyList_Append(result, pair);
		Py_DECREF(pair);
	}
	goto done;

error:
	Py_XDECREF(result);
	result = NULL;

done:
	if (entries1)
		free_objects(entries1, n1);
	if (entries2)
		free_objects(entries2, n2);
	return result;
}

static PyObject *py_is_tree(PyObject *self, PyObject *args)
{
	PyObject *entry, *mode, *result;
	long lmode;

	if (!PyArg_ParseTuple(args, "O", &entry))
		return NULL;

	mode = PyObject_GetAttrString(entry, "mode");
	if (!mode)
		return NULL;

	if (mode == Py_None) {
		result = Py_False;
		Py_INCREF(result);
	} else {
		lmode = PyInt_AsLong(mode);
		if (lmode == -1 && PyErr_Occurred()) {
			Py_DECREF(mode);
			return NULL;
		}
		result = PyBool_FromLong(S_ISDIR((mode_t)lmode));
	}
	Py_DECREF(mode);
	return result;
}

static int add_hash(PyObject *get, PyObject *set, char *str, int n)
{
	PyObject *str_obj = NULL, *hash_obj = NULL, *value = NULL,
		*set_value = NULL;
	long hash;

	/* It would be nice to hash without copying str into a PyString, but that
	 * isn't exposed by the API. */
	str_obj = PyString_FromStringAndSize(str, n);
	if (!str_obj)
		goto error;
	hash = PyObject_Hash(str_obj);
	if (hash == -1)
		goto error;
	hash_obj = PyInt_FromLong(hash);
	if (!hash_obj)
		goto error;

	value = PyObject_CallFunctionObjArgs(get, hash_obj, NULL);
	if (!value)
		goto error;
	set_value = PyObject_CallFunction(set, "(Ol)", hash_obj,
		PyInt_AS_LONG(value) + n);
	if (!set_value)
		goto error;

	Py_DECREF(str_obj);
	Py_DECREF(hash_obj);
	Py_DECREF(value);
	Py_DECREF(set_value);
	return 0;

error:
	Py_XDECREF(str_obj);
	Py_XDECREF(hash_obj);
	Py_XDECREF(value);
	Py_XDECREF(set_value);
	return -1;
}

static PyObject *py_count_blocks(PyObject *self, PyObject *args)
{
	PyObject *obj, *chunks = NULL, *chunk, *counts = NULL, *get = NULL,
		*set = NULL;
	char *chunk_str, *block = NULL;
	Py_ssize_t num_chunks, chunk_len;
	int i, j, n = 0;
	char c;

	if (!PyArg_ParseTuple(args, "O", &obj))
		goto error;

	counts = PyObject_CallFunctionObjArgs(defaultdict_cls, int_cls, NULL);
	if (!counts)
		goto error;
	get = PyObject_GetAttrString(counts, "__getitem__");
	set = PyObject_GetAttrString(counts, "__setitem__");

	chunks = PyObject_CallMethod(obj, "as_raw_chunks", NULL);
	if (!chunks)
		goto error;
	if (!PyList_Check(chunks)) {
		PyErr_SetString(PyExc_TypeError,
			"as_raw_chunks() did not return a list");
		goto error;
	}
	num_chunks = PyList_GET_SIZE(chunks);
	block = PyMem_New(char, block_size);
	if (!block) {
		PyErr_NoMemory();
		goto error;
	}

	for (i = 0; i < num_chunks; i++) {
		chunk = PyList_GET_ITEM(chunks, i);
		if (!PyString_Check(chunk)) {
			PyErr_SetString(PyExc_TypeError, "chunk is not a string");
			goto error;
		}
		if (PyString_AsStringAndSize(chunk, &chunk_str, &chunk_len) == -1)
			goto error;

		for (j = 0; j < chunk_len; j++) {
			c = chunk_str[j];
			block[n++] = c;
			if (c == '\n' || n == block_size) {
				if (add_hash(get, set, block, n) == -1)
					goto error;
				n = 0;
			}
		}
	}
	if (n && add_hash(get, set, block, n) == -1)
		goto error;

	Py_DECREF(chunks);
	Py_DECREF(get);
	Py_DECREF(set);
	PyMem_Free(block);
	return counts;

error:
	Py_XDECREF(chunks);
	Py_XDECREF(get);
	Py_XDECREF(set);
	Py_XDECREF(counts);
	PyMem_Free(block);
	return NULL;
}

static PyMethodDef py_diff_tree_methods[] = {
	{ "_is_tree", (PyCFunction)py_is_tree, METH_VARARGS, NULL },
	{ "_merge_entries", (PyCFunction)py_merge_entries, METH_VARARGS, NULL },
	{ "_count_blocks", (PyCFunction)py_count_blocks, METH_VARARGS, NULL },
	{ NULL, NULL, 0, NULL }
};

static PyObject *
moduleinit(void)
{
	PyObject *m, *objects_mod = NULL, *diff_tree_mod = NULL;
	PyObject *block_size_obj = NULL;

#if PY_MAJOR_VERSION >= 3
	static struct PyModuleDef moduledef = {
		PyModuleDef_HEAD_INIT,
		"_diff_tree",         /* m_name */
		NULL,                 /* m_doc */
		-1,                   /* m_size */
		py_diff_tree_methods, /* m_methods */
		NULL,                 /* m_reload */
		NULL,                 /* m_traverse */
		NULL,                 /* m_clear*/
		NULL,                 /* m_free */
	};
	m = PyModule_Create(&moduledef);
#else
	m = Py_InitModule("_diff_tree", py_diff_tree_methods);
#endif
	if (!m)
		goto error;

	objects_mod = PyImport_ImportModule("dulwich.objects");
	if (!objects_mod)
		goto error;

	tree_entry_cls = PyObject_GetAttrString(objects_mod, "TreeEntry");
	Py_DECREF(objects_mod);
	if (!tree_entry_cls)
		goto error;

	diff_tree_mod = PyImport_ImportModule("dulwich.diff_tree");
	if (!diff_tree_mod)
		goto error;

	null_entry = PyObject_GetAttrString(diff_tree_mod, "_NULL_ENTRY");
	if (!null_entry)
		goto error;

	block_size_obj = PyObject_GetAttrString(diff_tree_mod, "_BLOCK_SIZE");
	if (!block_size_obj)
		goto error;
	block_size = (int)PyInt_AsLong(block_size_obj);

	if (PyErr_Occurred())
		goto error;

	defaultdict_cls = PyObject_GetAttrString(diff_tree_mod, "defaultdict");
	if (!defaultdict_cls)
		goto error;

	/* This is kind of hacky, but I don't know of a better way to get the
	 * PyObject* version of int. */
	int_cls = PyDict_GetItemString(PyEval_GetBuiltins(), "int");
	if (!int_cls) {
		PyErr_SetString(PyExc_NameError, "int");
		goto error;
	}

	Py_DECREF(diff_tree_mod);

	return m;

error:
	Py_XDECREF(objects_mod);
	Py_XDECREF(diff_tree_mod);
	Py_XDECREF(null_entry);
	Py_XDECREF(block_size_obj);
	Py_XDECREF(defaultdict_cls);
	Py_XDECREF(int_cls);
	return NULL;
}

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC
PyInit__diff_tree(void)
{
	return moduleinit();
}
#else
PyMODINIT_FUNC
init_diff_tree(void)
{
	moduleinit();
}
#endif
