#pragma once
//#include "test_precomp.hpp"
class Halcon_Blob_Test : public cvtest::FolderTest
{
public:
	typedef cvtest::FolderTest Base;
	Halcon_Blob_Test(string filename,int in_count, int out_count);
protected:
	void trans_input_mat();
	void trans_output_to_mat();

	//������������������Hmat�������ͣ�����Mat->����Hmat->���Hmat->���Mat
	vector<vector<HMat> > test_hmat;
	//Halcon��������������Hobject���ͣ�����Mat->����Hobject->���Hobject->���Mat
	vector<vector<Halcon::Hobject>> test_hobject;

private:
	template<typename T1, typename T2>
	void resize_vector(vector<vector<T1>>& test_vector, const vector<vector<T2>>& test_mat);

};