#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkChangeLabelImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkVotingBinaryHoleFillingImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkCastImageFilter.h"

int main(int argc, char *argv[])
{
	const unsigned int Dimension = 3;
	//typedef unsigned char PixelType;
	typedef signed short PixelType;
	typedef itk::Image<PixelType, Dimension> ImageType;
	typedef itk::Image<float, Dimension> InternalImageType;

	typedef itk::ImageFileReader<InternalImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("cykl1.mha");
	reader->Update();

	// Wydzielenie obrzasu do dalszej segmentacji. Wydzielenie  - arbitralnie dobrane punkty.
	ImageType::SizeType cropSize;
    cropSize[0] = 33;
	cropSize[1] = 33; // 35
	cropSize[2] = 0;

	typedef itk::CropImageFilter <InternalImageType, InternalImageType> CropImageFilterType;
	CropImageFilterType::Pointer cropFilter = CropImageFilterType::New();
	cropFilter->SetBoundaryCropSize(cropSize);
	cropFilter->SetInput(reader->GetOutput());
	cropFilter->Update();

	// Zapis wyniku crop image
	typedef itk::ImageFileWriter<ImageType> WriterType;
	/*WriterType::Pointer writerCrop = WriterType::New();
	writerCrop->SetFileName("writerCrop7.mha");
	writerCrop->SetInput(cropFilter->GetOutput());
	writerCrop->Update();*/

	//// Progowanie 
	//typedef itk::ThresholdImageFilter<ImageType> ThresholdImageFilterType;
	//ThresholdImageFilterType::Pointer thresFilter = ThresholdImageFilterType::New();
	//thresFilter->SetInput(cropFilter->GetOutput());
	//thresFilter->SetLower(46);
	//thresFilter->SetUpper(55);
	//thresFilter->SetOutsideValue(0);
	//thresFilter->Update();

	//// Zapis wyniku progowania
	//WriterType::Pointer writerThres = WriterType::New();
	//writerThres->SetInput(thresFilter->GetOutput());
	//writerThres->SetFileName("theshold2test3_55.mha");
	//writerThres->Update();

	// Wstepne wygladzenie obrazu curvature filter
	typedef itk::CurvatureFlowImageFilter<InternalImageType, InternalImageType> CurvatureFilterType;
    CurvatureFilterType::Pointer curvatureFilter1 = CurvatureFilterType::New();
    curvatureFilter1->SetInput(cropFilter->GetOutput());
    const unsigned int numberOfIterations =5; //ok 5;  //7; 
    const double       timeStep = 0.25; 

    curvatureFilter1->SetNumberOfIterations( numberOfIterations );
    curvatureFilter1->SetTimeStep(timeStep );
    curvatureFilter1->Update();

	/*WriterType::Pointer writerCurvature1 = WriterType::New();
    writerCurvature1->SetFileName("writerCurvaturerLiver_new1_Wst.mha");
    writerCurvature1->SetInput(curvatureFilter1->GetOutput());
    writerCurvature1->Update();*/

	// Binaryzacja
	typedef itk::BinaryThresholdImageFilter<InternalImageType, InternalImageType> BinaryThresholdImageType;
	BinaryThresholdImageType::Pointer binFilter = BinaryThresholdImageType::New();
	//binFilter->SetInput(cropFilter->GetOutput());
	binFilter->SetInput(curvatureFilter1->GetOutput());
	binFilter->SetLowerThreshold(44);
	binFilter->SetUpperThreshold(55);
	binFilter->SetInsideValue(1);
	binFilter->SetOutsideValue(0);

	//WriterType::Pointer writerBin = WriterType::New();
	//writerBin->SetInput(binFilter->GetOutput());
	//writerBin->SetFileName("BinImageTest2.mha");
	//writerBin->Update();

	// Element strukturalny
	int radius = 60; // bylo ostatnio 54; //ok/48; // 18 - optymal
	typedef itk::BinaryBallStructuringElement<InternalImageType::PixelType, 3> StructuringElementType;
	StructuringElementType strElement;
	strElement.SetRadius(radius);
	strElement.CreateStructuringElement();

	// Dylatacja
	typedef itk::BinaryDilateImageFilter<InternalImageType, InternalImageType, StructuringElementType> BinaryDilateImageFilterType;
	BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
	dilateFilter->SetInput(binFilter->GetOutput());
	dilateFilter->SetKernel(strElement);
	dilateFilter->Update();

	//WriterType::Pointer writerDilate = WriterType::New();
	//writerDilate->SetInput(dilateFilter->GetOutput());
	//writerDilate->SetFileName("ImageDilate_18.mha");
	//writerDilate->Update();

	// Otwarcie 
	typedef itk::BinaryMorphologicalOpeningImageFilter<InternalImageType, InternalImageType, StructuringElementType>
		BinaryOpeningFilterType;
	BinaryOpeningFilterType::Pointer openingFilter = BinaryOpeningFilterType::New();
	openingFilter->SetInput(dilateFilter->GetOutput());
	openingFilter->SetKernel(strElement);
	openingFilter->Update();

	/*WriterType::Pointer writerOpen = WriterType::New();
	writerOpen->SetInput(openingFilter->GetOutput());
	writerOpen->SetFileName("openImage1.mha");
	writerOpen->Update();*/

	// Domkniecie
	typedef itk::BinaryMorphologicalClosingImageFilter<InternalImageType, InternalImageType, StructuringElementType>
		BinaryClosingImageFilterType;
	BinaryClosingImageFilterType::Pointer closingFilter = BinaryClosingImageFilterType::New();
	closingFilter->SetInput(openingFilter->GetOutput());
	closingFilter->SetKernel(strElement);
	closingFilter->Update();

	/*WriterType::Pointer writerClose = WriterType::New();
	writerClose->SetFileName("closingFilterImage.mha");
	writerClose->SetInput(closingFilter->GetOutput());
	writerClose->Update();*/

	// Wypelnianie dziur
	typedef itk::VotingBinaryHoleFillingImageFilter<InternalImageType, InternalImageType> VotingFilterType;
	VotingFilterType::Pointer voteFilter = VotingFilterType::New();

	InternalImageType::SizeType indexRadius;
	const unsigned int radiusX = 18;
	const unsigned int radiusY = 18;
	const unsigned int radiusZ = 1;

	indexRadius[0] = radiusX; // radius along x
	indexRadius[1] = radiusY; // radius along y
	indexRadius[2] = radiusZ; /// radius along z

	voteFilter->SetRadius(indexRadius);
	voteFilter->SetBackgroundValue(0);
	voteFilter->SetForegroundValue(1);
	voteFilter->SetMajorityThreshold(2);
	voteFilter->SetInput(closingFilter->GetOutput());
	voteFilter->Update();

	/*WriterType::Pointer writerVoter = WriterType::New();
	writerVoter->SetInput(voteFilter->GetOutput());
	writerVoter->SetFileName("ImageVoter_48+18_os.mha");
	writerVoter->Update();*/

	// Wygladzenie obrazu curvature filter number 2
	//typedef itk::CurvatureFlowImageFilter<ImageType, ImageType > CurvatureFilterType;
    CurvatureFilterType::Pointer curvatureFilter = CurvatureFilterType::New();
    curvatureFilter->SetInput(voteFilter->GetOutput());
    curvatureFilter->SetNumberOfIterations(10); // bylo 10 
    curvatureFilter->SetTimeStep(timeStep );
    curvatureFilter->Update();

	// Zmiana typu pixela z float -> signed short
	typedef itk::CastImageFilter<InternalImageType, ImageType> CastFilterType;
	CastFilterType::Pointer castFilter = CastFilterType::New();
	castFilter->SetInput(curvatureFilter->GetOutput());
	castFilter->Update();

	WriterType::Pointer writerCurvature = WriterType::New();
    writerCurvature->SetFileName("SegmentedLiverFinal_15_elStr60.mha");
    writerCurvature->SetInput(castFilter->GetOutput());
    writerCurvature->Update();

	return EXIT_SUCCESS;
}