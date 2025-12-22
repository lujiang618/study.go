// src/vaapi_import.cpp
#include <va/va.h>
#include <va/va_drm.h>
#include <xf86drm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Create a VADisplay from render node (e.g. /dev/dri/renderD128)
VADisplay va_display_from_rendernode(const char* rendernode_path, int* out_drm_fd) {
    int drm_fd = open(rendernode_path, O_RDWR | O_CLOEXEC);
    if (drm_fd < 0) {
        perror("open render node");
        return NULL;
    }
    VADisplay va_dpy = vaGetDisplayDRM(drm_fd);
    if (!va_dpy) { close(drm_fd); return NULL; }
    int major, minor;
    if (vaInitialize(va_dpy, &major, &minor) != VA_STATUS_SUCCESS) {
        fprintf(stderr,"vaInitialize failed\n");
        vaTerminate(va_dpy);
        close(drm_fd);
        return NULL;
    }
    *out_drm_fd = drm_fd;
    return va_dpy;
}

// Import external dmabuf fd into a VASurface
// Note: We assume a single prime_fd that backs the full planar data (driver-dependent).
VASurfaceID va_surface_from_dmabuf(VADisplay va_dpy, int prime_fd, int width, int height, unsigned fourcc,
                                  int pitches[3], int offsets[3], int num_planes) {
    VASurfaceID surf = VA_INVALID_ID;

    VASurfaceAttrib attribs[2];
    memset(attribs,0,sizeof(attribs));

    // memtype attribute
    attribs[0].type = VASurfaceAttribMemoryType;
    attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[0].value.type = VAGenericValueTypeInteger;
    attribs[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;

    // external buffer descriptor attribute
    VASurfaceAttribExternalBuffers extbuf;
    memset(&extbuf,0,sizeof(extbuf));
    extbuf.pixel_format = fourcc; // e.g. VA_FOURCC_NV12
    extbuf.width = width;
    extbuf.height = height;
    extbuf.data_size = pitches[0] * height + (pitches[1] * height/2); // rough estimate for NV12
    extbuf.num_planes = num_planes;
    for (int i=0;i<3;i++){ extbuf.pitches[i] = pitches[i]; extbuf.offsets[i]=offsets[i]; }
    extbuf.buffers = (int*)&prime_fd;
    extbuf.num_buffers = 1;
    extbuf.flags = 0;
    // attach pointer
    attribs[1].type = VASurfaceAttribExternalBufferDescriptor;
    attribs[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[1].value.type = VAGenericValueTypePointer;
    attribs[1].value.value.p = &extbuf;

    VAStatus s = vaCreateSurfaces(va_dpy, VA_RT_FORMAT_YUV420, width, height, &surf, 1, attribs, 2);
    if (s != VA_STATUS_SUCCESS) {
        fprintf(stderr,"vaCreateSurfaces failed: %d\n", s);
        return VA_INVALID_ID;
    }
    // NOTE: after successful creation, ownership/dup of fd semantics depend on driver
    return surf;
}
