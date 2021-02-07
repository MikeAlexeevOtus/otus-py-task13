#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include "deviceapps.pb-c.h"

#define MAGIC  0xFFFFFFFF
#define DEVICE_APPS_TYPE 1

typedef struct pbheader_s {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
} pbheader_t;
#define PBHEADER_INIT {MAGIC, 0, 0}


// https://github.com/protobuf-c/protobuf-c/wiki/Examples
void fill_device_app(DeviceApps *msg, PyObject* item) {
    PyObject* device_data = PyDict_GetItemString(item, "device");

    const char *device_id = PyString_AsString(PyDict_GetItemString(device_data, "id"));
    const char *device_type = PyString_AsString(PyDict_GetItemString(device_data, "type"));
    msg->device->has_id = 1;
    msg->device->id.data = (uint8_t*)device_id;
    msg->device->id.len = strlen(device_id);
    msg->device->has_type = 1;
    msg->device->type.data = (uint8_t*)device_type;
    msg->device->type.len = strlen(device_type);

    PyObject* lat = PyDict_GetItemString(item, "lat");
    PyObject* lon = PyDict_GetItemString(item, "lon");
    PyObject* apps = PyDict_GetItemString(item, "apps");
    if (lat != NULL) {
        msg->has_lat = 1;
        msg->lat = PyFloat_AsDouble(lat);
    }
    if (lat != NULL) {
        msg->has_lon = 1;
        msg->lon = PyFloat_AsDouble(lon);
    }

    msg->n_apps = PyList_Size(apps);
    // memory free is done in free_device_app
    msg->apps = malloc(sizeof(uint32_t) * msg->n_apps);

    for (int i=0; i < msg->n_apps; i++) {
        msg->apps[i] = (uint32_t) PyLong_AsLong(PyList_GetItem(apps, i));
    }
}

void free_device_app(DeviceApps *msg) {
    free(msg->apps);
}


void example() {
    DeviceApps msg = DEVICE_APPS__INIT;
    DeviceApps__Device device = DEVICE_APPS__DEVICE__INIT;

    void *buf;
    unsigned len;

    char *device_id = "e7e1a50c0ec2747ca56cd9e1558c0d7c";
    char *device_type = "idfa";
    device.has_id = 1;
    device.id.data = (uint8_t*)device_id;
    device.id.len = strlen(device_id);
    device.has_type = 1;
    device.type.data = (uint8_t*)device_type;
    device.type.len = strlen(device_type);
    msg.device = &device;

    msg.has_lat = 1;
    msg.lat = 67.7835424444;
    msg.has_lon = 1;
    msg.lon = -22.8044005471;

    msg.n_apps = 3;
    msg.apps = malloc(sizeof(uint32_t) * msg.n_apps);
    msg.apps[0] = 42;
    msg.apps[1] = 43;
    msg.apps[2] = 44;
    len = device_apps__get_packed_size(&msg);

    buf = malloc(len);
    device_apps__pack(&msg, buf);

    fprintf(stderr,"Writing %d serialized bytes\n",len); // See the length of message
    fwrite(buf, len, 1, stdout); // Write to stdout to allow direct command line piping

    free(msg.apps);
    free(buf);
}

// Read iterator of Python dicts
// Pack them to DeviceApps protobuf and write to file with appropriate header
// Return number of written bytes as Python integer
static PyObject* py_deviceapps_xwrite_pb(PyObject* self, PyObject* args) {
    const char* path;
    PyObject* o;
    PyObject *iterator;
    PyObject *item;
    size_t total_written_bytes = 0;
    gzFile out;

    if (!PyArg_ParseTuple(args, "Os", &o, &path))
        return NULL;

    iterator = PyObject_GetIter(o);
    if (iterator == NULL) {
        return NULL;
    }
    out = gzopen(path, "wb");
    if (out == NULL) {
        return NULL;
    }

    while ((item = PyIter_Next(iterator))) {
        pbheader_t header = PBHEADER_INIT;
        void *buf;

        DeviceApps msg = DEVICE_APPS__INIT;
        DeviceApps__Device device = DEVICE_APPS__DEVICE__INIT;
        msg.device = &device;
        fill_device_app(&msg, item);

        header.length = device_apps__get_packed_size(&msg);
        header.type = DEVICE_APPS_TYPE;
        buf = malloc(header.length);
        device_apps__pack(&msg, buf);

        total_written_bytes += gzwrite(out, &header, sizeof(pbheader_t));
        total_written_bytes += gzwrite(out, buf, header.length);

        free(buf);
        free_device_app(&msg);

        Py_DECREF(item);
    }
    gzclose(out);

    Py_DECREF(iterator);
    return PyLong_FromSsize_t(total_written_bytes);
}

// Unpack only messages with type == DEVICE_APPS_TYPE
// Return iterator of Python dicts
static PyObject* py_deviceapps_xread_pb(PyObject* self, PyObject* args) {
    const char* path;

    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;

    printf("Read from: %s\n", path);
    Py_RETURN_NONE;
}


static PyMethodDef PBMethods[] = {
     {"deviceapps_xwrite_pb", py_deviceapps_xwrite_pb, METH_VARARGS, "Write serialized protobuf to file fro iterator"},
     {"deviceapps_xread_pb", py_deviceapps_xread_pb, METH_VARARGS, "Deserialize protobuf from file, return iterator"},
     {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC initpb(void) {
     (void) Py_InitModule("pb", PBMethods);
}
