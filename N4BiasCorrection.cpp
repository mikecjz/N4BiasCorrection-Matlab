/*
 *This file contains the source code for the N4BiasCorrection functionality that is to be called from matlab
 *as a matlab executable.
 *Usage inside matlab [corrected_image, log_bias_field] = N4BiasCorrection(original_image)
 *Currently, it only supports 2D or 3D images with float(single) or double
 *Author: Junzhou Chen (Junzhou.Chen@med.usc.edu)
 */
#include <SimpleITK.h>
#include <stdlib.h>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <variant>
#include <iostream>
#include <fstream>

namespace sitk = itk::simple;

class MexFunction : public matlab::mex::Function
{
public:
    // Operator API for matlab to call
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {

        // APIs to communicate with matlab runtime
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
        matlab::data::ArrayFactory factory;

        checkArguments(outputs, inputs);

        if (inputs[0].getType() == matlab::data::ArrayType::DOUBLE)
        {
            matlab::data::TypedArray<double> inMatrix = std::move(inputs[0]);
            process<double>(outputs, inMatrix);
        }
        else if (inputs[0].getType() == matlab::data::ArrayType::SINGLE)
        {
            matlab::data::TypedArray<float> inMatrix = std::move(inputs[0]);
            process<float>(outputs, inMatrix);
        }
        else
        {
            // handle unspported data type
            matlabPtr->feval(u"error",
                             0, std::vector<matlab::data::Array>({factory.createScalar("Input datatype not supported. Supported types: double and single (noncomplex)")}));
        }
    }

    template <typename T>
    void process(matlab::mex::ArgumentList &outputs, matlab::data::TypedArray<T> &inMatrix)
    {

        // Define output arrays
        matlab::data::TypedArray<T> outputMatrix;
        matlab::data::TypedArray<float> logBiasMatrix;

        // Call Bias Correction. outputMatrix, logBiasMatrix passed as references
        std::vector<matlab::data::TypedArray<T>> outputVector = N4BiasCorrection<T>(inMatrix, outputMatrix, logBiasMatrix);

        // Assign outputs
        outputs[0] = outputMatrix;
        outputs[1] = logBiasMatrix;
    }

    // Main funtionality N4BiasCorrection
    template <typename T>
    void N4BiasCorrection(matlab::data::TypedArray<T> &inMatrix,
                          matlab::data::TypedArray<T> &outputMatrix,
                          matlab::data::TypedArray<float> &logBiasMatrix)
    {

        // APIs to communicate with matlab runtime
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
        matlab::data::ArrayFactory factory;

        // Calculate original matrix dimensions
        std::vector<size_t> sizesArray = inMatrix.getDimensions();
        size_t nDim = sizesArray.size();
        size_t nElms = inMatrix.getNumberOfElements();

        // Create native pointer buffer
        T *imageBuffer = Matrix2Pointer<T>(inMatrix);

        // Create SimpleITK Image
        sitk::Image inputImage = CreateImageFromPointer(imageBuffer, sizesArray);
        sitk::Image image = inputImage;

        // Calculate Mask
        sitk::Image maskImage = sitk::OtsuThreshold(image, 0, 1, 200);

        unsigned int shrinkFactor = 1;

        sitk::N4BiasFieldCorrectionImageFilter *corrector = new sitk::N4BiasFieldCorrectionImageFilter();
        unsigned int numFittingLevels = 4;

        // Calculates corrected image and log_bias_field
        sitk::Image corrected_image = corrector->Execute(image, maskImage);
        sitk::Image log_bias_field = corrector->GetLogBiasFieldAsImage(inputImage);

        // Convert to Matlab Array
        outputMatrix = Image2Matrix<T>(corrected_image, sizesArray);
        logBiasMatrix = Image2Matrix<float>(log_bias_field, sizesArray);

    }

    /* This method converts a matlab::data::TypedArray of either double or single and convert
    to a 1D generic pointer of the same type
    */
    template <typename T>
    T *Matrix2Pointer(matlab::data::TypedArray<T> &inMatrix)
    {

        // Calculate original matrix dimensions
        std::vector<size_t> sizesArray = inMatrix.getDimensions();
        size_t nDim = sizesArray.size();
        size_t nElms = inMatrix.getNumberOfElements();

        T *nativeArr = new T[nElms];

        // Copy the data to native array
        size_t idx = 0;
        for (auto elem : inMatrix)
        {
            nativeArr[idx] = elem;
            idx++;
        }

        // return
        return nativeArr;
    }

    /* This method converts from a 1D generic pointer to matlab::data::Array of the same type
    (double or float)
    */
    template <typename T>
    matlab::data::TypedArray<T> Image2Matrix(sitk::Image image, std::vector<size_t> &dims)
    {

        // Create empty TypedArray
        matlab::data::ArrayFactory arrayFactory;
        matlab::data::TypedArray<T> matlabArray = arrayFactory.createArray<T>(dims);

        // Helper variable that help selects the overloaded GetPixelFromImage function definition
        T dummyVar;

        // Copy the data to matla::array
        size_t idx = 0;
        for (auto &elem : matlabArray)
        {
            // Calculate subscripts from linear index
            std::vector<uint32_t> subscripts = IndexToSubscript(idx, dims);

            // Extract pixel value from subcripts and assign to matlab matrix
            elem = GetPixelFromImage(image, subscripts, dummyVar);
            idx++;
        }

        return matlabArray;
    }

    /* This overloaded method returns a float* buffer pointer from a SimpleITK Image object
     */
    float GetPixelFromImage(sitk::Image sitkImage, std::vector<uint32_t> subscripts, float dummyVar)
    {
        float pixelVal = sitkImage.GetPixelAsFloat(subscripts);
        return pixelVal;
    }

    /* This overloaded method returns a double* buffer pointer from a SimpleITK Image object
     */
    double GetPixelFromImage(sitk::Image sitkImage, std::vector<uint32_t> subscripts, double dummyVar)
    {
        double pixelVal = sitkImage.GetPixelAsDouble(subscripts);
        return pixelVal;
    }

    // Function to recursively copy data
    /*
    void recursiveCopy(matlab::data::TypedArray<double> array, std::vector<size_t> indices, size_t currentDim, double*& destPtr) {
        auto dims = array.getDimensions();
        if (currentDim == dims.size() - 1) {
            for (size_t i = 0; i < dims[currentDim]; ++i) {
                indices[currentDim] = i;
                *destPtr = array[indices];
                ++destPtr;
            }
        } else {
            for (size_t i = 0; i < dims[currentDim]; ++i) {
                indices[currentDim] = i;
                recursiveCopy(array, indices, currentDim + 1, destPtr);
            }
        }
    }
    */

    /* This overloaded method creates a SimpleITK Image object from a float* buffer pointer
     */
    sitk::Image CreateImageFromPointer(float *imageBuffer, std::vector<size_t> sizesArray)
    {

        // Create unsigned integer vector to be compatible with sitk::ImportAsFloat
        std::vector<unsigned int> sizesArrayUInt(sizesArray.begin(), sizesArray.end());

        // Return Image
        return sitk::ImportAsFloat(imageBuffer, sizesArrayUInt);
    }

    /* This overloaded method creates a SimpleITK Image object from a double* buffer pointer
     */
    sitk::Image CreateImageFromPointer(double *imageBuffer, std::vector<size_t> sizesArray)
    {

        // Create unsigned integer vector to be compatible with sitk::ImportAsDouble
        std::vector<unsigned int> sizesArrayUInt(sizesArray.begin(), sizesArray.end());

        // Return Image
        return sitk::ImportAsDouble(imageBuffer, sizesArrayUInt);
    }

    // Method to check validity of input
    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
        matlab::data::ArrayFactory factory;

        if (inputs.size() != 1)
        {
            matlabPtr->feval(u"error",
                             0, std::vector<matlab::data::Array>({factory.createScalar("Only one input is allowed and required")}));
        }

        if (inputs[0].getType() == matlab::data::ArrayType::COMPLEX_SINGLE ||
            inputs[0].getType() == matlab::data::ArrayType::COMPLEX_DOUBLE)
        {
            matlabPtr->feval(u"error",
                             0, std::vector<matlab::data::Array>({factory.createScalar("Input multiplier must be a noncomplex")}));
        }

        if (inputs[0].getDimensions().size() == 1 ||
            inputs[0].getDimensions().size() > 4)
        {
            matlabPtr->feval(u"error",
                             0, std::vector<matlab::data::Array>({factory.createScalar("Input must be images with 2D, 3D, or 4D")}));
        }
    }

    /*Helper function to convert a linear index into subscripts. similar to the matlab function ind2sub
     */
    std::vector<uint32_t> IndexToSubscript(size_t index, const std::vector<size_t> &dims)
    {
        std::vector<uint32_t> subscripts(dims.size(), 0);
        size_t remaining = index;

        for (int i = 0; i < dims.size(); i++)
        {
            subscripts[i] = remaining % dims[i];
            remaining /= dims[i];
        }

        return subscripts;
    }

    /* This function is a debug utility that writes a 1D pointer array to a binary raw file.
     */
    template <typename T>
    void WritePointerToRaw(std::string fileName, T *pointer, int size)
    {
        std::ofstream outFile(fileName, std::ios::out | std::ios::binary);
        outFile.write(reinterpret_cast<char *>(pointer), size * sizeof(T));
        outFile.close();
    }
};