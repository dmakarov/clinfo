/*
 * clInfo.c --
 *
 *      Program to enumerate and dump all of the OpenCL information for a
 *      machine (or at least for a specific run-time).
 *
 * (Jeremy Sugerman, 13 August 2009)
 */
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <OpenCL/opencl.h>
#else
#include "CL/cl.h"
#endif

/*
 * Usage --
 *
 *      Prints the usage message and exits.
 *
 * Results:
 *      void, but calls exit(1)...
 */
static void
Usage(const char *progName) {
   Warning("Usage: %s [options]\n", progName);
   Warning("Options:\n");
   Warning("  -h, --help                This message\n");

   exit(1);
}

/*
 * ParseOpts --
 *
 *      Converts the commandline parameters into their internal
 *      representation.
 *
 * Results:
 *      void, opts is initialized.
 */
static void
ParseOpts( int argc, char *argv[] )
{
  static struct option longOptions[] = { { "help", 0, 0, 'h' }, };
  int opt;

  while ( EOF != ( opt = getopt_long( argc, argv, "h", longOptions, NULL ) ) )
  {
    switch ( opt )
    {
    case 'h':
    default:
      Usage(argv[0]);
      break;
    }
  }
}

/*
 * CLErrString --
 *
 *      Utility function that converts an OpenCL status into a human
 *      readable string.
 *
 * Results:
 *      const char * pointer to a static string.
 */
static const char *
CLErrString(cl_int status)
{
   static struct { cl_int code; const char *msg; } error_table[] = {
      { CL_SUCCESS, "success" },
      { CL_DEVICE_NOT_FOUND, "device not found", },
      { CL_DEVICE_NOT_AVAILABLE, "device not available", },
      { CL_COMPILER_NOT_AVAILABLE, "compiler not available", },
      { CL_MEM_OBJECT_ALLOCATION_FAILURE, "mem object allocation failure", },
      { CL_OUT_OF_RESOURCES, "out of resources", },
      { CL_OUT_OF_HOST_MEMORY, "out of host memory", },
      { CL_PROFILING_INFO_NOT_AVAILABLE, "profiling not available", },
      { CL_MEM_COPY_OVERLAP, "memcopy overlaps", },
      { CL_IMAGE_FORMAT_MISMATCH, "image format mismatch", },
      { CL_IMAGE_FORMAT_NOT_SUPPORTED, "image format not supported", },
      { CL_BUILD_PROGRAM_FAILURE, "build program failed", },
      { CL_MAP_FAILURE, "map failed", },
      { CL_INVALID_VALUE, "invalid value", },
      { CL_INVALID_DEVICE_TYPE, "invalid device type", },
      { 0, NULL },
   };
   static char unknown[25];
   int ii;

   for (ii = 0; error_table[ii].msg != NULL; ii++) {
      if (error_table[ii].code == status) {
         return error_table[ii].msg;
      }
   }

   snprintf(unknown, sizeof unknown, "unknown error %d", status);
   return unknown;
}

static void
printImageFormats( int device_index, const cl_device_id *devices, cl_mem_flags flags, cl_mem_object_type image_type )
{
  cl_uint fmt;
  cl_int status;
  cl_image_format *image_formats;
  cl_context context;
  cl_uint num_image_formats;
  context = clCreateContext( NULL, 1, devices, NULL, NULL, &status );
  if ( status != CL_SUCCESS )
  {
    fprintf( stderr, "\tdevice[%d]: Unable to create context: %s!\n", device_index, CLErrString( status ) );
    return;
  }
  status = clGetSupportedImageFormats( context, flags, image_type, 0, NULL, &num_image_formats );
  if ( status != CL_SUCCESS )
  {
    fprintf( stderr, "\tdevice[%d]: Unable to get number of supported image formats: %s!\n",
             device_index, CLErrString( status ) );
    return;
  }
  image_formats = (cl_image_format*) malloc( num_image_formats * sizeof(cl_image_format) );
  status = clGetSupportedImageFormats( context, flags, image_type, num_image_formats, image_formats, NULL );
  if ( status != CL_SUCCESS )
  {
    fprintf( stderr, "\tdevice[%d]: Unable to get supported image formats: %s!\n",
             device_index, CLErrString( status ) );
    return;
  }
  for ( fmt = 0; fmt < num_image_formats; ++fmt )
  {
    if ( fmt > 0 )
      printf( "                                                  " );
    switch ( image_formats[fmt].image_channel_order )
    {
    case CL_R:             printf( " [CL_R            " ); break;
    case CL_A:             printf( " [CL_A            " ); break;
    case CL_RG:            printf( " [CL_RG           " ); break;
    case CL_RA:            printf( " [CL_RA           " ); break;
    case CL_RGB:           printf( " [CL_RGB          " ); break;
    case CL_RGBA:          printf( " [CL_RGBA         " ); break;
    case CL_BGRA:          printf( " [CL_BGRA         " ); break;
    case CL_ARGB:          printf( " [CL_ARGB         " ); break;
    case CL_INTENSITY:     printf( " [CL_INTENSITY    " ); break;
    case CL_LUMINANCE:     printf( " [CL_LUMINANCE    " ); break;
    case CL_Rx:            printf( " [CL_Rx           " ); break;
    case CL_RGx:           printf( " [CL_RGx          " ); break;
    case CL_RGBx:          printf( " [CL_RGBx         " ); break;
#ifdef CL_DEPTH
    case CL_DEPTH:         printf( " [CL_DEPTH        " ); break;
#endif
#ifdef CL_DEPTH_STENCIL
    case CL_DEPTH_STENCIL: printf( " [CL_DEPTH_STENCIL" ); break;
#endif
    default:               printf( " [UKNOWN  %8x", image_formats[fmt].image_channel_order );
    }
    switch ( image_formats[fmt].image_channel_data_type )
    {
    case CL_SNORM_INT8:      printf( ", CL_SNORM_INT8]\n" );      break;
    case CL_SNORM_INT16:     printf( ", CL_SNORM_INT16]\n" );     break;
    case CL_UNORM_INT8:      printf( ", CL_UNORM_INT8]\n" );      break;
    case CL_UNORM_INT16:     printf( ", CL_UNORM_INT16]\n" );     break;
    case CL_UNORM_SHORT_565: printf( ", CL_UNORM_SHORT_565]\n" ); break;
    case CL_UNORM_SHORT_555: printf( ", CL_UNORM_SHORT_555]\n" ); break;
    case CL_UNORM_INT_101010:printf( ", CL_UNORM_INT_101010]\n" );break;
    case CL_SIGNED_INT8:     printf( ", CL_SIGNED_INT8]\n" );     break;
    case CL_SIGNED_INT16:    printf( ", CL_SIGNED_INT16]\n" );    break;
    case CL_SIGNED_INT32:    printf( ", CL_SIGNED_INT32]\n" );    break;
    case CL_UNSIGNED_INT8:   printf( ", CL_UNSIGNED_INT8]\n" );   break;
    case CL_UNSIGNED_INT16:  printf( ", CL_UNSIGNED_INT16]\n" );  break;
    case CL_UNSIGNED_INT32:  printf( ", CL_UNSIGNED_INT32]\n" );  break;
    case CL_HALF_FLOAT:      printf( ", CL_HALF_FLOAT]\n" );      break;
    case CL_FLOAT:           printf( ", CL_FLOAT]\n" );           break;
#ifdef CL_UNORM_INT24
    case CL_UNORM_INT24:     printf( ", CL_UNORM_INT24]\n" );     break;
#endif
    default:                 printf( ", UKNOWN %8x]\n", image_formats[fmt].image_channel_data_type );
    }
  }
  free( image_formats );
  if ( ( status = clReleaseContext( context ) ) != CL_SUCCESS )
    fprintf( stderr, "\tdevice[%d]: Unable to release context: %s!\n", device_index, CLErrString( status ) );
}

/**
 * PrintDevice --
 *
 *      Dumps everything about the given device ID.
 *
 * Results:
 *      void.
 */
static void
PrintDevice( int device_index, cl_device_id device )
{
#define LONG_PROPS \
  defn(VENDOR_ID), \
  defn(MAX_COMPUTE_UNITS), \
  defn(MAX_WORK_ITEM_DIMENSIONS), \
  defn(MAX_WORK_GROUP_SIZE), \
  defn(PREFERRED_VECTOR_WIDTH_CHAR), \
  defn(PREFERRED_VECTOR_WIDTH_SHORT), \
  defn(PREFERRED_VECTOR_WIDTH_INT), \
  defn(PREFERRED_VECTOR_WIDTH_LONG), \
  defn(PREFERRED_VECTOR_WIDTH_FLOAT), \
  defn(PREFERRED_VECTOR_WIDTH_DOUBLE), \
  defn(MAX_CLOCK_FREQUENCY), \
  defn(ADDRESS_BITS), \
  defn(MAX_MEM_ALLOC_SIZE), \
  defn(IMAGE_SUPPORT), \
  defn(MAX_READ_IMAGE_ARGS), \
  defn(MAX_WRITE_IMAGE_ARGS), \
  defn(IMAGE2D_MAX_WIDTH), \
  defn(IMAGE2D_MAX_HEIGHT), \
  defn(IMAGE3D_MAX_WIDTH), \
  defn(IMAGE3D_MAX_HEIGHT), \
  defn(IMAGE3D_MAX_DEPTH), \
  defn(MAX_SAMPLERS), \
  defn(MAX_PARAMETER_SIZE), \
  defn(MEM_BASE_ADDR_ALIGN), \
  defn(MIN_DATA_TYPE_ALIGN_SIZE), \
  defn(GLOBAL_MEM_CACHELINE_SIZE), \
  defn(GLOBAL_MEM_CACHE_SIZE), \
  defn(GLOBAL_MEM_SIZE), \
  defn(MAX_CONSTANT_BUFFER_SIZE), \
  defn(MAX_CONSTANT_ARGS), \
  defn(LOCAL_MEM_SIZE), \
  defn(ERROR_CORRECTION_SUPPORT), \
  defn(PROFILING_TIMER_RESOLUTION), \
  defn(ENDIAN_LITTLE), \
  defn(AVAILABLE), \
  defn(COMPILER_AVAILABLE),

#define STR_PROPS \
  defn(NAME), \
  defn(VENDOR), \
  defn(PROFILE), \
  defn(VERSION), \
  defn(EXTENSIONS),

#define HEX_PROPS \
   defn(SINGLE_FP_CONFIG), \
   defn(QUEUE_PROPERTIES),


/* XXX For completeness, it'd be nice to dump this one, too. */
#define WEIRD_PROPS \
   CL_DEVICE_MAX_WORK_ITEM_SIZES,

   static struct { cl_device_info param; const char *name; } longProps[] = {
#define defn(X) { CL_DEVICE_##X, #X }
      LONG_PROPS
#undef defn
      { 0, NULL },
   };
   static struct { cl_device_info param; const char *name; } hexProps[] = {
#define defn(X) { CL_DEVICE_##X, #X }
      HEX_PROPS
#undef defn
      { 0, NULL },
   };
   static struct { cl_device_info param; const char *name; } strProps[] = {
#define defn(X) { CL_DEVICE_##X, #X }
      STR_PROPS
#undef defn
      { CL_DRIVER_VERSION, "DRIVER_VERSION" },
      { 0, NULL },
   };
   cl_int status;
   size_t size;
   char buf[65536];
   char *word;
   uint64_t val; /* Avoids unpleasant surprises for some params */
   int ii;
   for ( ii = 0; strProps[ii].name != NULL; ++ii )
   {
      status = clGetDeviceInfo( device, strProps[ii].param, sizeof buf, buf, &size );
      if ( status != CL_SUCCESS )
      {
         fprintf( stderr, "\tdevice[%d]: Unable to get %s: %s!\n",
                  device_index, strProps[ii].name, CLErrString(status) );
         continue;
      }
      if (size > sizeof buf)
      {
        fprintf( stderr, "\tdevice[%d]: Large %s (%ld bytes)!  Truncating to %ld!\n",
                 device_index, strProps[ii].name, size, sizeof(buf) );
      }
      if ( strcmp( "EXTENSIONS", strProps[ii].name ) )
        printf( "\tdevice[%d]: %-30s: %s\n", device_index, strProps[ii].name, buf );
      else
      {
        printf( "\tdevice[%d]: %-30s: %s\n", device_index, strProps[ii].name, ( word = strtok( buf, " " ) ) );
        for ( word = strtok( NULL, " " ); word; word = strtok( NULL, " " ) )
          printf( "%50s %s\n", "", word );
      }
   }

   status = clGetDeviceInfo( device, CL_DEVICE_TYPE, sizeof(val), &val, NULL );
   if (status == CL_SUCCESS) {
     printf("\tdevice[%d]: TYPE                          : ", device_index );
     if (val & CL_DEVICE_TYPE_DEFAULT) {
       val &= ~CL_DEVICE_TYPE_DEFAULT;
       printf("Default ");
     }
     if (val & CL_DEVICE_TYPE_CPU) {
       val &= ~CL_DEVICE_TYPE_CPU;
       printf("CPU ");
     }
     if (val & CL_DEVICE_TYPE_GPU) {
       val &= ~CL_DEVICE_TYPE_GPU;
       printf("GPU ");
     }
     if (val & CL_DEVICE_TYPE_ACCELERATOR) {
       val &= ~CL_DEVICE_TYPE_ACCELERATOR;
       printf("Accelerator ");
     }
     if (val != 0) {
       printf( "Unknown (0x%lx) ", (unsigned long) val );
      }
     printf( "\n" );
   } else {
     fprintf( stderr, "\tdevice[%d]: Unable to get TYPE: %s!\n", device_index, CLErrString(status) );
   }

   status = clGetDeviceInfo( device, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(val), &val, NULL);
   if (status == CL_SUCCESS) {
     printf("\tdevice[%d]: EXECUTION_CAPABILITIES        : ", device_index );
     if (val & CL_EXEC_KERNEL) {
       val &= ~CL_EXEC_KERNEL;
       printf("Kernel ");
     }
     if (val & CL_EXEC_NATIVE_KERNEL) {
       val &= ~CL_EXEC_NATIVE_KERNEL;
       printf("Native ");
     }
     if (val) {
       printf( "Unknown (0x%lx) ", (unsigned long) val );
     }
     printf("\n");
   } else {
     fprintf( stderr, "\tdevice[%d]: Unable to get EXECUTION_CAPABILITIES: %s!\n",
              device_index, CLErrString(status));
   }

   status = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof val, &val, NULL);
   if ( status == CL_SUCCESS )
   {
     static const char *cacheTypes[] = { "None", "Read-Only", "Read-Write" };
     static size_t numTypes = sizeof cacheTypes / sizeof cacheTypes[0];

     printf( "\tdevice[%d]: GLOBAL_MEM_CACHE_TYPE         : %s (%ld)\n",
             device_index, val < numTypes ? cacheTypes[val] : "???", (unsigned long) val );
   }
   else
   {
     fprintf( stderr, "\tdevice[%d]: Unable to get GLOBAL_MEM_CACHE_TYPE: %s!\n", device_index, CLErrString(status));
   }
   status = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof val, &val, NULL);
   if (status == CL_SUCCESS)
   {
     static const char *lmemTypes[] = { "???", "Local", "Global" };
     static size_t numTypes = sizeof lmemTypes / sizeof lmemTypes[0];

     printf( "\tdevice[%d]: CL_DEVICE_LOCAL_MEM_TYPE      : %s (%" PRIu64 ")\n",
             device_index, val < numTypes ? lmemTypes[val] : "???", val );
   }
   else
   {
     fprintf( stderr, "\tdevice[%d]: Unable to get CL_DEVICE_LOCAL_MEM_TYPE: %s!\n", device_index, CLErrString(status));
   }

   for (ii = 0; hexProps[ii].name != NULL; ii++) {
      status = clGetDeviceInfo(device, hexProps[ii].param, sizeof val, &val, &size);
      if (status != CL_SUCCESS) {
         fprintf( stderr, "\tdevice[%d]: Unable to get %s: %s!\n",
                  device_index, hexProps[ii].name, CLErrString(status));
         continue;
      }
      if ( size > sizeof val )
      {
        fprintf( stderr, "\tdevice[%d]: Large %s (%lu bytes)!  Truncating to %lu!\n",
                 device_index, hexProps[ii].name, size, sizeof val );
      }
      printf("\tdevice[%d]: %-30s: 0x%" PRIx64 "\n", device_index, hexProps[ii].name, val);
   }

   for (ii = 0; longProps[ii].name != NULL; ii++) {
      status = clGetDeviceInfo(device, longProps[ii].param, sizeof val, &val, &size);
      if (status != CL_SUCCESS) {
         fprintf( stderr, "\tdevice[%d]: Unable to get %s: %s!\n",
                  device_index, longProps[ii].name, CLErrString(status));
         continue;
      }
      if ( size > sizeof val )
      {
         fprintf( stderr, "\tdevice[%d]: Large %s (%lu bytes)!  Truncating to %lu!\n",
                  device_index, longProps[ii].name, size, sizeof val );
      }
      printf("\tdevice[%d]: %-30s: %'" PRIu64 "\n", device_index, longProps[ii].name, val);
   }
   printf( "\tdevice[%d]: %-30s:", device_index, "IMAGE FORMATS" );
   printImageFormats( device_index, &device, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D );
}

/*
 * PrintPlatform --
 *
 *      Dumps everything about the given platform ID.
 *
 * Results:
 *      void.
 */
static void
PrintPlatform( int platform_index, cl_platform_id platform )
{
   static struct { cl_platform_info param; const char *name; } props[] = {
      { CL_PLATFORM_PROFILE, "profile" },
      { CL_PLATFORM_VERSION, "version" },
      { CL_PLATFORM_NAME, "name" },
      { CL_PLATFORM_VENDOR, "vendor" },
      { CL_PLATFORM_EXTENSIONS, "extensions" },
      { 0, NULL },
   };
   cl_device_id *deviceList;
   cl_uint numDevices;
   cl_int status;
   char buf[65536];
   char *word;
   size_t size;
   cl_uint ii;

   for ( ii = 0; props[ii].name != NULL; ii++ )
   {
      status = clGetPlatformInfo(platform, props[ii].param, sizeof buf, buf, &size);
      if (status != CL_SUCCESS)
      {
         fprintf( stderr, "platform[%d]: Unable to get %s: %s\n",
                  platform_index, props[ii].name, CLErrString(status) );
         continue;
      }
      if ( size > sizeof buf )
      {
         fprintf( stderr, "platform[%d]: Huge %s (%lu bytes)!  Truncating to %lu\n",
                  platform_index, props[ii].name, size, sizeof buf );
      }
      if ( strcmp( "extensions", props[ii].name ) )
        printf("platform[%d]: %-10s: %s\n", platform_index, props[ii].name, buf);
      else
      {
        printf( "platform[%d]: %-10s: %s\n", platform_index, props[ii].name, ( word = strtok( buf, " " ) ) );
        for ( word = strtok( NULL, " " ); word; word = strtok( NULL, " " ) )
          printf( "%24s %s\n", "", word );
      }
   }

   if ((status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices)) != CL_SUCCESS)
   {
      fprintf( stderr, "platform[%d]: Unable to query the number of devices: %s\n",
               platform_index, CLErrString(status));
      return;
   }
   printf( "platform[%d]: Found %d device%s.\n", platform_index, numDevices, ( numDevices == 1 ? "" : "s" ) );

   deviceList = malloc(numDevices * sizeof(cl_device_id));
   if ( ( status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, deviceList, NULL ) ) != CL_SUCCESS )
   {
      fprintf( stderr, "platform[%d]: Unable to enumerate the devices: %s\n",
               platform_index, CLErrString(status) );
      free( deviceList );
      return;
   }

  for ( ii = 0; ii < numDevices; ++ii )
  {
    PrintDevice( ii, deviceList[ii] );
    if ( ii < numDevices - 1 )
    {
      fprintf( stdout, "--------------------------------------------------------------------------------\n" );
    }
  }

   free( deviceList );
}

int
main( int argc, char * argv[] )
{
  cl_int status;
  cl_platform_id *platformList;
  cl_uint numPlatforms;
  cl_uint ii;

  setlocale( LC_NUMERIC, "en_US.UTF-8" );

  ParseOpts( argc, argv );

  if ( ( status = clGetPlatformIDs( 0, NULL, &numPlatforms ) ) != CL_SUCCESS )
  {
    fprintf( stderr, "Unable to query the number of platforms: %s\n", CLErrString(status) );
    exit( 1 );
  }
  printf( "Found %d platform%s.\n", numPlatforms, ( numPlatforms == 1 ? "" : "s" ) );

  platformList = malloc( sizeof(cl_platform_id) * numPlatforms );
  if ( ( status = clGetPlatformIDs( numPlatforms, platformList, NULL ) ) != CL_SUCCESS )
  {
    fprintf( stderr, "Unable to enumerate the platforms: %s\n", CLErrString( status ) );
    exit( 1 );
  }

  for ( ii = 0; ii < numPlatforms; ++ii )
  {
    PrintPlatform( ii, platformList[ii] );
    if ( ii + 1 < numPlatforms )
    {
      fprintf( stdout, "================================================================================\n" );
    }
  }

  free( platformList );
  exit( 0 );
}
