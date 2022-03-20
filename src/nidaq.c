#include "nidaq.h"

#include <stdint.h>
#include <string.h>

#include <NIDAQmx.h>

static int find_first_ni_device(const char *device_type_to_find,
                                char *device_name_out,
                                size_t device_name_out_size)
{
  char device_names[128];

  // Get a string of device names
  DAQmxGetSystemInfoAttribute(DAQmx_Sys_DevNames,
                              device_names,
                              SIZEOF_ARRAY(device_names));

  // Split string of device names by comma
  char *device_name = strtok(device_names, ",");
  do
  {
    int size = DAQmxGetDeviceAttribute(device_name,
                                       DAQmx_Dev_ProductType,
                                       NULL);

    char device_type[16];
    DAQmxGetDeviceAttribute(device_name,
                            DAQmx_Dev_ProductType,
                            device_type,
                            size);

    if (strcmp(device_type, device_type_to_find) == 0)
    {
      strncpy(device_name_out, device_name, device_name_out_size);
      return 0;
    }
  } while ((device_name = strtok(NULL, ",")));

  return -1;
}

