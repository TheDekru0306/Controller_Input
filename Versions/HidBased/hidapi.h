#ifndef DEBBUGJSINPUT_HIDAPI_H
#define DEBBUGJSINPUT_HIDAPI_H

#include <libudev.h>
#include <stdio.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <poll.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/version.h>


static __u32 kernel_version = 0;

static int parse_uevent_info(const char *uevent, int *bus_type,
                             unsigned short *vendor_id, unsigned short *product_id,
                             char **serial_number_utf8, char **product_name_utf8)
{
    char *tmp = strdup(uevent);
    char *saveptr = nullptr;
    char *line;
    char *key;
    char *value;

    int found_id = 0;
    int found_serial = 0;
    int found_name = 0;

    line = strtok_r(tmp, "\n", &saveptr);
    while (line != nullptr) {
        /* line: "KEY=value" */
        key = line;
        value = strchr(line, '=');
        if (!value) {
            goto next_line;
        }
        *value = '\0';
        value++;

        if (strcmp(key, "HID_ID") == 0) {
            /**
             *        type vendor   product
             * HID_ID=0003:000005AC:00008242
             **/
            int ret = sscanf(value, "%x:%hx:%hx", bus_type, vendor_id, product_id);
            if (ret == 3) {
                found_id = 1;
            }
        } else if (strcmp(key, "HID_NAME") == 0) {
            /* The caller has to free the product name */
            *product_name_utf8 = strdup(value);
            found_name = 1;
        } else if (strcmp(key, "HID_UNIQ") == 0) {
            /* The caller has to free the serial number */
            *serial_number_utf8 = strdup(value);
            found_serial = 1;
        }

        next_line:
        line = strtok_r(nullptr, "\n", &saveptr);
    }

    free(tmp);
    return (found_id && found_name && found_serial);
}

inline const char *device_string_names[] = {
        "manufacturer",
        "product",
        "serial",
};

enum device_string_id {
    DEVICE_STRING_MANUFACTURER,
    DEVICE_STRING_PRODUCT,
    DEVICE_STRING_SERIAL,

    DEVICE_STRING_COUNT,
};

static wchar_t *utf8_to_wchar_t(const char *utf8)
{
    wchar_t *ret = nullptr;

    if (utf8) {
        size_t wlen = mbstowcs(nullptr, utf8, 0);
        if ((size_t) -1 == wlen) {
            return wcsdup(L"");
        }
        ret = new wchar_t[wlen + 1];
        mbstowcs(ret, utf8, wlen+1);
        ret[wlen] = 0x0000;
    }

    return ret;
}

static wchar_t *copy_udev_string(struct udev_device *dev, const char *udev_name)
{
    return utf8_to_wchar_t(udev_device_get_sysattr_value(dev, udev_name));
}

struct hid_device_info {
    /** Platform-specific device path */
    char *path;
    /** Device Vendor ID */
    unsigned short vendor_id;
    /** Device Product ID */
    unsigned short product_id;
    /** Serial Number */
    wchar_t *serial_number;
    /** Device Release Number in binary-coded decimal,
        also known as Device Version Number */
    unsigned short release_number;
    /** Manufacturer String */
    wchar_t *manufacturer_string;
    /** Product string */
    wchar_t *product_string;
    /** Usage Page for this Device/Interface
        (Windows/Mac only). */
    unsigned short usage_page;
    /** Usage for this Device/Interface
        (Windows/Mac only).*/
    unsigned short usage;
    /** The USB interface which this logical device
        represents. Valid on both Linux implementations
        in all cases, and valid on the Windows implementation
        only if the device contains more than one interface. */
    int interface_number;

    /** Pointer to the next device */
    struct hid_device_info *next;
};


static int get_hid_report_descriptor(const char *rpt_path, struct hidraw_report_descriptor *rpt_desc)
{
    int rpt_handle;
    ssize_t res;

    rpt_handle = open(rpt_path, O_RDONLY);
    if (rpt_handle < 0) {
        //printf("open failed (%s): %s", rpt_path, strerror(errno));
        return -1;
    }
    memset(rpt_desc, 0x0, sizeof(*rpt_desc));
    res = read(rpt_handle, rpt_desc->value, HID_MAX_DESCRIPTOR_SIZE);
    if (res < 0) {
        //printf("read failed (%s): %s", rpt_path, strerror(errno));
    }
    rpt_desc->size = (__u32) res;

    close(rpt_handle);
    return (int) res;
}

static int get_hid_report_descriptor_from_sysfs(const char *sysfs_path, struct hidraw_report_descriptor *rpt_desc)
{
    int res = -1;
    /* Construct <sysfs_path>/device/report_descriptor */
    size_t rpt_path_len = strlen(sysfs_path) + 25 + 1;
    char* rpt_path = (char*) calloc(1, rpt_path_len);
    snprintf(rpt_path, rpt_path_len, "%s/device/report_descriptor", sysfs_path);

    res = get_hid_report_descriptor(rpt_path, rpt_desc);
    free(rpt_path);

    return res;
}
static int get_hid_item_size(__u8 *report_descriptor, unsigned int pos, __u32 size, int *data_len, int *key_size)
{
    int key = report_descriptor[pos];
    int size_code;

    /*
     * This is a Long Item. The next byte contains the
     * length of the data section (value) for this key.
     * See the HID specification, version 1.11, section
     * 6.2.2.3, titled "Long Items."
     */
    if ((key & 0xf0) == 0xf0) {
        if (pos + 1 < size)
        {
            *data_len = report_descriptor[pos + 1];
            *key_size = 3;
            return 1;
        }
        *data_len = 0; /* malformed report */
        *key_size = 0;
    }

    /*
     * This is a Short Item. The bottom two bits of the
     * key contain the size code for the data section
     * (value) for this key. Refer to the HID
     * specification, version 1.11, section 6.2.2.2,
     * titled "Short Items."
     */
    size_code = key & 0x3;
    switch (size_code) {
        case 0:
        case 1:
        case 2:
            *data_len = size_code;
            *key_size = 1;
            return 1;
        case 3:
            *data_len = 4;
            *key_size = 1;
            return 1;
        default:
            /* Can't ever happen since size_code is & 0x3 */
            *data_len = 0;
            *key_size = 0;
            break;
    };

    /* malformed report */
    return 0;
}

static __u32 get_hid_report_bytes(__u8 *rpt, size_t len, size_t num_bytes, size_t cur)
{
    /* Return if there aren't enough bytes. */
    if (cur + num_bytes >= len)
        return 0;

    if (num_bytes == 0)
        return 0;
    else if (num_bytes == 1)
        return rpt[cur + 1];
    else if (num_bytes == 2)
        return (rpt[cur + 2] * 256 + rpt[cur + 1]);
    else if (num_bytes == 4)
        return (
                rpt[cur + 4] * 0x01000000 +
                rpt[cur + 3] * 0x00010000 +
                rpt[cur + 2] * 0x00000100 +
                rpt[cur + 1] * 0x00000001
        );
    else
        return 0;
}

static int get_next_hid_usage(__u8 *report_descriptor, __u32 size, unsigned int *pos, unsigned short *usage_page, unsigned short *usage)
{
    int data_len, key_size;
    int initial = *pos == 0; /* Used to handle case where no top-level application collection is defined */
    int usage_pair_ready = 0;

    /* Usage is a Local Item, it must be set before each Main Item (Collection) before a pair is returned */
    int usage_found = 0;

    while (*pos < size) {
        int key = report_descriptor[*pos];
        int key_cmd = key & 0xfc;

        /* Determine data_len and key_size */
        if (!get_hid_item_size(report_descriptor, *pos, size, &data_len, &key_size))
            return -1; /* malformed report */

        switch (key_cmd) {
            case 0x4: /* Usage Page 6.2.2.7 (Global) */
                *usage_page = get_hid_report_bytes(report_descriptor, size, data_len, *pos);
                break;

            case 0x8: /* Usage 6.2.2.8 (Local) */
                *usage = get_hid_report_bytes(report_descriptor, size, data_len, *pos);
                usage_found = 1;
                break;

            case 0xa0: /* Collection 6.2.2.4 (Main) */
                /* A Usage Item (Local) must be found for the pair to be valid */
                if (usage_found)
                    usage_pair_ready = 1;

                /* Usage is a Local Item, unset it */
                usage_found = 0;
                break;

            case 0x80: /* Input 6.2.2.4 (Main) */
            case 0x90: /* Output 6.2.2.4 (Main) */
            case 0xb0: /* Feature 6.2.2.4 (Main) */
            case 0xc0: /* End Collection 6.2.2.4 (Main) */
                /* Usage is a Local Item, unset it */
                usage_found = 0;
                break;
        }

        /* Skip over this key and it's associated data */
        *pos += data_len + key_size;

        /* Return usage pair */
        if (usage_pair_ready)
            return 0;
    }

    /* If no top-level application collection is found and usage page/usage pair is found, pair is valid
       https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/top-level-collections */
    if (initial && usage_found)
        return 0; /* success */

    return 1; /* finished processing */
}


inline struct hid_device_info *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;

    struct hid_device_info *root = nullptr; /* return object */
    struct hid_device_info *cur_dev = nullptr;
    struct hid_device_info *prev_dev = nullptr; /* previous device */

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        return nullptr;
    }

    /* Create a list of the devices in the 'hidraw' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    /* For each item, see if it matches the vid/pid, and if so
       create a udev_device record for it */
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *sysfs_path;
        const char *dev_path;
        const char *str;
        struct udev_device *raw_dev; /* The device's hidraw udev node. */
        struct udev_device *hid_dev; /* The device's HID udev node. */
        struct udev_device *usb_dev; /* The device's USB udev node. */
        struct udev_device *intf_dev; /* The device's interface (in the USB sense). */
        unsigned short dev_vid;
        unsigned short dev_pid;
        char *serial_number_utf8 = nullptr;
        char *product_name_utf8 = nullptr;
        int bus_type;
        int result;
        struct hidraw_report_descriptor report_desc;

        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        sysfs_path = udev_list_entry_get_name(dev_list_entry);
        raw_dev = udev_device_new_from_syspath(udev, sysfs_path);
        dev_path = udev_device_get_devnode(raw_dev);

        hid_dev = udev_device_get_parent_with_subsystem_devtype(
                raw_dev,
                "hid",
                nullptr);

        if (!hid_dev) {
            /* Unable to find parent hid device. */
            goto next;
        }

        result = parse_uevent_info(
                udev_device_get_sysattr_value(hid_dev, "uevent"),
                &bus_type,
                &dev_vid,
                &dev_pid,
                &serial_number_utf8,
                &product_name_utf8);

        if (!result) {
            /* parse_uevent_info() failed for at least one field. */
            goto next;
        }

        if (bus_type != BUS_USB && bus_type != BUS_BLUETOOTH) {
            /* We only know how to handle USB and BT devices. */
            goto next;
        }

        /* Check the VID/PID against the arguments */
        if ((vendor_id == 0x0 || vendor_id == dev_vid) &&
            (product_id == 0x0 || product_id == dev_pid)) {
            struct hid_device_info *tmp = new struct hid_device_info;

            /* VID/PID match. Create the record. */

            if (cur_dev) {
                cur_dev->next = tmp;
            }
            else {
                root = tmp;
            }
            prev_dev = cur_dev;
            cur_dev = tmp;

            /* Fill out the record */
            cur_dev->next = nullptr;
            cur_dev->path = dev_path? strdup(dev_path): nullptr;

            /* VID/PID */
            cur_dev->vendor_id = dev_vid;
            cur_dev->product_id = dev_pid;

            /* Serial Number */
            cur_dev->serial_number = utf8_to_wchar_t(serial_number_utf8);

            /* Release Number */
            cur_dev->release_number = 0x0;

            /* Interface Number */
            cur_dev->interface_number = -1;

            switch (bus_type) {
                case BUS_USB:
                    /* The device pointed to by raw_dev contains information about
                       the hidraw device. In order to get information about the
                       USB device, get the parent device with the
                       subsystem/devtype pair of "usb"/"usb_device". This will
                       be several levels up the tree, but the function will find
                       it. */
                    usb_dev = udev_device_get_parent_with_subsystem_devtype(
                            raw_dev,
                            "usb",
                            "usb_device");

                    if (!usb_dev) {
                        /* Free this device */
                        free(cur_dev->serial_number);
                        free(cur_dev->path);
                        free(cur_dev);

                        /* Take it off the device list. */
                        if (prev_dev) {
                            prev_dev->next = nullptr;
                            cur_dev = prev_dev;
                        }
                        else {
                            cur_dev = root = nullptr;
                        }

                        goto next;
                    }

                    /* Manufacturer and Product strings */
                    cur_dev->manufacturer_string = copy_udev_string(usb_dev, device_string_names[DEVICE_STRING_MANUFACTURER]);
                    cur_dev->product_string = copy_udev_string(usb_dev, device_string_names[DEVICE_STRING_PRODUCT]);

                    /* Release Number */
                    str = udev_device_get_sysattr_value(usb_dev, "bcdDevice");
                    cur_dev->release_number = (str)? strtol(str, nullptr, 16): 0x0;

                    /* Get a handle to the interface's udev node. */
                    intf_dev = udev_device_get_parent_with_subsystem_devtype(
                            raw_dev,
                            "usb",
                            "usb_interface");
                    if (intf_dev) {
                        str = udev_device_get_sysattr_value(intf_dev, "bInterfaceNumber");
                        cur_dev->interface_number = (str)? strtol(str, nullptr, 16): -1;
                    }

                    break;

                case BUS_BLUETOOTH:
                    /* Manufacturer and Product strings */
                    cur_dev->manufacturer_string = wcsdup(L"");
                    cur_dev->product_string = utf8_to_wchar_t(product_name_utf8);

                    break;

                default:
                    /* Unknown device type - this should never happen, as we
                     * check for USB and Bluetooth devices above */
                    break;
            }

            /* Usage Page and Usage */
            result = get_hid_report_descriptor_from_sysfs(sysfs_path, &report_desc);
            if (result >= 0) {
                unsigned short page = 0, usage = 0;
                unsigned int pos = 0;
                /*
                 * Parse the first usage and usage page
                 * out of the report descriptor.
                 */
                if (!get_next_hid_usage(report_desc.value, report_desc.size, &pos, &page, &usage)) {
                    cur_dev->usage_page = page;
                    cur_dev->usage = usage;
                }
                /*
                 * Parse any additional usage and usage pages
                 * out of the report descriptor.
                 */
                while (!get_next_hid_usage(report_desc.value, report_desc.size, &pos, &page, &usage)) {
                    /* Create new record for additional usage pairs */
                    tmp = (struct hid_device_info*) calloc(1, sizeof(struct hid_device_info));
                    cur_dev->next = tmp;
                    prev_dev = cur_dev;
                    cur_dev = tmp;
                    /* Update fields */
                    cur_dev->path = strdup(dev_path);
                    cur_dev->vendor_id = dev_vid;
                    cur_dev->product_id = dev_pid;
                    cur_dev->serial_number = prev_dev->serial_number? wcsdup(prev_dev->serial_number): nullptr;
                    cur_dev->release_number = prev_dev->release_number;
                    cur_dev->interface_number = prev_dev->interface_number;
                    cur_dev->manufacturer_string = prev_dev->manufacturer_string? wcsdup(prev_dev->manufacturer_string): nullptr;
                    cur_dev->product_string = prev_dev->product_string? wcsdup(prev_dev->product_string): nullptr;
                    cur_dev->usage_page = page;
                    cur_dev->usage = usage;
                }
            }
        }

        next:
        free(serial_number_utf8);
        free(product_name_utf8);
        udev_device_unref(raw_dev);
        /* hid_dev, usb_dev and intf_dev don't need to be (and can't be)
           unref()d.  It will cause a double-free() error.  I'm not
           sure why.  */
    }
    /* Free the enumerator and udev objects. */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return root;
}

inline int hid_read_timeout(int dev, unsigned char *data, size_t length, int milliseconds)
{
    int bytes_read;

    if (milliseconds >= 0) {
        /* Milliseconds is either 0 (non-blocking) or > 0 (contains
           a valid timeout). In both cases we want to call poll()
           and wait for data to arrive.  Don't rely on non-blocking
           operation (O_NONBLOCK) since some kernels don't seem to
           properly report device disconnection through read() when
           in non-blocking mode.  */
        int ret;
        struct pollfd fds;

        fds.fd = dev;
        fds.events = POLLIN;
        fds.revents = 0;
        ret = poll(&fds, 1, milliseconds);
        if (ret == -1 || ret == 0) {
            /* Error or timeout */
            return ret;
        }
        else {
            /* Check for errors on the file descriptor. This will
               indicate a device disconnection. */
            if (fds.revents & (POLLERR | POLLHUP | POLLNVAL))
                return -1;
        }
    }

    bytes_read = read(dev, data, length);
    if (bytes_read < 0 && (errno == EAGAIN || errno == EINPROGRESS))
        bytes_read = 0;

    return bytes_read;
}

inline int hid_read(int dev, unsigned char *data, size_t length)
{
    return hid_read_timeout(dev, data, length, 0);
}
inline int hid_write(int dev, const unsigned char *data, size_t length)
{
    int bytes_written;

    bytes_written = write(dev, data, length);

    return bytes_written;
}


#endif //DEBBUGJSINPUT_HIDAPI_H
