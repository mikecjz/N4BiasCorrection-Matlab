# N4-Biascorrection Matlab

## Description

This repo contains the source code for the N4BiasCorrection functionality that is to be called from matlab 
as a matlab executable. 

This code provides an MATLAB C++ interface to call N4BiasCorrection implemented by [[SimpleITK](https://simpleitk.readthedocs.io/en/master/link_N4BiasFieldCorrection_docs.html)]([Source](https://github.com/SimpleITK/SimpleITK))



## Building MEX 

1. git clone the [SimpleITK project](https://github.com/SimpleITK/SimpleITK) and compile following the [official doc](https://simpleitk.readthedocs.io/en/master/building.html)
2. Clone this repo and in [CMakeLists.txt](./CMakeLists.txt), modify the variables `SimpleITK_DIR` and `ITK_DIR`
3. Run Cmake. 
```bash 
cmake .
```
4. Compile
```bash
cmake --build . --config Release
```


## Running N4BiasCorrection in Matlab
Make sure the compiled executable is in matlab's search path. 

```matlab
[corrected_image, log_bias_field] = N4BiasCorrection(image);
```

This function supports only non-conplex 2D and 3D images with data type float(single) and double.
