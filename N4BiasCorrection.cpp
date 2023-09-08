/*

*
*/
#include <SimpleITK.h>
#include <stdlib.h>
#include "mex.hpp"
#include "mexAdapter.hpp"
#include <variant>

namespace sitk = itk::simple;

class MexFunction : public matlab::mex::Function {
public:

    //Operator API for matlab to call
    template<typename T>
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        checkArguments(outputs, inputs);
        matlab::data::TypedArray<T> inMatrix = std::move(inputs[0]);

        //Call Bias Correction
        std::vector<matlab::data::TypedArray<T>> outputVector = N4BiasCorrection(inMatrix);

        //Separate corrected image and bias field
        matlab::data::TypedArray<T> outputMatrix = outputVector[0]; //corrected image
        matlab::data::TypedArray<T> logBiasMatrix = outputVector[1]; //log bias field image

        //Asign outputs
        outputs[0] = outputMatrix;
        outputs[1] = logBiasMatrix;

    }

    //Main funtionality N4BiasCorrection
    template<typename T>
    std::vector<matlab::data::TypedArray<T>> N4BiasCorrection(matlab::data::TypedArray<T>& inMatrix) {

        // APIs to communicate with matlab runtime 
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
        matlab::data::ArrayFactory factory;

        //Calculate original matrix dimensions
        std::vector<size_t> sizesArray = inMatrix.getDimensions();
        size_t nDim = sizesArray.size();
        size_t nElms = inMatrix.getNumberOfElements();

        bool isDouble = true;


        //Convert from matlab data types to  pointer of right type
        if (inMatrix.getType() == matlab::data::ArrayType::DOUBLE) {
            isDouble = true;
        }
        else if (inMatrix.getType() == matlab::data::ArrayType::SINGLE){
           
             isDouble = false;
        }
        else { 
            //handle unspported data type
            matlabPtr->feval(u"error", 
                0, std::vector<matlab::data::Array>({ factory.createScalar("Input datatype not supported. Supported types: double and single (noncomplex)") }));
        }

        //Create Native pointer buffer
        T* imageBuffer = Matrix2Pointer(inMatrix);

        //Create SimpleITK Image 
        sitk::Image inputImage = CreateImageFromBuffer(imageBuffer, sizesArray, isDouble);
        sitk::Image image = inputImage;

        //Calculate Mask
        sitk::Image maskImage = sitk::OtsuThreshold( image, 0, 1, 200 );

        unsigned int shrinkFactor = 1;

        sitk::N4BiasFieldCorrectionImageFilter *corrector = new sitk::N4BiasFieldCorrectionImageFilter();
        unsigned int numFittingLevels = 4;


        //Calculates corrected image and log_bias_field
        sitk::Image corrected_image = corrector->Execute( image, maskImage );
        sitk::Image log_bias_field = corrector->GetLogBiasFieldAsImage( inputImage );

        //Get Image buffer from the sitk Image objects
        T* outputImageBuffer;
        T* logBiasImageBuffer;

        if (isDouble) {
            outputImageBuffer = corrected_image.GetBufferAsDouble();
            logBiasImageBuffer = log_bias_field.GetBufferAsDouble();
        }else{
            outputImageBuffer = corrected_image.GetBufferAsFloat();
            logBiasImageBuffer = log_bias_field.GetBufferAsFloat();
        }

        matlab::data::TypedArray<T> outputMatrix = Pointer2Matrix(outputImageBuffer, sizesArray);
        matlab::data::TypedArray<T> logBiasMatrix = Pointer2Matrix(logBiasImageBuffer, sizesArray);

        return std::vector<matlab::data::TypedArray<T>>({outputMatrix, logBiasMatrix});

    }


    /* This method converts a matlab::data::Array of either double or single and convert 
    to a 1D generic pointer of the same type
    */
    template<typename T>
    T* Matrix2Pointer(matlab::data::TypedArray<T>& inMatrix) {


        //Calculate original matrix dimensions
        std::vector<size_t> sizesArray = inMatrix.getDimensions();
        size_t nDim = sizesArray.size();
        size_t nElms = inMatrix.getNumberOfElements();

        /*
        std::variant<double*, float*> nativeArrVariant;

        //Decale pointer of right type and allocate memory
        if (inMatrix.getType() == matlab::data::ArrayType::DOUBLE) {
            double *nativeArr = new double[nElms];
            nativeArrVariant = nativeArr;
        }
        else if (inMatrix.getType() == matlab::data::ArrayType::SINGLE){
            float *nativeArr = new float[nElms];
            nativeArrVariant = nativeArr;
        }
        */

        T *nativeArr = new T[nElms];

         // Copy the data to native array
        size_t idx = 0;
        for (auto elem : inMatrix) {
            nativeArr[idx] = elem;
            idx++;
        }

        //return
        return nativeArr;

    }

    /* This method converts from a 1D generic pointer to matlab::data::Array of the same type 
    (double or float)
    */
    template<typename T>
    matlab::data::TypedArray<T> Pointer2Matrix(T* imageBuffer, std::vector<size_t>& dims){

        // Create empty TypedArray
        matlab::data::ArrayFactory arrayFactory;
        matlab::data::TypedArray<T> matlabArray = arrayFactory.createArray<T>(dims);

        //Get number of total elemens
        size_t nElms = matlabArray.getNumberOfElements();

        // Copy the data to matla::array
        size_t idx = 0;
        for (auto& elem : matlabArray) {
            elem = imageBuffer[idx];
            idx++;
        }

        return matlabArray;
  
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

    /* This method creates a SimpleITK Image object from a std::variant<double*, float*> buffer pointer
    */
    template<typename T>
    sitk::Image CreateImageFromBuffer(T* imageBuffer, std::vector<size_t> sizesArray, bool isDouble){
        
        //Create unsigned integer vector to be compatible with sitk::ImportAsFloat
        std::vector<unsigned int> sizesArrayUInt(sizesArray.begin(), sizesArray.end());

        if (isDouble){
            //double* imageBuffer = std::get<double*>(imageBufferVariant);
            return sitk::ImportAsDouble(imageBuffer, sizesArrayUInt);
        }else{
            //float* imageBuffer = std::get<float*>(imageBufferVariant);
            return sitk::ImportAsFloat(imageBuffer, sizesArrayUInt);
        }


    }

    //Method to check validity of input
    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
        matlab::data::ArrayFactory factory;

        if (inputs.size() != 1) {
            matlabPtr->feval(u"error", 
                0, std::vector<matlab::data::Array>({ factory.createScalar("Only one input is allowed and required") }));
        }

        
        if (inputs[0].getType() == matlab::data::ArrayType::COMPLEX_SINGLE ||
            inputs[0].getType() == matlab::data::ArrayType::COMPLEX_DOUBLE) {
            matlabPtr->feval(u"error", 
                0, std::vector<matlab::data::Array>({ factory.createScalar("Input multiplier must be a noncomplex") }));
        }


        if (inputs[0].getDimensions().size() == 1 ||
            inputs[0].getDimensions().size() >  4) {
            matlabPtr->feval(u"error", 
                0, std::vector<matlab::data::Array>({ factory.createScalar("Input must be images with 2D, 3D, or 4D") }));
        }
    }
};