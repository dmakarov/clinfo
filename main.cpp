/**
 * clinfo --
 *
 *      Program to enumerate and dump all of the OpenCL information for a
 *      machine (or at least for a specific run-time).
 *
 * (Jeremy Sugerman, 13 August 2009)
 */
#include <algorithm>
#include <getopt.h>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <memory>
#include <string>
#include "CL/cl.h"
#endif

using namespace std;

class CL_info {

public:

  CL_info(int argc, char** argv) : dump_image_formats(false)
  {
    static struct option options[] = {
      {"help",          0, nullptr, 'h'},
      {"image-formats", 0, nullptr, 'i'},
      {nullptr,         0, nullptr, 0}};
    int opt;

    while (EOF != (opt = getopt_long(argc, argv, "hi", options, nullptr)))
    {
      switch (opt)
      {
      case 'i':
        dump_image_formats = true;
        break;
      case 'h':
      default:
        usage(argv[0]);
        break;
      }
    }
  }

  void display()
  {
    cl_uint num_platforms;
    auto err = clGetPlatformIDs(0, NULL, &num_platforms);
    check_opencl_status(err, "Unable to query the number of platforms");
    cout << num_platforms << " platform" << (num_platforms == 1 ? ":" : "s:") << endl;
    unique_ptr<cl_platform_id> platform_ids(new cl_platform_id[num_platforms]);
    if (nullptr == platform_ids)
    {
      cerr << "Can't allocate memory for platform IDs (size " << num_platforms * sizeof(cl_platform_id) << ")";
      exit(1);
    }
    err = clGetPlatformIDs(num_platforms, platform_ids.get(), nullptr);
    check_opencl_status(err, "Unable to enumerate the platforms");
    for (cl_uint ii = 0; ii < num_platforms; ++ii)
    {
      print_platform(ii, platform_ids.get()[ii]);
      if (ii + 1 < num_platforms)
        cout << "================================================================================\n";
    }
  }

private:
  bool dump_image_formats;
  static char unknown[25];

  /**
   * usage --
   *
   *      Prints the usage message and exits.
   *
   * Results:
   *      void, but calls exit(1)
   */
  void usage(const char *program)
  {
    cerr << "Usage: " << program << " [options]\n";
    cerr << "Options:\n";
    cerr << "  -h, --help                This message\n";
    cerr << "  -i, --image-formats       Print image formats for each device\n";
    exit(1);
  }

  void check_opencl_status(cl_int err, const string& msg)
  {
    if (CL_SUCCESS != err)
    {
      cerr << msg << ": " << cl_error_str(err) << endl;
      exit(1);
    }
  }

  /**
   * cl_error_str --
   *
   *      Utility function that converts an OpenCL error into a human
   *      readable string.
   *
   * Results:
   *      const char * pointer to a static string.
   */
  const char* cl_error_str(cl_int error)
  {
    static struct {cl_int code; const char *msg;} error_table[] = {
      {CL_SUCCESS,                       "no error"                     },
      {CL_DEVICE_NOT_FOUND,              "device not found"             },
      {CL_DEVICE_NOT_AVAILABLE,          "device not available"         },
      {CL_COMPILER_NOT_AVAILABLE,        "compiler not available"       },
      {CL_MEM_OBJECT_ALLOCATION_FAILURE, "mem object allocation failure"},
      {CL_OUT_OF_RESOURCES,              "out of resources"             },
      {CL_OUT_OF_HOST_MEMORY,            "out of host memory"           },
      {CL_PROFILING_INFO_NOT_AVAILABLE,  "profiling not available"      },
      {CL_MEM_COPY_OVERLAP,              "memcopy overlaps"             },
      {CL_IMAGE_FORMAT_MISMATCH,         "image format mismatch"        },
      {CL_IMAGE_FORMAT_NOT_SUPPORTED,    "image format not supported"   },
      {CL_BUILD_PROGRAM_FAILURE,         "build program failed"         },
      {CL_MAP_FAILURE,                   "map failed"                   },
      {CL_INVALID_VALUE,                 "invalid value"                },
      {CL_INVALID_DEVICE_TYPE,           "invalid device type"          },
      {0, nullptr}};

    for (int ii = 0; error_table[ii].msg != NULL; ++ii)
      if (error_table[ii].code == error)
        return error_table[ii].msg;

    snprintf(unknown, sizeof unknown, "unknown error %d", error);
    return unknown;
  }

  string format_long(uint64_t val)
  {
    string r = to_string(val % 1000);
    while (val > 999)
    {
      auto x = val % 1000;
      val = val / 1000;
      r = to_string(val % 1000) + "," + (x < 100 ? (x < 10 ? string("00") + r : string("0") + r) : r);
    }
    return r;
  }

  /**
   * print_image_format --
   *
   *      dumps all image formats supported by the device.
   *
   * Results:
   *      void.
   */
  void print_image_formats(int device_index,
                           const cl_device_id *devices,
                           cl_mem_flags flags,
                           cl_mem_object_type image_type)
  {
    cl_uint fmt;
    cl_int err;
    cl_context context;
    cl_uint num_image_formats;
    context = clCreateContext(NULL, 1, devices, NULL, NULL, &err);
    if (err != CL_SUCCESS)
    {
      cerr << "\tdevice[" << device_index << "]: Unable to create context: "
           << cl_error_str(err) << "!" << endl;
      return;
    }
    err = clGetSupportedImageFormats(context, flags, image_type, 0, NULL, &num_image_formats);
    if (err != CL_SUCCESS)
    {
      cerr << "\tdevice[" << device_index << "]: Unable to get number of supported image formats: "
           << cl_error_str(err) << "!" << endl;
      return;
    }
    auto image_formats = new cl_image_format[num_image_formats];
    err = clGetSupportedImageFormats(context, flags, image_type, num_image_formats, image_formats, NULL);
    if (err != CL_SUCCESS)
    {
      cerr << "\tdevice[" << device_index << "]: Unable to get supported image formats: "
           << cl_error_str(err) << "!" << endl;
      return;
    }
    for (fmt = 0; fmt < num_image_formats; ++fmt)
    {
      if (fmt > 0) cout << "                                          ";
      switch (image_formats[fmt].image_channel_order)
      {
      case CL_R:             cout << " CL_R            "; break;
      case CL_A:             cout << " CL_A            "; break;
      case CL_RG:            cout << " CL_RG           "; break;
      case CL_RA:            cout << " CL_RA           "; break;
      case CL_RGB:           cout << " CL_RGB          "; break;
      case CL_RGBA:          cout << " CL_RGBA         "; break;
      case CL_BGRA:          cout << " CL_BGRA         "; break;
      case CL_ARGB:          cout << " CL_ARGB         "; break;
      case CL_INTENSITY:     cout << " CL_INTENSITY    "; break;
      case CL_LUMINANCE:     cout << " CL_LUMINANCE    "; break;
      case CL_Rx:            cout << " CL_Rx           "; break;
      case CL_RGx:           cout << " CL_RGx          "; break;
      case CL_RGBx:          cout << " CL_RGBx         "; break;
#ifdef CL_DEPTH
      case CL_DEPTH:         printf(" CL_DEPTH        "); break;
#endif
#ifdef CL_DEPTH_STENCIL
      case CL_DEPTH_STENCIL: printf(" CL_DEPTH_STENCIL"); break;
#endif
      default:               printf(" UKNOWN  %8x", image_formats[fmt].image_channel_order);
      }
      switch (image_formats[fmt].image_channel_data_type)
      {
      case CL_SNORM_INT8:      cout << ", CL_SNORM_INT8\n";      break;
      case CL_SNORM_INT16:     cout << ", CL_SNORM_INT16\n";     break;
      case CL_UNORM_INT8:      cout << ", CL_UNORM_INT8\n";      break;
      case CL_UNORM_INT16:     cout << ", CL_UNORM_INT16\n";     break;
      case CL_UNORM_SHORT_565: cout << ", CL_UNORM_SHORT_565\n"; break;
      case CL_UNORM_SHORT_555: cout << ", CL_UNORM_SHORT_555\n"; break;
      case CL_UNORM_INT_101010:cout << ", CL_UNORM_INT_101010\n";break;
      case CL_SIGNED_INT8:     cout << ", CL_SIGNED_INT8\n";     break;
      case CL_SIGNED_INT16:    cout << ", CL_SIGNED_INT16\n";    break;
      case CL_SIGNED_INT32:    cout << ", CL_SIGNED_INT32\n";    break;
      case CL_UNSIGNED_INT8:   cout << ", CL_UNSIGNED_INT8\n";   break;
      case CL_UNSIGNED_INT16:  cout << ", CL_UNSIGNED_INT16\n";  break;
      case CL_UNSIGNED_INT32:  cout << ", CL_UNSIGNED_INT32\n";  break;
      case CL_HALF_FLOAT:      cout << ", CL_HALF_FLOAT\n";      break;
      case CL_FLOAT:           cout << ", CL_FLOAT\n";           break;
#ifdef CL_UNORM_INT24
      case CL_UNORM_INT24:     printf(", CL_UNORM_INT24\n");     break;
#endif
      default:                 printf(", UKNOWN %8x\n", image_formats[fmt].image_channel_data_type);
      }
    }
    delete[] image_formats;
    if ((err = clReleaseContext(context)) != CL_SUCCESS)
      cerr << "\tdevice[" << device_index << "]: Unable to release context: "
           << cl_error_str(err) << "!" << endl;
  }

  void print_extensions(const char* buf, int width)
  {
    vector<string> words;
    stringstream ss;
    string word;
    ss.str(buf);
    while (ss >> word)
      words.push_back(word);
    sort(words.begin(), words.end());
    cout << words[0] << endl;
    for (vector<string>::size_type ii = 1; ii != words.size(); ++ii)
      cout << setw(width) << " " << words[ii] << endl;
  }

  /**
   * print_device --
   *
   *      Dumps everything about the given device ID.
   *
   * Results:
   *      void.
   */
  void print_device(int device_index, cl_device_id device)
  {
#define LONG_PROPS                           \
    def(VENDOR_ID),                          \
    def(MAX_COMPUTE_UNITS),                  \
    def(MAX_WORK_ITEM_DIMENSIONS),           \
    def(MAX_WORK_GROUP_SIZE),                \
    def(PREFERRED_VECTOR_WIDTH_CHAR),        \
    def(PREFERRED_VECTOR_WIDTH_SHORT),       \
    def(PREFERRED_VECTOR_WIDTH_INT),         \
    def(PREFERRED_VECTOR_WIDTH_LONG),        \
    def(PREFERRED_VECTOR_WIDTH_FLOAT),       \
    def(PREFERRED_VECTOR_WIDTH_DOUBLE),      \
    def(MAX_CLOCK_FREQUENCY),                \
    def(ADDRESS_BITS),                       \
    def(MAX_MEM_ALLOC_SIZE),                 \
    def(IMAGE_SUPPORT),                      \
    def(MAX_READ_IMAGE_ARGS),                \
    def(MAX_WRITE_IMAGE_ARGS),               \
    def(IMAGE2D_MAX_WIDTH),                  \
    def(IMAGE2D_MAX_HEIGHT),                 \
    def(IMAGE3D_MAX_WIDTH),                  \
    def(IMAGE3D_MAX_HEIGHT),                 \
    def(IMAGE3D_MAX_DEPTH),                  \
    def(MAX_SAMPLERS),                       \
    def(MAX_PARAMETER_SIZE),                 \
    def(MEM_BASE_ADDR_ALIGN),                \
    def(MIN_DATA_TYPE_ALIGN_SIZE),           \
    def(GLOBAL_MEM_CACHELINE_SIZE),          \
    def(GLOBAL_MEM_CACHE_SIZE),              \
    def(GLOBAL_MEM_SIZE),                    \
    def(MAX_CONSTANT_BUFFER_SIZE),           \
    def(MAX_CONSTANT_ARGS),                  \
    def(LOCAL_MEM_SIZE),                     \
    def(ERROR_CORRECTION_SUPPORT),           \
    def(PROFILING_TIMER_RESOLUTION),         \
    def(ENDIAN_LITTLE),                      \
    def(AVAILABLE),                          \
    def(COMPILER_AVAILABLE),

#define STR_PROPS                            \
    def(NAME),                               \
    def(VENDOR),                             \
    def(PROFILE),                            \
    def(VERSION),

#define HEX_PROPS                            \
    def(SINGLE_FP_CONFIG),                   \
    def(QUEUE_PROPERTIES),

    static struct {cl_device_info param; const char* name;} longProps[] = {
#define def(X) {CL_DEVICE_##X, #X}
      LONG_PROPS
#undef def
      {0, NULL}
    };
    static struct {cl_device_info param; const char* name;} hexProps[] = {
#define def(X) {CL_DEVICE_##X, #X}
      HEX_PROPS
#undef def
      {0, NULL}
    };
    static struct {cl_device_info param; const char* name;} strProps[] = {
#define def(X) {CL_DEVICE_##X, #X}
      STR_PROPS
#undef def
      {CL_DRIVER_VERSION, "DRIVER_VERSION"},
      {CL_DEVICE_EXTENSIONS, "EXTENSIONS"},
      {0, NULL}
    };
    size_t work_item_sizes[3];
    char buf[65536];
    uint64_t val; /* Avoids unpleasant surprises for some params */
    size_t size;
    cl_int err;

    err = clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof val, &val, NULL);
    if (err == CL_SUCCESS)
    {
      printf("device[%d]: TYPE                          : ", device_index);
      if (val & CL_DEVICE_TYPE_DEFAULT)
      {
        val &= ~CL_DEVICE_TYPE_DEFAULT;
        cout << "Default ";
      }
      if (val & CL_DEVICE_TYPE_CPU)
      {
        val &= ~CL_DEVICE_TYPE_CPU;
        cout << "CPU ";
      }
      if (val & CL_DEVICE_TYPE_GPU)
      {
        val &= ~CL_DEVICE_TYPE_GPU;
        cout << "GPU ";
      }
      if (val & CL_DEVICE_TYPE_ACCELERATOR)
      {
        val &= ~CL_DEVICE_TYPE_ACCELERATOR;
        cout << "Accelerator ";
      }
      if (val != 0)
      {
        printf("Unknown (0x%lx) ", (unsigned long) val);
      }
      cout << endl;
    }
    else
    {
      fprintf(stderr, "device[%d]: Unable to get TYPE: %s!\n", device_index, cl_error_str(err));
    }

    for (int ii = 0; strProps[ii].name != NULL; ++ii)
    {
      err = clGetDeviceInfo(device, strProps[ii].param, sizeof buf, buf, &size);
      if (err != CL_SUCCESS)
      {
        fprintf(stderr, "device[%d]: Unable to get %s: %s!\n", device_index, strProps[ii].name, cl_error_str(err));
        continue;
      }
      if (size > sizeof buf)
      {
        fprintf(stderr, "device[%d]: Large %s (%ld bytes)!  Truncating to %ld!\n",
                device_index, strProps[ii].name, size, sizeof buf);
      }
      cout << "device[" << device_index << "]: " << left << setw(30) << strProps[ii].name << ": ";
      if (string("EXTENSIONS") != strProps[ii].name)
        cout << buf << endl;
      else
        print_extensions(buf, 43);
    }

    err = clGetDeviceInfo(device, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof val, &val, NULL);
    if (err == CL_SUCCESS)
    {
      printf("device[%d]: EXECUTION_CAPABILITIES        : ", device_index);
      if (val & CL_EXEC_KERNEL)
      {
        val &= ~CL_EXEC_KERNEL;
        cout << "Kernel ";
      }
      if (val & CL_EXEC_NATIVE_KERNEL)
      {
        val &= ~CL_EXEC_NATIVE_KERNEL;
        cout << "Native ";
      }
      if (val)
      {
        printf("Unknown (0x%lx) ", (unsigned long) val);
      }
      cout << endl;
    }
    else
    {
      fprintf(stderr, "device[%d]: Unable to get EXECUTION_CAPABILITIES: %s!\n", device_index, cl_error_str(err));
    }

    err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof val, &val, NULL);
    if (err == CL_SUCCESS)
    {
      static const char *cacheTypes[] = { "None", "Read-Only", "Read-Write" };
      static size_t numTypes = sizeof cacheTypes / sizeof cacheTypes[0];

      printf("device[%d]: GLOBAL_MEM_CACHE_TYPE         : %s (%ld)\n",
             device_index, val < numTypes ? cacheTypes[val] : "???", (unsigned long) val);
    }
    else
    {
      fprintf(stderr, "device[%d]: Unable to get GLOBAL_MEM_CACHE_TYPE: %s!\n", device_index, cl_error_str(err));
    }
    err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof val, &val, NULL);
    if (err == CL_SUCCESS)
    {
      static const char* memory_types[] = { "???", "Local", "Global" };
      static size_t numTypes = sizeof memory_types / sizeof memory_types[0];

      cout << "device[" << device_index << "]: CL_DEVICE_LOCAL_MEM_TYPE      : "
           << (val < numTypes ? memory_types[val] : "???") << " (" << val << ")" << endl;
    }
    else
    {
      fprintf(stderr, "device[%d]: Unable to get CL_DEVICE_LOCAL_MEM_TYPE: %s!\n", device_index, cl_error_str(err));
    }

    for (int ii = 0; hexProps[ii].name != NULL; ++ii)
    {
      err = clGetDeviceInfo(device, hexProps[ii].param, sizeof val, &val, &size);
      if (CL_SUCCESS != err)
      {
        fprintf(stderr, "device[%d]: Unable to get %s: %s!\n", device_index, hexProps[ii].name, cl_error_str(err));
        continue;
      }
      if (size > sizeof val)
      {
        fprintf(stderr, "device[%d]: Large %s (%lu bytes)!  Truncating to %lu!\n",
                device_index, hexProps[ii].name, size, sizeof val);
      }
      cout << "device[" << device_index << "]: " << left << setw(30) << hexProps[ii].name
           << ": 0x" << hex << val << dec << endl;
    }

    for (int ii = 0; longProps[ii].name != NULL; ++ii)
    {
      err = clGetDeviceInfo(device, longProps[ii].param, sizeof val, &val, &size);
      if (CL_SUCCESS != err)
      {
        cerr << "device[" << device_index << "]: Unable to get " << longProps[ii].name
             << ": " << cl_error_str(err) << "!" << endl;
        continue;
      }
      if (size > sizeof val)
      {
        cerr << "device[" << device_index << "]: Large " << longProps[ii].name
             << " (" << size << " bytes)!  Truncating to " << sizeof val << "!" << endl;
      }
      cout << "device[" << device_index << "]: " << left << setw(30) << longProps[ii].name
           << ": " << format_long(val) << endl;
    }
    err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof work_item_sizes, work_item_sizes, NULL);
    if (CL_SUCCESS != err)
    {
      cerr << "device[" << device_index << "]: Unable to get MAX_WORK_ITEM_SIZES: "
           << cl_error_str(err) << "!" << endl;
    }
    else
    {
      printf("device[%d]: %-30s: %zd, %zd, %zd\n", device_index, "MAX_WORK_ITEM_SIZES",
             work_item_sizes[0], work_item_sizes[1], work_item_sizes[2]);
    }
    if (dump_image_formats)
    {
      printf("device[%d]: %-30s:", device_index, "IMAGE FORMATS");
      print_image_formats(device_index, &device, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D);
    }
  }

  /**
   * print_platform --
   *
   *      Dumps everything about the given platform ID.
   *
   * Results:
   *      void.
   */
  void print_platform(int index, cl_platform_id platform)
  {
    static struct { cl_platform_info param; const char *name; } props[] = {
      { CL_PLATFORM_NAME,       "name"       },
      { CL_PLATFORM_VENDOR,     "vendor"     },
      { CL_PLATFORM_PROFILE,    "profile"    },
      { CL_PLATFORM_VERSION,    "version"    },
      { CL_PLATFORM_EXTENSIONS, "extensions" },
      { 0, nullptr },
    };
    char buf[65536];
    stringstream ss;
    size_t size;
    cl_int err;

    for (cl_uint ii = 0; props[ii].name != nullptr; ++ii)
    {
      err = clGetPlatformInfo(platform, props[ii].param, sizeof buf, buf, &size);
      ss << "platform[" << index << "]: Unable to get " << props[ii].name;
      check_opencl_status(err, ss.str());
      ss.str(string());
      if (size > sizeof buf)
        cerr << "platform[" << index << "]: Huge " << props[ii].name
             << " (" << size << " bytes)!  Truncating to " << sizeof buf << endl;
      cout << "platform[" << index << "]: " << left << setw(10) << props[ii].name << ": ";
      if (string("extensions") != props[ii].name)
        cout << buf << endl;
      else
        print_extensions(buf, 25);
    }
    cl_uint num_devices;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    ss << "platform[" << index << "]: Unable to query the number of devices";
    check_opencl_status(err, ss.str());
    ss.str(string());
    cout << "platform[" << index << "], " << num_devices << " device" << (num_devices == 1 ? "" : "s") << ":" << endl;
    unique_ptr<cl_device_id> device_ids(new cl_device_id[num_devices]);
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, device_ids.get(), NULL);
    ss << "platform[" << index << "]: Unable to enumerate the devices";
    check_opencl_status(err, ss.str());
    ss.str(string());
    for (cl_uint ii = 0; ii < num_devices; ++ii)
    {
      print_device(ii, *(device_ids.get() + ii));
      if (ii + 1 < num_devices)
        cout << "--------------------------------------------------------------------------------\n";
    }
  }

};

char CL_info::unknown[25];

int main(int argc, char* argv[])
{
  CL_info info(argc, argv);
  info.display();
  return EXIT_SUCCESS;
}
