/* DRM (Direct Rendering Manager,  _not_ Digital Restrictions or Rights
Management!) functions.  Modified from code provided at

https://embear.ch/blog/drm-framebuffer   (describes the code)
https://github.com/embear-engineering/drm-framebuffer   */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#ifdef __linux__
#include <drm/drm.h>
#include <drm/drm_mode.h>
#else
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#endif

#include <xf86drm.h>
#include <xf86drmMode.h>

struct type_name {
    unsigned int type;
    const char *name;
};

struct framebuffer {
    int fd;
    uint32_t buffer_id;
    uint16_t res_x;
    uint16_t res_y;
    uint8_t *data;
    uint32_t size;
    struct drm_mode_create_dumb dumb_framebuffer;
    drmModeCrtcPtr crtc;
    drmModeConnectorPtr connector;
    drmModeModeInfoPtr resolution;
};

static const struct type_name connector_type_names[] = {
    { DRM_MODE_CONNECTOR_Unknown, "unknown" },
    { DRM_MODE_CONNECTOR_VGA, "VGA" },
    { DRM_MODE_CONNECTOR_DVII, "DVI-I" },
    { DRM_MODE_CONNECTOR_DVID, "DVI-D" },
    { DRM_MODE_CONNECTOR_DVIA, "DVI-A" },
    { DRM_MODE_CONNECTOR_Composite, "composite" },
    { DRM_MODE_CONNECTOR_SVIDEO, "s-video" },
    { DRM_MODE_CONNECTOR_LVDS, "LVDS" },
    { DRM_MODE_CONNECTOR_Component, "component" },
    { DRM_MODE_CONNECTOR_9PinDIN, "9-pin DIN" },
    { DRM_MODE_CONNECTOR_DisplayPort, "DP" },
    { DRM_MODE_CONNECTOR_HDMIA, "HDMI-A" },
    { DRM_MODE_CONNECTOR_HDMIB, "HDMI-B" },
    { DRM_MODE_CONNECTOR_TV, "TV" },
    { DRM_MODE_CONNECTOR_eDP, "eDP" },
    { DRM_MODE_CONNECTOR_VIRTUAL, "Virtual" },
    { DRM_MODE_CONNECTOR_DSI, "DSI" },
    { DRM_MODE_CONNECTOR_DPI, "DPI" },
};

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

const char *connector_type_name(unsigned int type)
{
    if (type < ARRAY_SIZE(connector_type_names)) {
        return connector_type_names[type].name;
    }

    return "INVALID";
}

void release_framebuffer(struct framebuffer *fb)
{
    if (fb->fd) {
        /* Try to become master again, else we can't set CRTC. Then the current master needs to reset everything. */
        drmSetMaster(fb->fd);
        if (fb->crtc) {
            /* Set back to orignal frame buffer */
            drmModeSetCrtc(fb->fd, fb->crtc->crtc_id, fb->crtc->buffer_id, 0, 0, &fb->connector->connector_id, 1, fb->resolution);
            drmModeFreeCrtc(fb->crtc);
        }
        if (fb->buffer_id)
            drmModeFreeFB(drmModeGetFB(fb->fd, fb->buffer_id));
        /* This will also release resolution */
        if (fb->connector) {
            drmModeFreeConnector(fb->connector);
            fb->resolution = 0;
        }
//      if (fb->dumb_framebuffer.handle)
//          ioctl(fb->fd, DRM_IOCTL_MODE_DESTROY_DUMB, fb->dumb_framebuffer);
        close(fb->fd);
    }
}

#define FRAMEBUFFER_COULD_NOT_OPEN_DEVICE              -1
#define FRAMEBUFFER_COULD_NOT_GET_RESOURCES            -2
#define FRAMEBUFFER_COULD_NOT_FIND_CONNECTOR           -3
#define FRAMEBUFFER_COULD_NOT_FIND_RESOLUTION          -4
#define FRAMEBUFFER_COULD_NOT_CREATE_FRAMEBUFFER       -5
#define FRAMEBUFFER_COULD_NOT_ADD_FRAMEBUFFER_TO_DRM   -6
#define FRAMEBUFFER_COULD_NOT_GET_ENCODER              -7
#define FRAMEBUFFER_MODE_MAP_FRAMEBUFFER_FAILED        -8
#define FRAMEBUFFER_MODE_MAP_FAILED                    -9

int get_framebuffer(const char *dri_device, const char *connector_name, struct framebuffer *fb)
{
    int err;
    int fd;
    drmModeResPtr res;
    drmModeEncoderPtr encoder = 0;

    /* Open the dri device /dev/dri/cardX */
    fd = open(dri_device, O_RDWR);
    if (fd < 0)
        return FRAMEBUFFER_COULD_NOT_OPEN_DEVICE;

    /* Get the resources of the DRM device (connectors, encoders, etc.)*/
    res = drmModeGetResources(fd);
    if (!res) {
        close( fd);
        return FRAMEBUFFER_COULD_NOT_GET_RESOURCES;
    }

    /* Search the connector provided as argument */
    drmModeConnectorPtr connector = 0;
    for (int i = 0; i < res->count_connectors; i++) {
        char name[32];

        connector = drmModeGetConnectorCurrent(fd, res->connectors[i]);
        if (!connector)
            continue;

        snprintf(name, sizeof(name), "%s-%u", connector_type_name(connector->connector_type),
                connector->connector_type_id);

        if (strncmp(name, connector_name, sizeof(name)) == 0)
                break;

        if (strcmp( "def", connector_name) == 0 && connector->count_modes)
                break;

        drmModeFreeConnector(connector);
    }
    drmModeFreeResources( res);

    if (!connector)
        return FRAMEBUFFER_COULD_NOT_FIND_CONNECTOR;

    /* Get the preferred resolution */
    drmModeModeInfoPtr resolution = 0;
    for (int i = 0; i < connector->count_modes; i++) {
            resolution = &connector->modes[i];
            if (resolution->type & DRM_MODE_TYPE_PREFERRED)
                    break;
    }

    if (!resolution) {
        err = FRAMEBUFFER_COULD_NOT_FIND_RESOLUTION;
        goto cleanup;
    }

    fb->dumb_framebuffer.height = resolution->vdisplay;
    fb->dumb_framebuffer.width = resolution->hdisplay;
    fb->dumb_framebuffer.bpp = 32;

    err = ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &fb->dumb_framebuffer);
    if (err) {
        err = FRAMEBUFFER_COULD_NOT_CREATE_FRAMEBUFFER;
        goto cleanup;
    }

    err = drmModeAddFB(fd, resolution->hdisplay, resolution->vdisplay, 24, 32,
            fb->dumb_framebuffer.pitch, fb->dumb_framebuffer.handle, &fb->buffer_id);
    if (err) {
        err = FRAMEBUFFER_COULD_NOT_ADD_FRAMEBUFFER_TO_DRM;
        goto cleanup;
    }

    encoder = drmModeGetEncoder(fd, connector->encoder_id);
    if (!encoder) {
        err = FRAMEBUFFER_COULD_NOT_GET_ENCODER;
        goto cleanup;
    }

    /* Get the crtc settings */
    fb->crtc = drmModeGetCrtc(fd, encoder->crtc_id);

    struct drm_mode_map_dumb mreq;

    memset(&mreq, 0, sizeof(mreq));
    mreq.handle = fb->dumb_framebuffer.handle;

    err = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (err) {
        err = FRAMEBUFFER_MODE_MAP_FRAMEBUFFER_FAILED;
        goto cleanup;
    }

    fb->data = mmap(0, fb->dumb_framebuffer.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
    if (fb->data == MAP_FAILED) {
        err = FRAMEBUFFER_MODE_MAP_FAILED;
        goto cleanup;
    }

    /* Make sure we are not master anymore so that other processes can add new framebuffers as well */
    drmDropMaster(fd);

    fb->fd = fd;
    fb->connector = connector;
    fb->resolution = resolution;

cleanup:
    /* We don't need the encoder and connector anymore so let's free them */
    if (encoder)
        drmModeFreeEncoder(encoder);

    if (err)
        release_framebuffer(fb);

    return err;
}

/* Interface between DRM functions above and PDCursesMod */

static struct framebuffer _drm_framebuffer;

static int init_drm( const char *dri_device, const char *connector_name)
{
    int rval = get_framebuffer( dri_device, connector_name, &_drm_framebuffer);

    if( !rval)
    {
        PDC_fb.framebuf = _drm_framebuffer.data;
        PDC_fb.xres     = _drm_framebuffer.dumb_framebuffer.width;
        PDC_fb.yres     = _drm_framebuffer.dumb_framebuffer.height;
        PDC_fb.bits_per_pixel = 32;
        PDC_fb.line_length = _drm_framebuffer.dumb_framebuffer.pitch;
        PDC_fb.smem_len = 0;        /* unused */
        drmSetMaster(_drm_framebuffer.fd);
        drmModeSetCrtc( _drm_framebuffer.fd, _drm_framebuffer.crtc->crtc_id, 0, 0, 0, NULL, 0, NULL);
        drmModeSetCrtc( _drm_framebuffer.fd, _drm_framebuffer.crtc->crtc_id, _drm_framebuffer.buffer_id, 0, 0, &_drm_framebuffer.connector->connector_id, 1, _drm_framebuffer.resolution);
        drmDropMaster(_drm_framebuffer.fd);
    }
    return( rval);
}

static void close_drm( void)
{
    release_framebuffer( &_drm_framebuffer);
}
