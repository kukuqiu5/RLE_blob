#include "accu_precomp.hpp"
#include <fstream>
///////// threshold //////////

class ErosionCircleTest : public FolderTest
{
public:
	ErosionCircleTest(double radius_)
	{
		low_ = 100;
		high_ = 200;
		radius = radius_;
		test_mat[OUTPUT].push_back(Mat());
		test_mat[REF_OUTPUT].push_back(Mat());
	};
protected:
	void run_func();
	void prepare_to_validation(int test_case_idx);
private:
	double low_;
	double high_;
	double radius;
};

void ErosionCircleTest::run_func()
{
	HRegion Region;
	HRegion Region_erosion;
	int a;
	threshold(GrayImage, Region, low_, high_);

	//a = Region.len;
	//ofstream outfile;
	//outfile.open("C:/Users/02652/Desktop/test/Opencv_Region.xml", ios::binary | ios::app | ios::in | ios::out, ios::trunc);
	//outfile << "count=" << a << "\n";
	//outfile << "\n";
	//for (int i = 0; i < a; i++){
	//	outfile << "ColumnBegin=" << Region.rle_x_start[i] << ",";
	//	outfile << "Row=" << Region.rle_y[i] << ",";
	//	outfile << "ColumnEnd=" << Region.rle_x_end[i] << "\n";
	//}
	//outfile.close();
	erosion_circle(Region, Region_erosion, radius);
	a = Region_erosion.len;
	

	//ofstream outfile;
	//outfile.open("C:/Users/02652/Desktop/test/Opencv_erosionCircle.xml", ios::binary | ios::app | ios::in | ios::out, ios::trunc);
	//outfile << "count=" << a << "\n";
	//outfile << "\n";
	//for (int i = 0; i < a; i++){
	//	outfile << "ColumnBegin=" << Region_erosion.rle_x_start[i] << ",";
	//	outfile << "Row=" << Region_erosion.rle_y[i] << ",";
	//	outfile << "ColumnEnd=" << Region_erosion.rle_x_end[i]<< "\n";
	//}
	//outfile.close();

	test_mat[OUTPUT][0] = Mat::ones(Size(a, 3), CV_16SC1);
	for (int i = 0; i < a; i++){
		test_mat[OUTPUT][0].at<short>(0, i) = Region_erosion.rle_x_start[i];
		test_mat[OUTPUT][0].at<short>(1, i) = Region_erosion.rle_y[i];
		test_mat[OUTPUT][0].at<short>(2, i) = Region_erosion.rle_x_end[i];

	}
}


void ErosionCircleTest::prepare_to_validation(int test_case_idx)
{
	/*������ʼ��*/
	using namespace Halcon;
	Hobject Region, RegionErosion, HalconGrayImage;
	Hlong NumRuns, Bytes;
	double KFactor, LFactor, MeanLength;
	HTuple Row, ColumnBegin, ColumnEnd, number;
	Mat2HObject(GrayImage, HalconGrayImage);
	threshold(HalconGrayImage, &Region, low_, high_);

	//runlength_features(Region, &NumRuns, &KFactor, &LFactor, &MeanLength, &Bytes);
	//get_region_runs(Region, &Row, &ColumnBegin, &ColumnEnd);
	//ofstream outfile;
	//outfile.open("C:/Users/02652/Desktop/test/Halcon_Region.xml", ios::binary | ios::app | ios::in | ios::out, ios::trunc);
	//outfile << "count=" << NumRuns << "\n";
	//outfile << "\n";
	//for (int i = 0; i < NumRuns; i++){
	//	outfile << "ColumnBegin=" << ColumnBegin[i].I() << ",";
	//	outfile << "Row=" << Row[i].I() << ",";
	//	outfile << "ColumnEnd=" << ColumnEnd[i].I() << "\n";
	//}
	//outfile.close();

	erosion_circle(Region, &RegionErosion, radius);
	runlength_features(RegionErosion, &NumRuns, &KFactor, &LFactor, &MeanLength, &Bytes);
	get_region_runs(RegionErosion, &Row, &ColumnBegin, &ColumnEnd);
	

	//ofstream outfile;
	//outfile.open("C:/Users/02652/Desktop/test/Halcon_erosionCircle.xml", ios::binary | ios::app | ios::in | ios::out, ios::trunc);
	//outfile << "count=" << NumRuns << "\n";
	//outfile << "\n";
	//for (int i = 0; i < NumRuns; i++){
	//	outfile << "ColumnBegin=" << ColumnBegin[i].I() << ",";
	//	outfile << "Row=" << Row[i].I() << ",";
	//	outfile << "ColumnEnd=" << ColumnEnd[i].I()<< "\n";
	//}
	//outfile.close();

	test_mat[REF_OUTPUT][0] = Mat::ones(Size(NumRuns, 3), CV_16SC1);
	for (int i = 0; i < NumRuns; i++){
		test_mat[REF_OUTPUT][0].at<short>(0, i) = ColumnBegin[i].I();
		test_mat[REF_OUTPUT][0].at<short>(1, i) = Row[i].I();
		test_mat[REF_OUTPUT][0].at<short>(2, i) = ColumnEnd[i].I();
	}
}


typedef std::tr1::tuple<double, double> Erosion_Circle_c;
typedef ::testing::TestWithParam<Erosion_Circle_c> Erosion_Circle_ersion;
TEST_P(Erosion_Circle_ersion, accuracy)
{
	Erosion_Circle_c t = GetParam();
	double radius = std::tr1::get<0>(t);
	ErosionCircleTest test(radius);
	test.safe_run();
}

INSTANTIATE_TEST_CASE_P(Core_Erosion_Circle, Erosion_Circle_ersion, testing::Combine(
	testing::Values(double(5),double(20),double(10.5), double(12.5), double(2.5), double(3.5), double(25.5), double(30.5), double(35.5)),
	testing::Values(double(10))));

