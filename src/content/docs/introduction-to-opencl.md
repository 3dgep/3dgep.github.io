---
title: "Introduction to OpenCL"
description: "In this article I will provide a brief introduction to OpenCL. OpenCL is a open standard for general purpose parallel programming across CPUs, GPUs, and other programmable parallel devices."
slug: introduction-to-opencl
lastUpdated: 2013-12-10
---

<span id="attachment-2193"></span><img src="/images/OpenCL_Logo.png" alt="OpenCL" width="150" class="thumb" />

:::note
This article was written in 2011 for OpenCL 1.1/1.2. The host API and kernel language shown here are still valid in OpenCL 3.0.
:::

In this article I will provide a brief introduction to OpenCL. OpenCL is a open standard for general purpose parallel programming across CPUs, GPUs, and other programmable parallel devices. I assume that the reader is familiar with the C/C++ programming languages. I will use Microsoft Visual Studio 2008 to show how you can setup a project that is compiled with the OpenCL API.

## Introduction

Similar to [OpenGL](http://www.opengl.org/), [OpenCL](https://www.khronos.org/opencl/) is a platform-independent programming API that allows us to take advantage of the massively parallel computing architectures such as multi-core CPU’s and GPUs. OpenCL only defines an open specification that hardware vendors can implement. For this reason, the specification must be defined in a general, platform-independent manner. This makes programming in OpenCL slightly more cumbersome to use than a platform-specific API such as NVIDIA’s CUDA (Compute Unified Device Architecture). CUDA has been discussed in several previous articles:

-   [CUDA Execution Model](/cuda-thread-execution-model/)
-   [CUDA Memory Model](/cuda-memory-model/)
-   [Optimizing CUDA](/optimizing-cuda-applications/)

In this article, I will introduce OpenCL as well as discuss the similarities with NVIDIA’s CUDA programming API.

## OpenCL Architecture

OpenCL consists of several layers which make-up the OpenCL programming framework. At the top level, the framework defines a platform model that describes how to manage the different devices that are available to the programmer. The **Platform** defines the OpenCL environment and it is through the platform that the different **Device**s are enumerated and one or more **Context**s are created to manage those devices. The context is also required to create one or more **CommandQueue**s that are used to schedule operations that will be performed on the OpenCL devices.

The **Execution** model defines how the host program executes kernels that are then executed on the OpenCL device.

The **Memory** Model defines the different types of memory that are available to the OpenCL application programmer. All OpenCL kernels have access to four distinct memory regions:

-   Global Memory
-   Constant Memory
-   Local Memory
-   Private Memory

The **Programming** model defines the OpenCL programming language. OpenCL kernels are written in the OpenCL programming language which is a subset of the ISO C99 language with extensions for parallelism.

## Platform Model

In OpenCL, the **Platform** consists of a **host** that is used to control one or more OpenCL **devices**. It is possible to have an environment where the **host** and the OpenCL **device** both refer to the CPU whereas in CUDA the **host** most often refers to the application code running on the CPU and the **device** refers to the NVIDIA GPU device that is available for use by the host program. In terms of OpenCL, the **host** always refers to the processor that executes the main application code and the **device** refers to the processor that executes the OpenCL kernel function that is written in the OpenCL programming language.

An OpenCL device subdivided into one or more **compute units** (**CU**). A **CU** is equivalent to a **streaming multiprocessor** (**SM**) in CUDA. Each **CU** is further subdivided into one ore more **processing elements** (**PE**). A **PE** is equivalent to a **streaming processor** (**SP**) in CUDA.

The image below shows a conceptual image of how a typical OpenCL platform might be composed.

<span id="attachment-2210"></span>

![OpenCL Platform Model](../../assets/OpenCL-Platform-Model1.png)

<div class="caption">

OpenCL Platform Model

</div>

As you can see from the image, the conceptual OpenCL platform model is very similar to the architecture of the NVIDIA GPU (as can be seen from the article on [CUDA Thread Model](/cuda-thread-execution-model/)).

CUDA Equivalents
| **OpenCL Term** | **CUDA Equivalent** |
| :-: | :-: |
| Host | Host |
| Device | Device |
| Compute Unit (CU) | Streaming Multiprocessor (SM) |
| Processing Element (PE) | Streaming Processor (SP) |

The OpenCL host application submits commands to the OpenCL **command queue**. CUDA provides a similar mechanism with the **cudaStream** structure. This is mostly hidden in CUDA because the CUDA context maintains a default **cudaStream** object that is used to synchronize execution of commands on the device when no stream object is explicitly specified. In OpenCL however, a valid command queue object is required to enqueue commands that are to be executed on the device.

## The OpenCL Platform Layer

The platform layer of the OpenCL framework defines a set of functions that allow us to discover the available OpenCL devices as well as create an OpenCL context that can be used to run the OpenCL programs. Before we can start using OpenCL in our own applications, we must first create this context. In this section I will demonstrate how we do that in a simple C function.

### Query The Platform

The first step to initializing the OpenCL runtime is to query the available platforms. To do that, we us the **clGetPlatformIDs** method as shown in the code sample below.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=113
    cl_int clError = CL_SUCCESS;

    cl_platform_id platform_id = 0;
    cl_uint num_platforms;

    // Step 1. Get the platform
    clError = clGetPlatformIDs( 0, NULL, &num_platforms );
    checkErr( clError, "clGetPlatformIDs( 0, NULL, &num_platforms );" );
    assert( num_platforms > 0 );

    cl_platform_id* platforms = new cl_platform_id[num_platforms];
    clError = clGetPlatformIDs( num_platforms, platforms, NULL );
    checkErr( clError, "clGetPlatformIDs( num_platforms, &platforms, NULL );" );

    platform_id = platforms[0];
```

The **clGetPlatformIDs** method has the following signature:

```cpp showLineNumbers
cl_int clGetPlatformIDs( cl_uint num_entries,
                         cl_platform_id* platforms,
                         cl_uint* num_platforms );
```

Where:

-   **cl\_uint num\_entries**: The number of values that will be returned in the **platforms** argument. This argument can be 0 if the **platforms** argument is **NULL**, otherwise it must be greater than 0.
-   **cl\_platform\_id\* platforms**: The location where an array of **cl\_platform\_id**s will be stored. This parameter can be NULL in which case, the function will not return the list of platform ID’s. This is useful if you only want to know how many platforms are available in the system.
-   **cl\_uint\* num\_platforms**: The number of OpenCL platforms that are available. This argument can be NULL in which case it is ignored.

You will notice that the **clGetPlatformIDs** method is invoked twice in this code block. The first time it is used (on line 119) the first two arguments are not used. We are only interested in the result of the final argument (**num\_platforms**) which is the number of platforms available on the system.

On line 124, we use the **clGetPlatformIDs** method again, this time supplying the number of available platforms as the first argument and we allocate an array of memory that is large enough to store the result an pass that in the second parameter (**platforms**). We can ignore the final parameter in this case because we already know how many values will be returned.

This method of first determining the size of the result, then performing the query to obtain the result is a common practice when querying OpenCL parameters as we will see later in this article.

On line 127, we simply choose the first platform returned from this function. In an environment where potentially more than one platform may be returned by the **clGetPlatformIDs** method, we may want to loop through the platform array and choose the best one for our needs. For simplicity in this example, I’ll just choose the first one available.

### Query The Devices

Our next step in initializing the OpenCL environment is querying the available OpenCL devices that are available on the platform. To do that, we’ll use the **clGetDeviceIDs** method. At a minimum, we need to have a valid platform ID to pass as the first parameter to this method. We’ll use the platform ID that we obtained from the previous step.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=129
    // Step 2: Get a valid device
    cl_uint num_devices;
    // Query the devices for the first platform
    clError = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices );
    checkErr( clError, "clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices );" );

    cl_device_id* devices = new cl_device_id[num_devices];

    clError = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL );
    checkErr( clError, "clGetDeviceIDs( platforms[0], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL );" );
```

The device query method **clGetDeviceIDs** has the following signature:

```cpp showLineNumbers
cl_int clGetDeviceIDs( cl_platform_id platform,
                       cl_device_type device_type,
                       cl_uint num_entries,
                       cl_device_id* devices,
                       cl_uint* num_devices );
```

Where:

-   **cl\_platform\_id platform**: The platform ID returned by the **clGetPlatformIDs** method shown earlier.
-   **cl\_device\_type device\_type**: A bitfield that identifies the type of OpenCL device. Valid values for this parameter are:
    -   **CL\_DEVICE\_TYPE\_CPU**: An OpenCL device that is the single, or multi-core CPU.
    -   **CL\_DEVICE\_TYPE\_GPU**: An OpenCL device that is a GPU. If you want to support shared device buffers like textures or pixel buffers, then you need to select a GPU device that also supports the graphics API you plan to be sharing buffers with (more on that in the section about extensions).
    -   **CL\_DEVICE\_TYPE\_ACCELERATOR**: Dedicated OpenCL accelerators (for example the IBM CELL Blade).
    -   **CL\_DEVICE\_TYPE\_CUSTOM**: Dedicated accelerators that do not support programs written in OpenCL C. (Why would I want this?)
    -   **CL\_DEVICE\_TYPE\_DEFAULT**: The default OpenCL device in the system.
    -   **CL\_DEVICE\_TYPE\_ALL**: All OpenCL devices available in the system.
-   **cl\_uint num\_entries**: The number of **cl\_device\_id** entries that can be added to the **devices** array. If **devices** is not NULL, then this parameter must be greater than 0.
-   **cl\_device\_id\* devices**: The location to return the list of OpenCL devices found. This argument can be NULL in which case, the **num\_entries** and the **devices** parameters are ignored. Useful if we simply want to query how many available devices there are in the system.
-   **cl\_uint\* num\_devices**: Returns the number of OpenCL devices that match **device\_type**. This parameter can be NULL, in which case it is ignored.

Just like when we queried the platforms, we will invoke the **clGetDeviceIDs** method twice. The first time on line 132 only to find out how many devices exist with the platform specified, and the second query on line 137 to get the actual device ID’s back.

The next step is to find out which device is the best one for our needs.

### Query The Device Extensions

For this particular example, we want to make sure the OpenCL device has support for sharing OpenGL textures. For this we need to check for the existence of the **“cl\_khr\_gl\_sharing”** extension.

We can query various properties of the OpenCL device using the **clGetDeviceInfo** method. This method can be used to query many different properties of the device, but what we are currently interested in is the **CL\_DEVICE\_EXTENSIONS** property.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=140
    // check to see if the device we have supports the cl_khr_gl_sharing extension
    for ( unsigned int i = 0; i < num_devices; ++i )
    {
        size_t param_value_size_ret;
        clError = clGetDeviceInfo( devices[i], CL_DEVICE_EXTENSIONS, 0, NULL, &param_value_size_ret );
        checkErr( clError, "clGetDeviceInfo( devices[i], CL_DEVICE_EXTENSIONS, 0, NULL, &param_value_size_ret );" );

        char* param_value = new char[param_value_size_ret];

        clError = clGetDeviceInfo( devices[i], CL_DEVICE_EXTENSIONS, param_value_size_ret, param_value, NULL );
        checkErr( clError, "clGetDeviceInfo( devices[i], CL_DEVICE_EXTENSIONS, param_value_size_ret, param_value, NULL );" );

        vec_str tokens = TokenizeString( param_value );

        delete [] param_value;

        vec_str::iterator extension = std::find(tokens.begin(), tokens.end(), std::string("cl_khr_gl_sharing") );

        if ( extension != tokens.end() )
        {
            // We found a device that supports the extension we wanted.
            m_CLDeviceID = devices[i];
            break;
        }
    }

    assert( m_CLDeviceID != NULL );
```

The **clGetDeviceInfo** method has the following signature:

```cpp showLineNumbers
cl_int clGetDeviceInfo( cl_device_id device,
                        cl_device_info param_name,
                        size_t param_value_size,
                        void* param_value,
                        size_t* param_value_size_ret );
```

Where:

-   **cl\_device\_id device**: A valid device ID returned by the **clGetDeviceIDs** method.
-   **cl\_device\_info param\_name**: An enumeration constant that identifies the device information being queried. It can be one of the values specified in table 4.3 of the OpenCL Specification (version 1.2).
-   **size\_t param\_value\_size**: The size in bytes of the buffer pointed to by **param\_value**. If **param\_value** is not NULL, this parameter must be greater than 0.
-   **void\* param\_value**: A pointer to a memory buffer that is used to store the value of the property specified by **param\_name**. This parameter can be NULL in which case both **param\_value\_size** and **param\_value** are ignored.
-   **size\_t \*param\_value\_size\_ret**: Returns the actual size in bytes of the data being queried by param\_value. This parameter can be NULL, in which case it is ignored.

Again we invoke the **clGetDeviceInfo** method twice. Once one line 144 to get the size of the buffer that we need to allocate to store the result, and then again on line 149 where the actual value of the parameter is stored in the **param\_value** parameter.

The **CL\_DEVICE\_EXTENSIONS** enumerated parameter name returns a space-delimited list of extension names that are supported by the device. We can tokenize the string returned by this function and if one of the tokens contains the value **“cl\_khr\_gl\_sharing”** then we know we have an OpenCL device that supports sharing memory buffers with OpenGL device contexts.

### Creating the Context

An OpenCL context encapsulates one or more OpenCL devices. The context object is necessary to create a command queue, loading OpenCL programs and building kernel objects, allocating device memory buffer objects, and executing kernel functions that run on the OpenCL device.

The first step to creating our context is defining the context properties that are used to create the context. The properties are defined as a NULL-terminated array of property name, value pairs. The minimum property list must consist of the platform ID (**CL\_CONTEXT\_PLATFORM**) that this context is created with.

In addition to specifying the platform ID during context creation, we also want to support sharing of OpenGL buffer objects with OpenCL. To do that, we must specify additional properties in the **properties** array when creating the context.

As of the OpenCL specification 1.2 extension specifications have moved from the OpenCL specification manual into a separate extension specification. Refer to section 9.6 “Creating CL context from a GL context or share group” in the OpenCL 1.2 Extension Specification manual.  
Refer to the [OpenCL registry](https://registry.khronos.org/OpenCL/) for API specifications, headers, and other documentation.

The properties that you need to specify are specific to the platform you are working with. On a windows platform where you have access to the **WGL** API you need to supply the attribute **CL\_GL\_CONTEXT\_KHR** and it should be set to the value returned by the function **wglGetCurrentContext**. You also must specify the draw context in the attribute named **CL\_WGL\_HDC\_KHR** which can be retrieved using the **wglGetCurrentDC** method.

Let’s see how we would specify the context properties to enable OpenGL buffer sharing on windows:

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=168
    // Step 3. Create a context
    cl_context_properties props[] =
    {
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
        0
    };
```

Now that we have defined the context creation properties, we can use them to create the OpenCL context.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=177
    m_CLContext = clCreateContext(props, 1, &m_CLDeviceID, NULL, NULL, &clError);
    checkErr( clError, "clCreateContext(props, 1, &m_CLDeviceID, NULL, NULL, &clError);" );
```

The **clCreateContext** method has the following signature:

```cpp showLineNumbers
cl_context clCreateContext ( const cl_context_properties *properties,
                             cl_uint num_devices,
                             const cl_device_id *devices,
                             void (CL_CALLBACK *pfn_notify)( const char *errinfo,
                                                             const void *private_info,
                                                             size_t cb,
                                                             void *user_data ),
                             void *user_data,
                             cl_int *errcode_ret )
```

Where:

-   **const cl\_context\_properties \*properties**: Specifies a null-terminated list of context property name, value pairs.
-   **cl\_uint num\_devices**: The number of devices that is specified in the **devices** argument. Contexts can be specified with multiple devices in which case, this argument will be greater than 1.
-   **const cl\_device\_id \*devices**: A list of unique devices returned by **clGetDeviceIDs**.
-   **CL\_CALLBACK \*pfn\_notify**: A callback function that can be registered with the OpenCL context. This callback function will be used by the OpenCL run-time to report information on errors during context creation as well as errors that occur at run-time in this context. This method may be called asynchronously by the OpenCL run-time but it is the responsibility of the application programmer to ensure this function is thread-safe. The parameters to the callback function are:
    
    -   **const char \*errinfo**: A pointer to an error string that describes the error that occurred.
    -   **const void \*private\_info**: A pointer to binary data that is returned by the OpenCL implementation that can be used to log additional information helpful in debugging the error.
    -   **size\_t cb**: The size in bytes of the **private\_info** data.
    -   **void \*user\_data**: The pointer to the user data that was specified when the context was created.
    
    The **pfn\_notify** parameter can be NULL in which case no callback function is registered.
    
-   **void \*user\_data**: A pointer to the user specified data that will be passed to the **pfn\_notify** function if an error occurs. This parameter can be NULL.
-   **cl\_int \*errcode\_ret**: If an error occurred while creating the context the appropriate error code will be returned in this property. This parameter can be NULL.

To summarize these steps, we first need to get a valid platform ID. The platform ID is needed to query the OpenCL devices that are available in the system. Then we create a context using the platform and one or more available devices.

These are the minimum steps necessary to initialize the OpenCL platform layer. Next I will discuss the OpenCL execution layer where we will create a command queue that is used to execute commands in terms of the OpenCL context we just created.

## Execution Model

The execution of an OpenCL program consists of two parts: The **host** program that runs the main application code, and manages contexts an other OpenCL objects, and the **kernel** program that defines functions that execute on the OpenCL device(s).

The core of the OpenCL execution model defines how kernels are executed on a device. The kernel function is en-queued for execution on the OpenCL command queue using an index space called the **NDRange** (N-Dimensional Range).

Unlike in the CUDA runtime API, OpenCL kernels cannot be executed using the “<<< >>>” execution configuration syntax, but instead the granularity of the execution grid is specified using an NDRange. An NDRange is an N-dimensional index space where N is 1, 2, or 3 dimension. In CUDA, the NDRange is similar to a **Grid**.

The NDRange is further divided into **work-groups**. During the kernel execution, the work-gropus are assigned a unique ID in each dimension of the NDRange. A work-group is equivalent to a **thread block** in CUDA.

The work-group is then further subdivided into individual **work-items** that perform the actual work. Each work-item has a unique ID within the work-group as well as a unique global ID within the NDRange. A work-item is equivalent to a **thread** in the CUDA execution model.

CUDA Equivalents
| **OpenCL Term** | **CUDA Equivalent** |
| :-: | :-: |
| NDRange | Grid |
| Work-Group | Block |
| Work-Item | Thread |

The image below shows how a typical NDRange is partitioned.

<span id="attachment-2232"></span>

![OpenCL Execution Model](../../assets/OpenCL-Execution-Model.png)

<div class="caption">

OpenCL Execution Model

</div>

## The OpenCL Runtime

The OpenCL runtime is responsible for managing objects that are created within the OpenCL context. These objects include command queues, memory objects, program objects, and kernel objects. The runtime API is also provides functions that allow you to enqueue commands to the command queue such as executing kernel functions, reading from and writing to OpenCL memory objects.

### Creating a Command Queue

The OpenCL command queue is used to queue a set of commands in order. A command queue is equivalent to the **cudaStream** object in CUDA.

The command queue must be created in terms of an OpenCL context that was created previously. A command queue must be associated to a single device but it can be any of the devices that was specified when the context was created.

A command queue is created using the **clCreateCommandQueue** method. An example of creating a command queue is shown below.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=180
    // Step 4. Create a command-queue.
    m_CLCommandQueue = clCreateCommandQueue( m_CLContext, m_CLDeviceID, 0, &clError );
    checkErr( clError, "clCreateCommandQueue( g_CLContext, m_CLDeviceID, 0, clError );" );
```

The **clCreateCommandQueue** has the following signature:

```cpp showLineNumbers
cl_command_queue clCreateCommandQueue( cl_context context,
                                       cl_device_id device,
                                       cl_command_queue_properties properties,
                                       cl_int *errcode_ret );
```

Where:

-   **cl\_context context**: Is a valid context that was created with the **clCreateContext** function described earlier.
-   **cl\_device\_id device**: Is a valid device ID that was obtained with **clGetDeviceIDs**. This device must be one of the devices that are associated with the **context** parameter.
-   **cl\_command\_queue\_properties properties**: Is a bit-field pattern that is used to specify properties of the command queue. The **properties**argument can consists of one ore more of the following arguments:
    -   **CL\_QUEUE\_OUT\_OF\_ORDER\_EXEC\_MODE\_ENABLE**: If specified, the commands in the command queue are executed out-of-order. Otherwise commands are executed in-order.
    -   **CL\_QUEUE\_PROFILING\_ENABLE**: If specified, profiling of commands in the command queue is enabled. For a detailed description of command profiling, refer to section 5.12 of the OpenCL 1.2 specification manual.
-   **cl\_int \*errcode\_ret**: Return an appropriate error code if something went wrong.

### Creating a Program Object

An OpenCL program object can be created from an OpenCL source file. We can use the **clCreateProgramWithSource** method to create an OpenCL program object from a OpenCL source program file. The following example demonstrates how we can create an OpenCL program object from a source code file:

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=184
    // Open OpenCL source file
    std::cout << "Opening source file: \"" << m_CLSourceFile << "\" ..." << std::endl;
    std::ifstream file(m_CLSourceFile.c_str());
    checkErr( file.is_open() ? CL_SUCCESS : -1, "Could not open source file." );

    std::string prog( std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>() ) );

    const size_t prog_length = prog.size() + 1;
    const char* prog_source = prog.c_str();

    // Step 5. Create the program
    std::cout << "Create program..." << std::endl;
    m_CLProgram = clCreateProgramWithSource(m_CLContext, 1, &prog_source, &prog_length, &clError);
    checkErr(clError, "clCreateProgramWithSource(m_CLContext, 1, &prog_source, &prog_length, &clError);" );
```

The first lines open the source code file and copy the text into a string.

On line 196, the program object is created with the **clCreateProgramWithSource** method. This function has the following signature:

```cpp title="main.cpp" showLineNumbers
cl_program clCreateProgramWithSource( cl_context context,
                                      cl_uint count,
                                      const char **strings,
                                      const size_t *lengths,
                                      cl_int *errcode_ret );
```

Where:

-   **cl\_context context**: Is a valid OpenCL context object created with the **clCreateContext** method.
-   **cl\_uint count**: The number of source files defined in the **strings** array parameter.
-   **const char \*\*strings**: An array of string pointers that make up the source code for the program object.
-   **const size\_t \*lengths**: An array with the sizes of the strings in the **strings** array. This parameter can be NULL, in which case each string in the **strings** array must be NULL-terminated.
-   **cl\_int \*errcode\_ret**: Returns an appropriate error code if an error occurs.

If this function succeeds, then **errcode\_ret** will be set to **CL\_SUCCESS** and a valid **cl\_program** will be returned by the function.

Before we can use the kernel functions defined in the program source code, we must build the program into an executable that can be executed on the OpenCL device.

### Building Program Executables

OpenCL program objects can either be compiled and linked in separate steps, or the program can be built (compiled & linked) in a single step. For this exercise, I am going to perform the compile and link step in a single build step using the **clBuildProgram** method.

An example of building the program we previously created is shown below.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=199
    // Step 6. Build the program
    std::cout << "Building program..." << std::endl;
    clError = clBuildProgram(m_CLProgram, 0, NULL, NULL, NULL, NULL);
```

This code example shows how we can compile & link the program object in a single step using the **clBuildProgram** method. This method has the following signature:

```cpp showLineNumbers
cl_int clBuildProgram( cl_program program,
                       cl_uint num_devices,
                       const cl_device_id *device_list,
                       const char *options,
                       void (CL_CALLBACK *pfn_notify)( cl_program program,
                                                       void *user_data ),
                       void *user_data );
```

Where:

-   **cl\_program program**: Is a valid program object that was created with **clCreateProgramWithSource** or **clCreateProgramWithBinary**.
-   **cl\_uint num\_devices**: The number of devices listed in the **device\_list** parameter.
-   **const cl\_device\_id \*device\_list**: A list of devices associated with **program**. This parameter can be NULL in which case the program will be built for all devices that were used to create the context associated with the program object.
-   **const char \*options**: A pointer to a NULL-terminated string of characters that describe the build options that are used to build the program executable. The list of support options is described in section 5.6.4 of the OpenCL specification manual (version 1.2).
-   **CL\_CALLBACK \*pfn\_notify**: A callback function that is used to notify the result of building the program. This parameter can be NULL in which case control will not be returned to the calling thread until the program has been successfully built. If a valid callback is registered with this function, control will return immediately to the calling thread and the user must use the callback function to determine if the program was successfully built or not. This callback function is invoked asynchronously and it is the responsibility of the application programmer to ensure this function is thread-safe.
-   **void \*user\_data**: A pointer to some user defined data that will be passed to the **pfn\_notify** function. This parameter can be NULL.

### Checking for Build Errors

It is reasonable to assume that your source code may contain syntax errors, especially in the beginning while you are developing the program. You must be able to check for build errors when the program is compiled or linked.

Checking for build errors can be done either using the **pfn\_notify** function passed to the build method, or if no **pfn\_notify** function was specified, the **clBuildProgram** will block until the program is finished building (successfully or not) in which case the method returns an error code which can be used to determine if there were any build errors.

In this example, I don’t want to specify a notification function, so I can check the error code returned from the build function:

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=202
    if ( clError != CL_SUCCESS )
    {
        size_t param_value_size;
        clError = clGetProgramBuildInfo( m_CLProgram, m_CLDeviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &param_value_size );
        checkErr( clError, "clGetProgramBuildInfo( m_CLProgram, m_CLDeviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &param_value_size_ret );" );

        char * buildLog = new char[param_value_size];

        clError = clGetProgramBuildInfo( m_CLProgram, m_CLDeviceID, CL_PROGRAM_BUILD_LOG, param_value_size, buildLog, NULL );
        checkErr( clError, "clGetProgramBuildInfo( m_CLProgram, m_CLDeviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &param_value_size_ret );" );

        std::cout << "Build Log: " << std::endl << buildLog << std::endl;

        delete [] buildLog;
    }
```

As shown in the example code, if something went wrong, we can use the **clGetProgramBuildInfo** method with the **CL\_PROGRAM\_BUILD\_LOG** parameter to retrieve the error log and display it to the user.

### Creating Kernel Objects

If our program was built successfully, we can retrieve the kernel objects that are defined in the source code. The kernel objects can be considered to be the entry-points to the kernel functions defined in the program object. Function declarations in the source code that are decorated with the **\_\_kernel** (or just **kernel**) attribute will be exported and accessible using the **clCreateKernel** method.

To get the kernel object in the program, we will use the **clCreateKernel** method.

The source code sample below shows how to retrieve a kernel object that represents a kernel function called “AddForce” inside the program object.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=233
    // Step 7. Create the kernels from the program
    m_CLKernels[eKF_AddForce] = clCreateKernel( m_CLProgram, "AddForce", &clError );
    checkErr( clError, "clCreateKernel( g_CLProgram, \"AddForce\", &clError );" );
```

The **clCreateKernel** function has the following signature:

```cpp showLineNumbers
cl_kernel clCreateKernel( cl_program program,
                          const char *kernel_name,
                          cl_int *errcode_ret );
```

Where:

-   **cl\_program program**: Is a valid program object that was successfully built with the **clBuildProgram** method.
-   **const char \*kernel\_name**: A NULL-terminated string of the name of the kernel function that is defined in the program executable.
-   **cl\_int \*errcode\_ret**: Returns an appropriate error code.

### Buffer Objects

An OpenCL buffer object is a 1D array of memory that can be used in an OpenCL kernel function. In CUDA, you use the **cudaMalloc** function to allocate a block of device memory. The pointer that **cudaMalloc** gives back is a valid pointer in the device’s global memory space. It is possible to perform pointer arithmetic on the returned pointer if you want to access a sub-block of the memory returned by the **cudaMalloc** function. It is not possible to perform the same pointer arithmetic on a block of OpenCL memory because you only have access to a handle that represents the buffer object.

#### Creating Buffer Objects

To create an OpenCL buffer object, you use the **clCreateBuffer** method as shown in the source code sample below.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=351
        m_clDensity[i] = clCreateBuffer(m_CLContext, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, bufferSize, m_fDensity, &clError );
        checkErr( clError, "clCreateBuffer(m_CLContext, CL_MEM_READ_WRITE, bufferSize, NULL, clError );" );
```

In this example, I want to allocate a block of memory that can be both read from and written to. I also want to copy the contents of the host pointer into the block of memory before it is returned.

This **clCreateBuffer** method has the following signature:

```cpp showLineNumbers
cl_mem clCreateBuffer( cl_context context,
                       cl_mem_flags flags,
                       size_t size,
                       void *host_ptr,
                       cl_int *errcode_ret );
```

Where:

-   **cl\_context context**: Is a valid OpenCL context that was created with **clCreateContext**.
-   **cl\_mem\_flags flags**: Is a bit-field that is used to specify allocation and usage information. This parameter can be 0 in which case the default flag **CL\_MEM\_READ\_WRITE**will be used. This parameter can be composed of the possible flag values:
    -   **CL\_MEM\_READ\_WRITE**: Specifies that the memory object will be read and written by a kernel.
    -   **CL\_MEM\_WRITE\_ONLY**: Specifies that the memory will be written to, but not read from inside a kernel function.
    -   **CL\_MEM\_READ\_ONLY**: Specifies that the memory will be read from, but not written to inside a kernel function.
    -   **CL\_MEM\_USE\_HOST\_PTR**: Specifies that the memory object will use the host memory reference by the **host\_ptr** pointer. In this case, the **host\_ptr** must not be NULL.
    -   **CL\_MEM\_ALLOC\_HOST\_PTR**: Specifies that the application wants to allocate memory from host accessible memory.
    -   **CL\_MEM\_COPY\_HOST\_PTR**: Specifies that the data in **host\_ptr** should be copied to the memory object during allocation. In this case **host\_ptr** cannot be NULL.
    -   **CL\_MEM\_HOST\_WRITE\_ONLY**: Specifies that the host will not try to read from this buffer, but only write to it. This flag enables write-combined memory access which results in optimized data transfer over the system bus.
    -   **CL\_MEM\_HOST\_READ\_ONLY**: Specifies that the host will not write to this buffer but only read the result from an OpenCL kernel function.
    -   **CL\_MEM\_HOST\_NO\_ACCESS**: Specifies that the host application will not try to access this buffer (useful for scratch buffers that are needed by the OpenCL kernel function.
-   **size\_t size**: The size in bytes of the buffer to allocate.
-   **cl\_int \*errcode\_ret**: The appropriate error code if an error occurred.

#### Creating an OpenCL Buffer Object from an OpenGL Buffer Object

It is also possible to create an OpenCL buffer object from an OpenGL buffer object using the **clCreateFromGLBuffer** method. OpenCL image objects can also be created from OpenGL texture objects using the **clCreateFromGLTexture** method.

Creating OpenCL buffers from OpenGL buffers is described in section 9.7.2 of the OpenCL Extension specification (version 1.2) and creating OpenCL image objects from OpenGL texture objects is described in section 9.7.3 of the OpenCL Extension Specification (version 1.2).

The OpenCL extension specification can be obtained from the OpenCL registry here: [https://registry.khronos.org/OpenCL/](https://registry.khronos.org/OpenCL/)

#### Reading, Writing and Copying Buffer Objects

Reading and writing from memory buffers in CUDA is performed using the **cudaMemcpy** method. In OpenCL there are different methods for reading from a buffer object, writing to a buffer object, and copying data from one buffer object to another.

The **clEnqueueReadBuffer** method is used to read data from an OpenCL device buffer into host memory. This method has the following signature:

```cpp showLineNumbers
cl_int clEnqueueReadBuffer( cl_command_queue command_queue,
                            cl_mem buffer,
                            cl_bool blocking_read,
                            size_t offset,
                            size_t size,
                            void *ptr,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list,
                            cl_event *event );
```

The **clEnqueueWriteBuffer** method is used to write to an OpenCL device buffer from host memory. This method has the following signature:

```cpp title="main.cpp" showLineNumbers
cl_int clEnqueueWriteBuffer( cl_command_queue command_queue,
                             cl_mem buffer,
                             cl_bool blocking_write,
                             size_t offset,
                             size_t size,
                             const void *ptr,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_event *event );
```

Where:

-   **cl\_command\_queue command\_queue**: Refers to the command-queue in which the read / write command will be queued. **command\_queue** and buffer must be created with the same OpenCL context.
-   **cl\_mem buffer**: Refers to a valid buffer object that was previously created with **clCreateBuffer**.
-   **cl\_bool blocking\_read, cl\_bool blocking\_write**: Indicates if the read and write operations are blocking or nonblocking. If **blocking\_read** is **CL\_TRUE** i.e. the read command is blocking, **clEnqueueReadBuffer** does not return until the buffer data has been read and copied into memory pointed to by ptr. If **blocking\_read** is **CL\_FALSE** i.e. the read command is non-blocking, **clEnqueueReadBuffer** queues a non-blocking read command and returns. The contents of the buffer that **ptr** points to cannot be used until the read command has completed. The **event** argument returns an event object which can be used to query the execution status of the read or write command.
-   **size\_t offset**: Is the offset in bytes in the buffer object to read from or write to.
-   **size\_t size**: Is the size in bytes of data being read or written.
-   **void \*ptr**: Is the pointer to buffer in host memory where data is to be read from or to be written to.
-   **cl\_uint num\_events\_in\_wait\_list**: Specifies the number of events in the **event\_wait\_list** parameter.
-   **const cl\_event \*event\_wait\_list**: Specify events that need to complete before this particular command can be executed. This parameter can be NULL in which case the **num\_events\_in\_wait\_list** parameter should be 0.
-   **cl\_event \*event**: returns an event object that identifies this particular read / write command and can be used to query or queue a wait for this particular command to complete. If **blocking\_read** or **blocking\_write** is CL\_FALSE, then this event will be the only way to determine if the read or write operation has completed.

Copying data from one device buffer to another is performed using the **clEnqueueCopyBuffer** method. This method has the following signature.

```cpp showLineNumbers
cl_int clEnqueueCopyBuffer( cl_command_queue command_queue,
                            cl_mem src_buffer,
                            cl_mem dst_buffer,
                            size_t src_offset,
                            size_t dst_offset,
                            size_t size,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list,
                            cl_event *event );
```

Where:

-   **cl\_command\_queue command\_queue**: Refers to the command-queue in which the copy command will be queued. The OpenCL context associated with **command\_queue**, **src\_buffer** and **dst\_buffer** must be the same.
-   **cl\_mem src\_buffer**: The buffer where the data will be copied from.
-   **cl\_mem dst\_buffer**: The buffer where the data will be copied to.
-   **size\_t src\_offset**: Refers to the offset where to begin copying data from **src\_buffer**.
-   **size\_t dst\_offset**: Refers to the offset where to begin copying data into **dst\_buffer**.
-   **size\_t size**: The size in bytes to copy.
-   **cl\_uint num\_events\_in\_wait\_list**: Specifies the number of events in the **event\_wait\_list** array parameter.
-   **const cl\_event \*event\_wait\_list**: Specify events that need to complete before this particular command can be executed. This parameter can be NULL in which case the **num\_events\_in\_wait\_list** parameter should be 0.
-   **cl\_event \*event**: returns an event object that identifies this particular copy command and can be used to query or queue a wait for this particular command to complete.

### Setting Kernel Arguments

Before you can execute a kernel on the OpenCL device, you must first set the kernel arguments using the **clSetKernelArg** method.

Assuming we have the following kernel function (declared in an OpenCL source file):

```cpp title="FluidSim.cl" showLineNumbers startLineNumber=159
__kernel void AddForce( global float* x1, global float* x0, float deltaTime );
```

Then the host code that is will set the arguments of the kernel function before it can be executed would look like this:

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=604
    cl_kernel kernel = m_CLKernels[eKF_AddForce];

    clError |= clSetKernelArg( kernel, 0, sizeof(cl_mem), &x1 );
    clError |= clSetKernelArg( kernel, 1, sizeof(cl_mem), &x0 );
    clError |= clSetKernelArg( kernel, 2, sizeof(cl_float), &fDeltaTime );
    checkErr( clError, "clSetKernelArg(...);" );
```

In this example, the **kernel** object was previously created with the **clCreateKernel** method. The **x1**, and **x0** variables are **cl\_mem** device buffer objects previously created with the **clCreateBuffer** method.

The **clSetKernelArg** method has the following signature:

```cpp showLineNumbers
cl_int clSetKernelArg( cl_kernel kernel,
                       cl_uint arg_index,
                       size_t arg_size,
                       const void *arg_value );
```

Where:

-   **cl\_kernel kernel**: Is a valid kernel object previously created with **clCreateKernel**.
-   **cl\_uint arg\_index**: Is the 0-based argument index of the argument to the kernel function.
-   **size\_t arg\_size**: specifies the size of the argument value. If the argument is a memory object, the size must be equal to **sizeof(cl\_mem)**. For arguments declared with the **\_\_local** qualifier, the size specified will be the size in bytes of the buffer that must be allocated for the **\_\_local** argument. If the argument is of type **sampler\_t**, the **arg\_size** value must be equal to **sizeof(cl\_sampler)**. For all other arguments, the size will be the size of argument type.
-   **const void \*arg\_value**: Is a pointer to data that should be used as the argument value for argument specified by **arg\_index**. If the argument is a memory object (buffer, image or image array), the arg\_value entry will be a pointer to the appropriate buffer, image or image array object. The memory object must be created with the context associated with the kernel object. If the argument is declared with the **\_\_local** qualifier, the **arg\_value** entry must be NULL. If the argument is declared with the **\_\_constant** qualifier, the size in bytes of the memory object cannot exceed **CL\_DEVICE\_MAX\_CONSTANT\_BUFFER\_SIZE** and the number of arguments declared as pointers to **\_\_constant** memory cannot exceed **CL\_DEVICE\_MAX\_CONSTANT\_ARGS**. You can use the **clGetDeviceInfo** to query these parameter values for the OpenCL device.

### Executing the Kernel

The function **clEnqueueNDRangeKernel** is used to enqueue a command to execute a kernel function on an OpenCL device.

An example of executing an kernel function is shown in the source code below.

```cpp title="FluidSimOCL.cpp" showLineNumbers startLineNumber=601
    size_t globalWorkSize[2] = { m_iGridWidth, m_iGridHeight };
    size_t globalOffset[2] = { 1, 1 };

    cl_kernel kernel = m_CLKernels[eKF_AddForce];

    clError |= clSetKernelArg( kernel, 0, sizeof(cl_mem), &x1 );
    clError |= clSetKernelArg( kernel, 1, sizeof(cl_mem), &x0 );
    clError |= clSetKernelArg( kernel, 2, sizeof(cl_float), &fDeltaTime );
    checkErr( clError, "clSetKernelArg(...);" );

    clError = clEnqueueNDRangeKernel( m_CLCommandQueue, kernel, 2, globalOffset, globalWorkSize, NULL, 0, NULL, NULL );
    checkErr( clError, "clEnqueueNDRangeKernel( m_CLCommandQueue, kernel, 2, globalOffset, globalWorkSize, NULL, 0, NULL, NULL );" );
```

This code shows setting the kernel arguments for the “AddForce” kernel that we created earlier. Before we can execute the kernel, we must make sure all the kernel arguments are properly set.

The kernel function is then queued to the command queue on line 611 using the **clEnqueueNDRangeKernel** function. This function has the following signature:

```cpp showLineNumbers
cl_int clEnqueueNDRangeKernel( cl_command_queue command_queue,
                               cl_kernel kernel,
                               cl_uint work_dim,
                               const size_t *global_work_offset,
                               const size_t *global_work_size,
                               const size_t *local_work_size,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event );
```

Where:

-   **cl\_command\_queue command\_queue**: Is a valid **cl\_command\_queue** object previously created with **clCreateCommandQueue**.
-   **cl\_kernel kernel**: Is a valid **cl\_kernel** object previously created with **clCreateKernel**. The OpenCL context associated with kernel and command-queue must be the same.
-   **cl\_uint work\_dim**: Is the number of dimensions used to specify the global work-items and work-items in the work-group. **work\_dim** must be greater than zero and less than or equal to **CL\_DEVICE\_MAX\_WORK\_ITEM\_DIMENSIONS**.
-   **const size\_t \*global\_work\_offset**: Can be used to specify an array of **work\_dim** unsigned values that describe the offset used to calculate the global ID of a work-item in the NDRange. If **global\_work\_offset** is NULL, the  
    global IDs start at offset (0, 0, … 0).
-   **const size\_t \*global\_work\_size**: points to an array of **work\_dim** unsigned values that describe the number of global work-items in **work\_dim** dimensions that will execute the kernel function. The total number of global work-items is computed as **global\_work\_size\[0\]** \* … \* **global\_work\_size\[work\_dim – 1\]**.
-   **const size\_t \*local\_work\_size**: Points to an array of **work\_dim** unsigned values that describe the number of work-items that make up a work-group (also referred to as the size of the work-group) that will execute the kernel specified by **kernel**. The total number of work-items in a work-group is  
    computed as **local\_work\_size\[0\]** \* … \* **local\_work\_size\[work\_dim – 1\]**. The total number of work-items in the work-group must be less than or equal to the **CL\_DEVICE\_MAX\_WORK\_GROUP\_SIZE** value and the number of workitems specified in **local\_work\_size\[0\]**, … **local\_work\_size\[work\_dim – 1\]** must be less than or equal to the corresponding values specified by **CL\_DEVICE\_MAX\_WORK\_ITEM\_SIZES\[0\]**, … **CL\_DEVICE\_MAX\_WORK\_ITEM\_SIZES\[work\_dim – 1\]**. **local\_work\_size** can also be a NULL value in which case the OpenCL implementation will determine how to be break the global work-items into appropriate work-group instances.
-   **cl\_uint num\_events\_in\_wait\_list**: Specifies the number of events in the **event\_wait\_list** array or 0 if **event\_wait\_list** is NULL.
-   **const cl\_event \*event\_wait\_list**: Specify events that need to complete before this particular command can be executed. This parameter can be NULL then this command does not wait on any event to complete.
-   **cl\_event \*event**: An event object that identifies this particular kernel execution instance. Event objects are unique and can be used to identify a particular kernel execution instance later on. If **event** is NULL, no event will be created for this kernel execution instance and therefore it will not be possible for the application to query or queue a wait for this particular kernel execution instance.

## Memory Model

When we talk about the OpenCL memory model, we are referring to the different regions of memory that an OpenCL kernel function has access to. There are four distinct memory regions that are specified in the OpenCL specification. The specification doesn’t specify where these memory locations must reside and how the memory access is implemented. We can show the similarities to the CUDA memory model (see the [CUDA Memory Model](/cuda-memory-model/) article).

The four regions of memory that are defined in the OpenCL specification are:

-   **Global Memory**: This memory region permits read/write access to all work-items in all work-groups. Work-items can read from or write to any element of a memory object.  
    Reads and writes to global memory may be cached depending on the capabilities of the device.
-   **Constant Memory**: A region of global memory that remains constant during the execution of a kernel. The host allocates and initializes memory objects placed into constant memory.
-   **Local Memory**: A memory region local to a work-group. This memory region can be used to allocate variables that are shared by all work-items in that work-group. It may be implemented as dedicated regions of memory on the OpenCL device.
-   **Private Memory**: A region of memory private to a work-item. Variables defined in one work-item’s private memory are not visible to another work-item.

The image below shows the conceptual memory model of an OpenCL device.

<span id="attachment-2263"></span>

![OpenCL Conceptural Memory Model](../../assets/OpenCL-Conceptural-Memory-Model.png)

<div class="caption">

OpenCL Conceptural Memory Model

</div>

As can be seen from the image, the private memory is only visible to a single processing element (work-item). Local memory is shared with all processing elements of a compute unit (work-group). And global and constant memory are accessible to all compute units and processing elements.

The table below shows a summary of the different types of memory and the access privileges depending on the scope.

OpenCL Memory Types
| **Memory** | **Host** | **Kernel** |
| :-: | --- | --- |
| **Allocation** | **Access** | **Allocation** | **Access** |
| --- | --- | --- | --- |
| **Global** | Dynamic | Read/Write | None | Read/Write |
| **Constant** | Dynamic | Read/Write | Static | Read Only |
| **Local** | Dynamic | None | Static | Read/Write |
| **Private** | None | None | Static | Read/Write |

The equivalent memory types in CUDA:

CUDA Memory Equivalents
| **OpenCL Memory Type** | **CUDA Memory Type** |
| :-: | :-: |
| Global | Global |
| Constant | Constant |
| Local | Shared |
| Private | Register, Local |

The application running on the host uses the OpenCL API to create memory objects in global memory, and to enqueue memory commands (described previously) that operate on these memory objects. The API functions however does not define what region of memory the OpenCL memory buffers reside in. The memory region is determined by the address space qualifiers assigned to the variable declarations in the OpenCL source file.

### Address Space Qualifiers

The OpenCL programming language defines several address space qualifiers that can be used on variables in the OpenCL program. The **\_\_global**, **\_\_local**, **\_\_constant**, and **\_\_private** address space qualifiers can be used to define the region of memory that is used to allocate the object.

If a variable is not qualified by an address space attribute, then the default address space **\_\_private** is implicitly assigned to the variable declaration.

Kernel function arguments declared to be a pointer or an array of a type can point to one of  
the following address spaces only: **\_\_global**, **\_\_local** or **\_\_constant**. A pointer to a specific address space can only be assigned to a pointer to the same address space. Casting a pointer to from one address space to a pointer to a different address space is illegal.

Function arguments must always specify the address space of memory they are used in. For example, if you pass a pointer to global memory to a function in OpenCL, the argument to the accepting function must also specify the address space.

```cpp showLineNumbers
// DoSomething operates on some memory declared in the global address space.
void DoSomething( __global float* someMemory )
{
...
}

__kernel KernelFunction( __global float* dstData, __global float* srcData )
{
    DoSomething( srcData ); // srcData must be in __global address space.
}
```

Variables declared in the program scope (declared outside of any function) must be declared in the **\_\_constant** address space. Constant memory declared in global scope must be statically defined.

#### \_\_global (or global) Address Space

The **\_\_global** or **global** address space name is used to refer to memory objects (buffer or image objects) allocated from the global memory pool. A buffer memory object can be declared as a pointer to a scalar, vector or user-defined struct. This allows the kernel to read and/or write any location in the buffer. The actual size of the array memory object is determined when the memory object is allocated via appropriate API calls in the host code.

#### \_\_local (or local) Address Space

The **\_\_local** or **local** address space name is used to describe variables that need to be allocated in local memory and are shared by all work-items of a work-group. Pointers to the **\_\_local** address space are allowed as arguments to functions (including kernel functions). Variables declared in the \_\_local address space inside a kernel function must occur at kernel function scope.

An example of a kernel function that declares local memory:

```cpp showLineNumbers
__kernel void my_func(...)
{
    __local float a; // A single float allocated
                     // in local address space.
    __local float b[10]; // An array of 10 floats
                         // allocated in local address space.
    if (...)
    {
        // example of variable in __local address space but not
        // declared at __kernel function scope.
        __local float c; // not allowed!
    }
}
```

Variables declared in the **\_\_local** address space are accessible to all work-items in a work-group but not across work-groups.

#### \_\_constant (or constant) Address Space

The **\_\_constant** or constant address space name is used to describe variables allocated in global memory and which are accessed inside a kernel(s) as read-only variables. These read-only variables can be accessed by all (global) work-items of the kernel during its execution. Pointers to the **\_\_constant** address space are allowed as arguments to functions (including kernel functions) and for variables declared inside functions.

Each argument to a kernel that is a pointer to the **\_\_constant** address space is counted separately towards the maximum number of such arguments, defined as **CL\_DEVICE\_MAX\_CONSTANT\_ARGS**.

Variables in the program scope or the kernel function scope can be declared in the **\_\_constant** address space. These variables are required to be statically initialized and the values used to initialize these variables must be a compile time constant. Writing to such a variable results in a compile-time error.

#### \_\_private (or private) Address Space

Variables inside a kernel function not declared with an address space qualifier, all variables  
inside non-kernel functions, and all function arguments are in the **\_\_private** address space. Variables declared as pointers are considered to point to the **\_\_private** address space if an address space qualifier is not specified.

## Programming Model

OpenCL supports two primary programming models: Data parallel, and Task paralell programming models. The primary model that supports the OpenCL design methodology is the data parallel programming model.

### Data Parallel Programming Model

In a data parallel programming model, we define our algorithms and execution domain in terms of a set of input data or output data and perform some operation on each element in that set.

For example, the N-Body simulation uses the Data Parallel programming model because each body description in the data set is processed as a single element in each work-item. All body descriptions are processed in parallel (thus data parallel) to compute the result of a single integration step of the simulation.

In a strictly data parallel model, there is a one-to-one mapping between the work-item in the kernel execution and a single element in the memory object being processed. This is very similar to how vertex and pixel shaders operate on the input data. In this case, access to other elements in the data set (other than the one being processed) are not accessible to the work-item. OpenCL implements a relaxed version of the data parallel programming model where a strict one-to-one mapping is not a requirement.

Data parallel kernel execution is scheduled on the command-queue using the **clEnqueueNDRangeKernel** function described earlier in the section titled “[Executing the Kernel](#executing-the-kernel)“.

### Task Parallel Programming Model

The OpenCL task parallel programming model defines a model in which a single instance of a kernel is executed independent of any index space. This is equivalent to executing a kernel on a Compute Unit with a work-group that consists of only a single work-item.

Task parallel kernel execution is scheduled on the command-queue using the **clEnqueueTask** method.

### Synchronization

Synchronization in an OpenCL application can occur at two levels:

-   Work-items within a work-group,
-   Commands enqueued to an OpenCL command-queue.

#### Work-Item Synchronization

Synchronization between work-items in a single work-group is done using a work-group barrier.  
All the work-items of a work-group must execute the barrier before any are allowed to continue  
execution beyond the barrier. Note that the work-group barrier must be encountered by all work-items of a work-group executing the kernel or by none at all. There is no mechanism for synchronization between work-groups.

The OpenCL programming language implements the **barrier** function which can be used in a kernel function to synchronize the execution of all the work-items in a single work-group.

The **barrier** function has the following signature:

```cpp showLineNumbers
void barrier( cl_mem_fence_flags flags );
```

Where **cl\_mem\_fence\_flags flags** specifies the memory address space and can set to a combination of the following constant literals:

-   **CLK\_LOCAL\_MEM\_FENCE**: The **barrier** function will either flush any variables stored in local memory or queue a memory fence to ensure correct ordering of memory operations to local memory.
-   **CLK\_GLOBAL\_MEM\_FENCE**: The **barrier** function will queue a memory fence to ensure correct ordering of memory operations to global memory.

A **barrier** function is necessary if a work-item in a work-group want to read an area of global, or local memory that has been updated by another work-item in the same work-group. This barrier synchronization function does not synchronize across work-groups.

#### Command Queue Synchronization

If it is necessary to synchronize commands in the command queue, or synchronize operations on global memory across work-groups, then you must perform command-queue synchronization.

The synchronization points between commands in command-queues are:

-   Command-queue barrier. The command-queue barrier ensures that all previously queued commands have finished execution and any resulting updates to memory objects are visible to subsequently enqueued commands before they begin execution. This barrier can only be used to synchronize between commands in a single command-queue.
-   Waiting on an event. All OpenCL API functions that enqueue commands return an event that identifies the command and memory objects it updates. A subsequent command waiting on that event is guaranteed that updates to those memory objects are visible before the command begins execution.

A command-queue barrier can be enqueued using the **clEnqueueBarrierWithWaitList**. The **clEnqueueBarrierWithWaitList** function enqueues a barrier command which waits for either a list of events to complete, or if the list is empty it waits for all commands previously enqueued in command\_queue to complete before it completes. This command blocks command execution, that is, any following commands enqueued after it do not execute until it completes. This command returns an event which can be waited on, i.e. this event can be waited on to insure that all events either in the **event\_wait\_list** or all previously enqueued commands, queued before this command to **command\_queue**, have completed.

Command-queue synchronization can also be accomplished using events. The **clWaitForEvents** function can be used to ensure a set of events have completed before continuing execution. This method has the following signature:

```cpp showLineNumbers
cl_int clWaitForEvents( cl_uint num_events, const cl_event *event_list );
```

The function **clWaitForEvents** waits on the host thread for commands identified by event objects in **event\_list** to complete. The events specified in **event\_list** act as synchronization points.

## Conclusion

This article introduces the reader to the primary models of the OpenCL programming API. The Platform Model describes how to initialize and configure the OpenCL run-time environment. The Execution Model defines how kernel functions are executed on the OpenCL device. The Memory Model defines the different regions of memory that an OpenCL kernel has access to. And finally the Programming Model defines the logical processing of our problem domain.

I hope that after reading this article, you have a better understanding of how to write an application that uses the OpenCL programming language and API.

## Code Sample

The following OpenCL code sample demonstrates a fluid solver that can execute both on the CPU and GPU (toggle with the ‘C’ key) using OpenCL. The mouse (click and drag) is used to introduce some smoke (density) into the simulation that is integrated in the OpenCL kernel functions.

You can download the source file for the Fluid Simulation including project and solution files for Microsoft Visual Studio 2008 here:

[\[FluidSim.zip\]](https://drive.google.com/file/d/0B9NpHNF2In_uNWI1NTA0YjEtMDk3Mi00MmUyLWFhZjEtMWFjYWRmMzY5NTM3/view)

## References

Khronos Group (2011, November). *The OpenCL Specification*. (Version 1.2, Revision 15). USA Available from: <[https://registry.khronos.org/OpenCL/specs/opencl-1.2.pdf](https://registry.khronos.org/OpenCL/specs/opencl-1.2.pdf)\>. Accessed: 19 December, 2011.
