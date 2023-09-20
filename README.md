# N4-Biascorrection Matlab

## Description

This repo contains the source code for the N4BiasCorrection functionality that is to be called from matlab 
as a matlab executable. 

This code provides an MATLAB C++ interface to call N4BiasCorrection implemented by [[SimpleITK](https://simpleitk.readthedocs.io/en/master/link_N4BiasFieldCorrection_docs.html)]([Source](https://github.com/SimpleITK/SimpleITK))



## Building The Executable [MEX](https://www.mathworks.com/help/matlab/matlab_external/build-c-mex-programs.html) 

1. git clone the [SimpleITK project](https://github.com/SimpleITK/SimpleITK) and compile following the [official doc](https://simpleitk.readthedocs.io/en/master/building.html). The functionalities of this repo requires the compiled libraries from the [SimpleITK project](https://github.com/SimpleITK/SimpleITK).

2. Clone this repo and in [CMakeLists.txt](./CMakeLists.txt), modify the variables `SimpleITK_DIR` and `ITK_DIR`. 
> If you built your SimpleITK using the super build example provided by their official document, these two variables should point to the **`SimpleITK-build/`** folder and the **`ITK-build/`** folder. You should be able to find the files **`SimpleITKConfig.cmake`** and **`ITKConfig.cmake`** respectively in these folders. See [this tutorial](https://simpleitk.readthedocs.io/en/master/link_CppCMake_docs.html) on how to configure Cmake for building SimpleITK applications.


3. Run cmake. 
```bash 
cmake ./
```
>Should you choose to compile with the debug mode, Run cmake with  <code class="bash"> cmake -DCMAKE_BUILD_TYPE=Debug ./ </code>. Debug configuration example for VS Code is included in [launch.json](.vscode/launch.json)

4. Compile
```bash
make
```

## Running N4BiasCorrection in Matlab
Make sure the compiled executable is in matlab's search path. 

```matlab
[corrected_image, log_bias_field] = N4BiasCorrection(image);
```

This function supports only non-conplex 2D and 3D images with data type float(single) and double.

## Demo

![Bias Correction Demonstration](./figures/combined.png)
