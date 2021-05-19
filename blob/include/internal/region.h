#pragma once
#define max_run_length 300000
class BLOB_EXPORTS HRegion
{
public:
	HRegion();
	HRegion(const HRegion& Region);
	HRegion & operator=(HRegion const & Region);
	virtual ~HRegion();
public:
	//��Ա����
	int const Width() const;
	int const Height() const;
	int const Area() const;
	double const Circularity() const;
	double const Rectangularity() const;

	//��Ա����
	int Image_Width;//��ǰͼƬ�Ŀ��
	int Image_Height;//��ǰͼƬ�ĸ߶�
	short* rle_x_start;//�г̱��������ʼֵ
	short* rle_y;//�г̱�����н���ֵ
	short* rle_x_end;//�г̱������ֵ
	int len;//�г̱��볤��
	vector<int> runLabels;//��ǰRegion�ı�ǩֵ
	int runLabels_Count;//��ǰRegion��Connection�°�������Region
private:
	int count;//���ƹ��캯������ֵ
};

typedef struct structMinAndMax{
	double Min = 0;
	double Max = 0;
	double Value = 0;
	string Name;
	bool flag = false;//�жϵ�ǰ�����Ƿ����
}MinMaxParam;


typedef struct structCond{
	MinMaxParam Width;
	MinMaxParam Height;
	MinMaxParam Area;
}CondParam;

typedef struct structListHRegion{
	HRegion* Region;
	int count = 0;
}ListHRegion;