#include "../include/blob_precomp.hpp"
#include "../include/morph.h"
#include <fstream>

namespace blob{
	void erosion_rectangle1(const HRegion& src, HRegion& dst, int width, int height){
		//��width=20ʱ���蹹��1��1&&2��1&&2&&3&&4��1&&2&&3&&4&&5&&6&&7&&8,...
		dst.Image_Width = src.Image_Width;
		dst.Image_Height = src.Image_Height;
		int step = 1;
		int rank = 1;

		//�����Ҫ�������Ĵ���,ʱ�临�Ӷ�Ϊ(log2^(n)),nΪ�߶�
		while (step < height)
		{
			step <<= 1;
			++rank;
		}

		HRegion *erosion_temp = new HRegion[rank];
		for (int i = 0; i < rank; ++i)
		{
			erosion_temp[i].Image_Width = src.Image_Width;
			erosion_temp[i].Image_Height = src.Image_Height;
		}

		//step1:erosion_temp[0].len��ʾΪȥ����������Ҫ����г�ʱ����г��ܳ���
		erosion_temp[0].len = erosion_rectangel_row(src, erosion_temp[0], width);

		//step2:��ǰ����ȡ���������һ��������Region
		for (int i = 1; i < rank; ++i)
		{
			erosion_temp[i].len = erosion_blocks(erosion_temp[i - 1], erosion_temp[i], height, i);
		}

		--rank;

		//step3:���г̵�ֵ�ƶ���ԭ���ֵ,x��ƫ��Ϊlog2^(width -1),y��ƫ��Ϊlog2^(height -1)
		int offset_x = ((width - 1) >> 1);
		int offset_y = ((height - 1) >> 1);
		translate(erosion_temp[rank], dst, offset_x, offset_y);
		
		//�ͷ��ڴ�
		delete[] erosion_temp;
	}

	void dilation_rectangle1(const HRegion& src, HRegion& dst, int width, int height){
		dst.Image_Width = src.Image_Width;
		dst.Image_Height = src.Image_Height;

		//�����Ҫ�������Ĵ���,ʱ�临�Ӷ�Ϊ(log2^(n)),nΪ�߶�
		int step = 1;
		int rank = 1;
		while (step < height)
		{
			step <<= 1;
			++rank;
		}
		HRegion dilation_width;
		HRegion *dilation_temp = new HRegion[rank];
		for (int i = 0; i < rank; ++i)
		{
			dilation_temp[i].Image_Width = src.Image_Width;
			dilation_temp[i].Image_Height = src.Image_Height;
		}

		//step1:��src��ÿ���г̱������width
		dilation_width.len = dilation_rectangel_row(src, dilation_width, width);
		
		//step2:����ǰRegion�ϲ��г̱���,���dilation_temp[0]
		dilation_union(dilation_width, dilation_temp[0]);

		//step3:��ǰ����ȡ���������һ��������Region
		for (int i = 1; i < rank; ++i)
		{
			dilation_temp[i].len = dilation_blocks(dilation_temp[i - 1], dilation_temp[i], height, i);
		}
		--rank;

		//step4:���г̵�ֵ�ƶ���ԭ���ֵ,x��ƫ��Ϊlog2^(width),y��ƫ��Ϊlog2^(height)
		int offset_x = ((width) >> 1);
		int offset_y = ((height) >> 1);
		dilation_translate(dilation_temp[rank], dst, offset_x, offset_y);
		delete[] dilation_temp;
	}

	void opening_rectangle1(const HRegion& src, HRegion& dst, int width, int height){
		//�������ȸ�ʴ������
		HRegion Region;
		Region.Image_Width = src.Image_Width;
		Region.Image_Height = src.Image_Height;
		erosion_rectangle1(src, Region,width,height);
		dilation_rectangle1(Region, dst, width, height);
	}

	void closing_rectangle1(const HRegion& src, HRegion& dst, int width, int height){
		//�����������ͺ�����
		HRegion Region;
		Region.Image_Width = src.Image_Width;
		Region.Image_Height = src.Image_Height;

		dst.Image_Width = src.Image_Width;
		dst.Image_Height = src.Image_Height;

		//�����Ҫ�������Ĵ���,ʱ�临�Ӷ�Ϊ(log2^(n)),nΪ�߶�
		int step = 1;
		int rank = 1;
		while (step < height)
		{
			step <<= 1;
			++rank;
		}
		HRegion dilation_width;
		HRegion *dilation_temp = new HRegion[rank];
		for (int i = 0; i < rank; ++i)
		{
			dilation_temp[i].Image_Width = src.Image_Width;
			dilation_temp[i].Image_Height = src.Image_Height;
		}

		//step1:��src��ÿ���г̱������width
		dilation_width.len = dilation_rectangel_row(src, dilation_width, width);

		//step2:����ǰRegion�ϲ��г̱���,���dilation_temp[0]
		dilation_union(dilation_width, dilation_temp[0]);

		//step3:��ǰ����ȡ���������һ��������Region
		for (int i = 1; i < rank; ++i)
		{
			dilation_temp[i].len = dilation_blocks(dilation_temp[i - 1], dilation_temp[i], height, i);
		}
		--rank;

		//step4:���г̵�ֵ�ƶ���ԭ���ֵ,x��ƫ��Ϊlog2^(width),y��ƫ��Ϊlog2^(height)
		int offset_x = ((width) >> 1);
		int offset_y = ((height) >> 1);
		//�����������ͺ�ʴ,Ԥ�����ͺ��������Ϣ
		closing_translate(dilation_temp[rank], Region, offset_x, offset_y);
		delete[] dilation_temp;
	
		//step5:��ʴ
		erosion_rectangle1(Region, dst, width, height);
	}

	void erosion_circle(const HRegion& src, HRegion& dst, double Radius){
		dst.Image_Width = src.Image_Width;
		dst.Image_Height = src.Image_Height;

		//n.0<Radius<n.5֮��Radius=n.0;  n.5<=Radius<n.0+1֮��Radius=n.5; nΪ����;n>0
		int Radius_Number;
		double Radius_value;
		Radius_Number = max(saturate_cast<int>(Radius), 0);
		if (Radius_Number > Radius){
			Radius_value = Radius_Number - 0.5;
		}
		else if (Radius_Number + 0.5 == Radius){
			Radius_value = Radius;
		}
		else{
			Radius_value = Radius_Number;
		}

		//���Բ�νṹԪ��
		HRegion CircleRegion;
		gen_circle(CircleRegion, 0, 0, Radius_value);

		//step1:��Բ�νṹԪ������ͬ���������ڵ��г̱������һ��(��¼��ǰ�г̱���ĳ���/�п�ʼ/�н���/��ֵ)
		int numberParam;
		vector<ErosionDilationCircleParam> m_ErosionCircleParam;
		ErosionDilationCircleParam l_ErosionCircleParam;
		l_ErosionCircleParam.len = CircleRegion.rle_x_end[0] - CircleRegion.rle_x_start[0] + 1;
		l_ErosionCircleParam.col_start = CircleRegion.rle_x_start[0];
		l_ErosionCircleParam.col_end = CircleRegion.rle_x_end[0];
		l_ErosionCircleParam.row = CircleRegion.rle_y[0];
		l_ErosionCircleParam.count++;
		numberParam = 0;
		m_ErosionCircleParam.push_back(l_ErosionCircleParam);
		for (int i = 1; i < Radius_value * 2; ++i){
			if ((CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1) == (CircleRegion.rle_x_end[i-1] - CircleRegion.rle_x_start[i-1] + 1)){
				m_ErosionCircleParam[numberParam].count++;
			}
			else{
				ErosionDilationCircleParam temp_ErosionCircleParam;
				temp_ErosionCircleParam.len = CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1;
				temp_ErosionCircleParam.col_start = CircleRegion.rle_x_start[i];
				temp_ErosionCircleParam.col_end = CircleRegion.rle_x_end[i];
				temp_ErosionCircleParam.row = CircleRegion.rle_y[i];
				temp_ErosionCircleParam.count++;
				numberParam++;
				m_ErosionCircleParam.push_back(temp_ErosionCircleParam);
			}
		}

		//������ʼ��
		HRegion *erosion_width_temp = new HRegion[m_ErosionCircleParam.size() / 2 + 1];
		for (int i = 0; i < m_ErosionCircleParam.size()/2 +1; ++i)
		{
			erosion_width_temp[i].Image_Width = src.Image_Width;
			erosion_width_temp[i].Image_Height = src.Image_Height;
		}


		HRegion *erosion_intersection_temp = new HRegion[m_ErosionCircleParam.size()];
		for (int i = 0; i < m_ErosionCircleParam.size(); ++i)
		{
			erosion_intersection_temp[i].Image_Width = src.Image_Width;
			erosion_intersection_temp[i].Image_Height = src.Image_Height;
		}


		HRegion *erosion_temp = new HRegion[m_ErosionCircleParam.size()];
		for (int i = 0; i < m_ErosionCircleParam.size(); ++i)
		{
			erosion_temp[i].Image_Width = src.Image_Width;
			erosion_temp[i].Image_Height = src.Image_Height;
		}

		//step2����ͬ���ȵľ��ο������ȥ��Ⱥ��Ӧ���г̱��������(����ĸ���Ϊstep1��һ���1,��ΪԲ�νṹԪ���ǶԳƵ�,��һ�뼴���Ƴ���һ��)
		for (int j = 0; j <m_ErosionCircleParam.size() / 2 + 1; ++j){
			erosion_width_temp[j].len = erosion_circle_row(src, erosion_width_temp[j], m_ErosionCircleParam[j].col_start, m_ErosionCircleParam[j].len, m_ErosionCircleParam[j].row + m_ErosionCircleParam[j].count - 1);
		}

		//step3:����Բ�νṹԪ�����м䳤����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)
		erosion_intersection(erosion_width_temp[m_ErosionCircleParam.size() / 2], m_ErosionCircleParam[m_ErosionCircleParam.size() / 2], erosion_intersection_temp[m_ErosionCircleParam.size() / 2]);

		//step4:����Բ�νṹԪ�س�����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)(Բ�νṹԪ�����¶Գ�,�ƶ�yֵ���Ի����һ��)
		for (int j = 0; j <m_ErosionCircleParam.size()/2; ++j){
			erosion_intersection2(erosion_width_temp[j], m_ErosionCircleParam[j], m_ErosionCircleParam[m_ErosionCircleParam.size()-j-1], erosion_intersection_temp[j], erosion_intersection_temp[m_ErosionCircleParam.size() - j -1]);
		}

		//������ֵ,�Ż��ڴ洦��
		erosion_temp[0] = erosion_intersection_temp[0];

		//step5:����ʣ������Region�Ľ���
		for (int k = 1; k < m_ErosionCircleParam.size(); ++k)
		{
			intersection2(erosion_temp[k - 1], erosion_intersection_temp[k], erosion_temp[k]);
		}

		//step6:���г̵�ֵ�ƶ���ԭ���ֵ
		//n.0<Radius<n.5֮��x�����ƶ�һ,y��ƫ��;n.5<=Radius<n.0+1֮��x,y��ƫ��; nΪ����;n>0
		//��Բ�νṹԪ��������ʴ,�뾶Ϊ����ʱ,��ʴ���Ϊż��,����Ӧ��ƫ��һ��;��������
		int offset_x;
		int offset_y;
		if (Radius_Number > Radius){
			offset_x = 0;
			offset_y = 0;
		}
		else if (Radius_Number + 0.5 == Radius){
			offset_x = 0;
			offset_y = 0;
		}
		else{
			offset_x = -1;
			offset_y = 0;
		}
		translate(erosion_temp[m_ErosionCircleParam.size() - 1], dst, offset_x, offset_y);

		//step7:�ڴ��ͷ�
		m_ErosionCircleParam.clear();
		delete[] erosion_width_temp;
		delete[] erosion_intersection_temp;
		delete[] erosion_temp;
	}

	void dilation_circle(const HRegion& src, HRegion& dst, double Radius){
		dst.Image_Width = src.Image_Width;
		dst.Image_Height = src.Image_Height;

		//n.0<Radius<n.5֮��Radius=n.0;  n.5<=Radius<n.0+1֮��Radius=n.5; nΪ����;n>0
		int Radius_Number;
		double Radius_value;
		Radius_Number = max(saturate_cast<int>(Radius), 0);
		if (Radius_Number > Radius){
			Radius_value = Radius_Number - 0.5;
		}
		else if (Radius_Number + 0.5 == Radius){
			Radius_value = Radius;
		}
		else{
			Radius_value = Radius_Number;
		}

		//���Բ�νṹԪ��
		HRegion CircleRegion;
		gen_circle(CircleRegion, 0, 0, Radius_value);

		//step1:��Բ�νṹԪ������ͬ���������ڵ��г̱������һ��(��¼��ǰ�г̱���ĳ���/�п�ʼ/�н���/��ֵ)
		int numberParam;
		vector<ErosionDilationCircleParam> m_DilationCircleParam;
		ErosionDilationCircleParam l_DilationCircleParam;
		l_DilationCircleParam.len = CircleRegion.rle_x_end[0] - CircleRegion.rle_x_start[0] + 1;
		l_DilationCircleParam.col_start = CircleRegion.rle_x_start[0];
		l_DilationCircleParam.col_end = CircleRegion.rle_x_end[0];
		l_DilationCircleParam.row = CircleRegion.rle_y[0];
		l_DilationCircleParam.count++;
		numberParam = 0;
		m_DilationCircleParam.push_back(l_DilationCircleParam);
		for (int i = 1; i < Radius_value * 2; ++i){
			if ((CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1) == (CircleRegion.rle_x_end[i - 1] - CircleRegion.rle_x_start[i - 1] + 1)){
				m_DilationCircleParam[numberParam].count++;
			}
			else{
				ErosionDilationCircleParam temp_DilationCircleParam;
				temp_DilationCircleParam.len = CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1;
				temp_DilationCircleParam.col_start = CircleRegion.rle_x_start[i];
				temp_DilationCircleParam.col_end = CircleRegion.rle_x_end[i];
				temp_DilationCircleParam.row = CircleRegion.rle_y[i];
				temp_DilationCircleParam.count++;
				numberParam++;
				m_DilationCircleParam.push_back(temp_DilationCircleParam);
			}
		}

		//������ʼ��
		HRegion *Dilation_width_temp = new HRegion[m_DilationCircleParam.size() / 2 + 1];
		for (int i = 0; i < m_DilationCircleParam.size() / 2 + 1; ++i)
		{
			Dilation_width_temp[i].Image_Width = src.Image_Width;
			Dilation_width_temp[i].Image_Height = src.Image_Height;
		}

		HRegion *Dilation_Union_temp = new HRegion[m_DilationCircleParam.size() / 2 + 1];
		for (int i = 0; i < m_DilationCircleParam.size() / 2 + 1; ++i)
		{
			Dilation_Union_temp[i].Image_Width = src.Image_Width;
			Dilation_Union_temp[i].Image_Height = src.Image_Height;
		}

		HRegion *Dilation_Union = new HRegion[m_DilationCircleParam.size()];
		for (int i = 0; i < m_DilationCircleParam.size(); ++i)
		{
			Dilation_Union[i].Image_Width = src.Image_Width;
			Dilation_Union[i].Image_Height = src.Image_Height;
		}

		HRegion *Dilation_temp = new HRegion[m_DilationCircleParam.size()];
		for (int i = 0; i < m_DilationCircleParam.size(); ++i)
		{
			Dilation_temp[i].Image_Width = src.Image_Width;
			Dilation_temp[i].Image_Height = src.Image_Height;
		}

		//step2����ͬ���ȵľ��ο�������Ͽ�Ⱥ��Ӧ���г̱��������(����ĸ���Ϊstep1��һ���1,��ΪԲ�νṹԪ���ǶԳƵ�,��һ�뼴���Ƴ���һ��)
		for (int j = 0; j <m_DilationCircleParam.size() / 2 + 1; ++j){
			Dilation_width_temp[j].len = dilation_circle_row(src, Dilation_width_temp[j], m_DilationCircleParam[j].col_start, m_DilationCircleParam[j].len, m_DilationCircleParam[j].row + m_DilationCircleParam[j].count - 1);
		}

		//step3������step2���г̱���ȡ����֮�����Ϣ
		for (int j = 0; j <m_DilationCircleParam.size() / 2 + 1; ++j){
			dilation_union(Dilation_width_temp[j], Dilation_Union_temp[j]);
		}

		//step4:����Բ�νṹԪ�����м䳤����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)
		dilation_union1(Dilation_Union_temp[m_DilationCircleParam.size() / 2], m_DilationCircleParam[m_DilationCircleParam.size() / 2], Dilation_Union[m_DilationCircleParam.size() / 2]);
		
		//step5:����Բ�νṹԪ�س�����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)(Բ�νṹԪ�����¶Գ�,�ƶ�yֵ���Ի����һ��)
		for (int j = 0; j <m_DilationCircleParam.size() / 2; ++j){
			dilation_union2(Dilation_Union_temp[j], m_DilationCircleParam[j], m_DilationCircleParam[m_DilationCircleParam.size() - j - 1], Dilation_Union[j], Dilation_Union[m_DilationCircleParam.size() - j - 1]);
		}

		//������ֵ,�Ż��ڴ洦��
		Dilation_temp[0] = Dilation_Union[0];

		//step6:�����г̱���Ĳ���
		for (int k = 1; k < m_DilationCircleParam.size(); ++k)
		{
			union2(Dilation_temp[k - 1], Dilation_Union[k], Dilation_temp[k]);
		}

		//step7:���г̵�ֵ�ƶ���ԭ���ֵ
		//n.0<Radius<n.5֮��x,y��ƫ��;n.5<=Radius<n.0+1֮��x,y��ƫ��; nΪ����;n>0
		//��Բ�νṹԪ����������,���ľ�����
		int offset_x;
		int offset_y;
		offset_x = 0;
		offset_y = 0;
		dilation_translate(Dilation_temp[m_DilationCircleParam.size() - 1], dst, offset_x, offset_y);

		//step8:�ڴ��ͷ�
		m_DilationCircleParam.clear();
		delete[] Dilation_width_temp;
		delete[] Dilation_Union_temp;
		delete[] Dilation_Union;
		delete[] Dilation_temp;
	}

	void opening_circle(const HRegion& src, HRegion& dst, double Radius){
		//�ȸ�ʴ������
		HRegion Region;
		Region.Image_Width = src.Image_Width;
		Region.Image_Height = src.Image_Height;

		//step1:��ʴ
		erosion_circle(src, Region, Radius);

		//dilation_circle
		dst.Image_Width = Region.Image_Width;
		dst.Image_Height = Region.Image_Height;

		//n.0<Radius<n.5֮��Radius=n.0;  n.5<=Radius<n.0+1֮��Radius=n.5; nΪ����;n>0
		int Radius_Number;
		double Radius_value;
		Radius_Number = max(saturate_cast<int>(Radius), 0);
		if (Radius_Number > Radius){
			Radius_value = Radius_Number - 0.5;
		}
		else if (Radius_Number + 0.5 == Radius){
			Radius_value = Radius;
		}
		else{
			Radius_value = Radius_Number;
		}

		//���Բ�νṹԪ��(������������迼��ԭ����ƶ�)
		//n.0<Radius<n.5֮��x��ƫ1,y��ƫ1;  n.5<=Radius<n.0+1֮�䲻��; nΪ����;n>0
		HRegion CircleRegion;
		if (Radius_Number > Radius){
			gen_circle(CircleRegion, 0, 0, Radius_value);
		}
		else if (Radius_Number + 0.5 == Radius){
			gen_circle(CircleRegion, 0, 0, Radius_value);
		}
		else{
			gen_circle(CircleRegion, 1, 1, Radius_value);
		}

		//step2:��Բ�νṹԪ������ͬ���������ڵ��г̱������һ��(��¼��ǰ�г̱���ĳ���/�п�ʼ/�н���/��ֵ)
		int numberParam;
		vector<ErosionDilationCircleParam> m_DilationCircleParam;
		ErosionDilationCircleParam l_DilationCircleParam;
		l_DilationCircleParam.len = CircleRegion.rle_x_end[0] - CircleRegion.rle_x_start[0] + 1;
		l_DilationCircleParam.col_start = CircleRegion.rle_x_start[0];
		l_DilationCircleParam.col_end = CircleRegion.rle_x_end[0];
		l_DilationCircleParam.row = CircleRegion.rle_y[0];
		l_DilationCircleParam.count++;
		numberParam = 0;
		m_DilationCircleParam.push_back(l_DilationCircleParam);
		for (int i = 1; i < Radius_value * 2; ++i){
			if ((CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1) == (CircleRegion.rle_x_end[i - 1] - CircleRegion.rle_x_start[i - 1] + 1)){
				m_DilationCircleParam[numberParam].count++;
			}
			else{
				ErosionDilationCircleParam temp_DilationCircleParam;
				temp_DilationCircleParam.len = CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1;
				temp_DilationCircleParam.col_start = CircleRegion.rle_x_start[i];
				temp_DilationCircleParam.col_end = CircleRegion.rle_x_end[i];
				temp_DilationCircleParam.row = CircleRegion.rle_y[i];
				temp_DilationCircleParam.count++;
				numberParam++;
				m_DilationCircleParam.push_back(temp_DilationCircleParam);
			}
		}

		//������ʼ��
		HRegion *Dilation_width_temp = new HRegion[m_DilationCircleParam.size() / 2 + 1];
		for (int i = 0; i < m_DilationCircleParam.size() / 2 + 1; ++i)
		{
			Dilation_width_temp[i].Image_Width = Region.Image_Width;
			Dilation_width_temp[i].Image_Height = Region.Image_Height;
		}

		HRegion *Dilation_Union_temp = new HRegion[m_DilationCircleParam.size() / 2 + 1];
		for (int i = 0; i < m_DilationCircleParam.size() / 2 + 1; ++i)
		{
			Dilation_Union_temp[i].Image_Width = Region.Image_Width;
			Dilation_Union_temp[i].Image_Height = Region.Image_Height;
		}

		HRegion *Dilation_Union = new HRegion[m_DilationCircleParam.size()];
		for (int i = 0; i < m_DilationCircleParam.size(); ++i)
		{
			Dilation_Union[i].Image_Width = Region.Image_Width;
			Dilation_Union[i].Image_Height = Region.Image_Height;
		}

		HRegion *Dilation_temp = new HRegion[m_DilationCircleParam.size()];
		for (int i = 0; i < m_DilationCircleParam.size(); ++i)
		{
			Dilation_temp[i].Image_Width = Region.Image_Width;
			Dilation_temp[i].Image_Height = Region.Image_Height;
		}

		//step3����ͬ���ȵľ��ο�������Ͽ�Ⱥ��Ӧ���г̱��������(����ĸ���Ϊstep1��һ���1,��ΪԲ�νṹԪ���ǶԳƵ�,��һ�뼴���Ƴ���һ��)
		for (int j = 0; j <m_DilationCircleParam.size() / 2 + 1; ++j){
			Dilation_width_temp[j].len = dilation_circle_row(Region, Dilation_width_temp[j], m_DilationCircleParam[j].col_start, m_DilationCircleParam[j].len, m_DilationCircleParam[j].row + m_DilationCircleParam[j].count - 1);
		}

		//step4������step2���г̱���ȡ����֮�����Ϣ
		for (int j = 0; j <m_DilationCircleParam.size() / 2 + 1; ++j){
			dilation_union(Dilation_width_temp[j], Dilation_Union_temp[j]);
		}

		//step5:����Բ�νṹԪ�����м䳤����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)
		dilation_union1(Dilation_Union_temp[m_DilationCircleParam.size() / 2], m_DilationCircleParam[m_DilationCircleParam.size() / 2], Dilation_Union[m_DilationCircleParam.size() / 2]);
		
		//step6:����Բ�νṹԪ�س�����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)(Բ�νṹԪ�����¶Գ�,�ƶ�yֵ���Ի����һ��)
		for (int j = 0; j <m_DilationCircleParam.size() / 2; ++j){
			dilation_union2(Dilation_Union_temp[j], m_DilationCircleParam[j], m_DilationCircleParam[m_DilationCircleParam.size() - j - 1], Dilation_Union[j], Dilation_Union[m_DilationCircleParam.size() - j - 1]);
		}

		//������ֵ,�Ż��ڴ洦��
		Dilation_temp[0] = Dilation_Union[0];

		//step7:�����г̱���Ĳ���
		for (int k = 1; k < m_DilationCircleParam.size(); ++k)
		{
			union2(Dilation_temp[k - 1], Dilation_Union[k], Dilation_temp[k]);
		}

		//step8:���г̵�ֵ�ƶ���ԭ���ֵ
		int offset_x;
		int offset_y;
		offset_x = 0;
		offset_y = 0;
		dilation_translate(Dilation_temp[m_DilationCircleParam.size() - 1], dst, offset_x, offset_y);

		//step9:�ڴ��ͷ�
		m_DilationCircleParam.clear();
		delete[] Dilation_width_temp;
		delete[] Dilation_Union_temp;
		delete[] Dilation_Union;
		delete[] Dilation_temp;
	}

	void closing_circle(const HRegion& src, HRegion& dst, double Radius){
		//�����ͺ�ʴ
		HRegion Region;
		Region.Image_Width = src.Image_Width;
		Region.Image_Height = src.Image_Height;

		//n.0<Radius<n.5֮��Radius=n.0;  n.5<=Radius<n.0+1֮��Radius=n.5; nΪ����;n>0
		int Radius_Number;
		double Radius_value;
		Radius_Number = max(saturate_cast<int>(Radius), 0);
		if (Radius_Number > Radius){
			Radius_value = Radius_Number - 0.5;
		}
		else if (Radius_Number + 0.5 == Radius){
			Radius_value = Radius;
		}
		else{
			Radius_value = Radius_Number;
		}

		//���Բ�νṹԪ��(������������迼��ԭ����ƶ�)
		//n.0<Radius<n.5֮��x��ƫ1,y��ƫ1;  n.5<=Radius<n.0+1֮�䲻��; nΪ����;n>0
		HRegion CircleRegion;
		if (Radius_Number > Radius){
			gen_circle(CircleRegion, 0, 0, Radius_value);
		}
		else if (Radius_Number + 0.5 == Radius){
			gen_circle(CircleRegion, 0, 0, Radius_value);
		}
		else{
			gen_circle(CircleRegion, 1, 1, Radius_value);
		}

		//step1:��Բ�νṹԪ������ͬ���������ڵ��г̱������һ��(��¼��ǰ�г̱���ĳ���/�п�ʼ/�н���/��ֵ)
		int numberParam;
		vector<ErosionDilationCircleParam> m_DilationCircleParam;
		ErosionDilationCircleParam l_DilationCircleParam;
		l_DilationCircleParam.len = CircleRegion.rle_x_end[0] - CircleRegion.rle_x_start[0] + 1;
		l_DilationCircleParam.col_start = CircleRegion.rle_x_start[0];
		l_DilationCircleParam.col_end = CircleRegion.rle_x_end[0];
		l_DilationCircleParam.row = CircleRegion.rle_y[0];
		l_DilationCircleParam.count++;
		numberParam = 0;
		m_DilationCircleParam.push_back(l_DilationCircleParam);
		for (int i = 1; i < Radius_value * 2; ++i){
			if ((CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1) == (CircleRegion.rle_x_end[i - 1] - CircleRegion.rle_x_start[i - 1] + 1)){
				m_DilationCircleParam[numberParam].count++;
			}
			else{
				ErosionDilationCircleParam temp_DilationCircleParam;
				temp_DilationCircleParam.len = CircleRegion.rle_x_end[i] - CircleRegion.rle_x_start[i] + 1;
				temp_DilationCircleParam.col_start = CircleRegion.rle_x_start[i];
				temp_DilationCircleParam.col_end = CircleRegion.rle_x_end[i];
				temp_DilationCircleParam.row = CircleRegion.rle_y[i];
				temp_DilationCircleParam.count++;
				numberParam++;
				m_DilationCircleParam.push_back(temp_DilationCircleParam);
			}
		}

		//������ʼ��
		HRegion *Dilation_width_temp = new HRegion[m_DilationCircleParam.size() / 2 + 1];
		for (int i = 0; i < m_DilationCircleParam.size() / 2 + 1; ++i)
		{
			Dilation_width_temp[i].Image_Width = src.Image_Width;
			Dilation_width_temp[i].Image_Height = src.Image_Height;
		}

		HRegion *Dilation_Union_temp = new HRegion[m_DilationCircleParam.size() / 2 + 1];
		for (int i = 0; i < m_DilationCircleParam.size() / 2 + 1; ++i)
		{
			Dilation_Union_temp[i].Image_Width = src.Image_Width;
			Dilation_Union_temp[i].Image_Height = src.Image_Height;
		}

		HRegion *Dilation_Union = new HRegion[m_DilationCircleParam.size()];
		for (int i = 0; i < m_DilationCircleParam.size(); ++i)
		{
			Dilation_Union[i].Image_Width = src.Image_Width;
			Dilation_Union[i].Image_Height = src.Image_Height;
		}

		HRegion *Dilation_temp = new HRegion[m_DilationCircleParam.size()];
		for (int i = 0; i < m_DilationCircleParam.size(); ++i)
		{
			Dilation_temp[i].Image_Width = src.Image_Width;
			Dilation_temp[i].Image_Height = src.Image_Height;
		}

		//step2����ͬ���ȵľ��ο�������Ͽ�Ⱥ��Ӧ���г̱��������(����ĸ���Ϊstep1��һ���1,��ΪԲ�νṹԪ���ǶԳƵ�,��һ�뼴���Ƴ���һ��)
		for (int j = 0; j <m_DilationCircleParam.size() / 2 + 1; ++j){
			Dilation_width_temp[j].len = dilation_circle_row(src, Dilation_width_temp[j], m_DilationCircleParam[j].col_start, m_DilationCircleParam[j].len, m_DilationCircleParam[j].row + m_DilationCircleParam[j].count - 1);
		}

		//step3������step2���г̱���ȡ����֮�����Ϣ
		for (int j = 0; j <m_DilationCircleParam.size() / 2 + 1; ++j){
			dilation_union(Dilation_width_temp[j], Dilation_Union_temp[j]);
		}

		//step4:����Բ�νṹԪ�����м䳤����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)
		dilation_union1(Dilation_Union_temp[m_DilationCircleParam.size() / 2], m_DilationCircleParam[m_DilationCircleParam.size() / 2], Dilation_Union[m_DilationCircleParam.size() / 2]);
		
		//step5:����Բ�νṹԪ�س�����������ڵ��г̱����໥ȡ������Ľ��(�����ο�����Ԫ��)(Բ�νṹԪ�����¶Գ�,�ƶ�yֵ���Ի����һ��)
		for (int j = 0; j <m_DilationCircleParam.size() / 2; ++j){
			dilation_union2(Dilation_Union_temp[j], m_DilationCircleParam[j], m_DilationCircleParam[m_DilationCircleParam.size() - j - 1], Dilation_Union[j], Dilation_Union[m_DilationCircleParam.size() - j - 1]);
		}

		//������ֵ,�Ż��ڴ洦��
		Dilation_temp[0] = Dilation_Union[0];

		//step6:�����г̱���Ĳ���
		for (int k = 1; k < m_DilationCircleParam.size(); ++k)
		{
			union2(Dilation_temp[k - 1], Dilation_Union[k], Dilation_temp[k]);
		}

		//step7:���г̵�ֵ�ƶ���ԭ���ֵ
		int offset_x;
		int offset_y;
		offset_x = 0;
		offset_y = 0;
		closing_translate(Dilation_temp[m_DilationCircleParam.size() - 1], Region, offset_x, offset_y);

		//step8:��ʴ
		erosion_circle(Region, dst, Radius);

		//step9:�ڴ��ͷ�
		m_DilationCircleParam.clear();
		delete[] Dilation_width_temp;
		delete[] Dilation_Union_temp;
		delete[] Dilation_Union;
		delete[] Dilation_temp;
	}

	void gen_circle(HRegion &Circle, double Row, double Column, double Radius){
		short* rle_x = Circle.rle_x_start;
		short* rle_y = Circle.rle_y;
		short* rle_x_end = Circle.rle_x_end;

		//n.0<Radius<n.5֮��Radius=n.0;  n.5<=Radius<n.0+1֮��Radius=n.5; nΪ����;n>0
		int Radius_Number;
		double Radius_value;
		Radius_Number = max(saturate_cast<int>(Radius), 0);
		if (Radius_Number > Radius){
			Radius_value = Radius_Number - 0.5;
		}
		else if (Radius_Number + 0.5 == Radius){
			Radius_value = Radius;
		}
		else{
			Radius_value = Radius_Number;
		}

		//��Բ�ΰ뾶Ϊ1.5
		if (Radius_value*2 == 3){
			*rle_x++ = Column ;
			*rle_y++ = Row  - 1;
			*rle_x_end++ = Column;

			*rle_x++ = Column -1;
			*rle_y++ = Row;
			*rle_x_end++ = Column + 1;

			*rle_x++ = Column;
			*rle_y++ = Row +1;
			*rle_x_end++ = Column;
		}
		else{//���ݵ�ǰ���dy,���㵱ǰ���dx,�Ӷ����㵱ǰԲ�ε��г̱���
			int offset = (int)((Radius_value*2.0 - 1.0) / 2.0);//ƫ����,(j1+(j2-1))/2,����ƫ��
			double r = Radius_value;
			double c = Radius_value;
			double inv_r2 = r ? 1. / ((double)r*r) : 0;
			for (int i = 0; i < Radius_value * 2; i++){
				int j1 = 0, j2 = 0;
				double dy = i + 0.5 - r;
				if (abs(dy) <= r){
					double dx = c*sqrt((r*r - dy*dy)*inv_r2);
					j1 = max(saturate_cast<int>(c - dx), 0);
					j2 = min(saturate_cast<int>(c + dx), Radius_value * 2);//���ֵȡ����
				}
				if (Radius_Number > Radius){
					*rle_x++ = Column + (j1 - offset);
					*rle_y++ = Row + (i - offset);
					*rle_x_end++ = Column + (j1 - offset) + (j2 - j1) - 1;
				}
				else if (Radius_Number + 0.5 == Radius){
					*rle_x++ = Column + (j1 - offset);
					*rle_y++ = Row + (i - offset);
					*rle_x_end++ = Column + (j1 - offset) + (j2 - j1) - 1;
				}
				else{
					*rle_x++ = Column + (j1 - offset) - 1;
					*rle_y++ = Row + (i - offset) - 1;
					*rle_x_end++ = Column + (j1 - offset) + (j2 - j1) - 1 - 1;
				}
			}
		}
		Circle.len = rle_y - Circle.rle_y;
	}

	void gen_rectangle1(HRegion &Rectangle,double Row1, double Column1, double Row2,double Column2){
		assert(Row1 < Row2);
		assert(Column1 < Column2);
		short* p_rle_x = Rectangle.rle_x_start;
		short* p_rle_end_x = Rectangle.rle_x_end;
		short* p_rle_y = Rectangle.rle_y;
		for (double row = Row1; row <= Row2; row++){
			*p_rle_x++ = short(Column1);
			*p_rle_end_x++ = short(Column2);
			*p_rle_y++ = row;
		}
		Rectangle.len = int(Row2 - Row1 + 1);
	}

	void intersection1(const ListHRegion& rle, HRegion& dst){//region�ϲ�
		ListHRegion temp;
		temp.Region = new HRegion[rle.count];//�����ڴ�
		if (rle.count> 0){
			if (rle.count == 1){
				dst = rle.Region[0];
			}
			else{
				temp.Region[0] = rle.Region[0];
				for (int i = 1; i < rle.count; i++){
					intersection2(temp.Region[i - 1], rle.Region[i], temp.Region[i]);
				}
				dst = temp.Region[rle.count - 1];
			}
		}
		delete[] temp.Region;
	}

	void intersection2(const HRegion& rle, const HRegion &Region, HRegion& dst){
		//step1���жϵ�ǰ����Region�Ƿ��������
		dst.Image_Width = rle.Image_Width;
		dst.Image_Height = rle.Image_Height;
		if (rle.len == 0 && Region.len == 0){
			return;
		}
		else if (rle.len == 0){
			for (int i = 0; i < Region.len; i++){
				dst.rle_x_start[i] = Region.rle_x_start[i];
				dst.rle_x_end[i] = Region.rle_x_end[i];
				dst.rle_y[i] = Region.rle_y[i];
			}
			dst.len = Region.len;
			return;
		}
		else if (Region.len == 0){
			for (int i = 0; i < rle.len; i++){
				dst.rle_x_start[i] = rle.rle_x_start[i];
				dst.rle_x_end[i] = rle.rle_x_end[i];
				dst.rle_y[i] = rle.rle_y[i];
			}
			dst.len = rle.len;
			return;
		}

		//step2�����rle��ͬһ�е������г̱���
		short* last_union_rle_y = dst.rle_y;
		int last_id = 0;
		int row_start_idx = 0;
		int* row_starts = new int[5000];//��ǰ�п�ʼ�� ���һ�п�ʼ��ֵ~��һ�н�����ֵ
		int* row_ends = new int[5000];//��ǰ�н�����
		int nrows = 0;
		short* rle_x_start = dst.rle_x_start;
		short* rle_y = dst.rle_y;
		short* rle_x_end = dst.rle_x_end;
		for (int id = 1; id < rle.len; ++id)
		{
			if (rle.rle_y[id] - rle.rle_y[last_id]>0)
			{
				row_starts[nrows] = row_start_idx;
				row_ends[nrows] = last_id;
				nrows++;
				row_start_idx = id;
			}
			last_id = id;
		}
		row_starts[nrows] = row_start_idx;
		row_ends[nrows] = last_id;
		nrows++;

		//step3:���Region��ͬһ�е������г̱���
		int region_last_id = 0;
		int region_row_start_idx = 0;
		int* region_row_starts = new int[5000];//��ǰ�п�ʼ�� ���һ�п�ʼ��ֵ~��һ�н�����ֵ
		int* region_row_ends = new int[5000];//��ǰ�н�����
		int region_nrows = 0;
		for (int Region_id = 1; Region_id < Region.len; ++Region_id)
		{
			if (Region.rle_y[Region_id] - Region.rle_y[region_last_id]>0)
			{
				region_row_starts[region_nrows] = region_row_start_idx;
				region_row_ends[region_nrows] = region_last_id;
				region_nrows++;
				region_row_start_idx = Region_id;
			}
			region_last_id = Region_id;
		}
		region_row_starts[region_nrows] = region_row_start_idx;
		region_row_ends[region_nrows] = region_last_id;
		region_nrows++;

		//step4:������Region������Ϣ��ͬʱ,���㵱ǰ�еĽ���
		int rle_count = 0;
		int region_count = 0;
		BOOL end_flag = false;
		while (nrows != rle_count && region_nrows != region_count)
		{
			/*if (nrows <= rle_count || region_nrows <= region_count){
			break;
			}
			else */if (rle.rle_y[row_ends[rle_count]] == Region.rle_y[region_row_ends[region_count]]){//������Region���г̱�����ͬһ��
				intersect_two_row(rle.rle_x_start, rle.rle_x_end, Region.rle_x_start, Region.rle_x_end,
					&rle_x_start, &rle_y, &rle_x_end, rle.rle_y[row_ends[rle_count]],
					row_starts[rle_count], row_ends[rle_count] + 1, region_row_starts[region_count], region_row_ends[region_count] + 1);
				rle_count++;
				region_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] > Region.rle_y[region_row_ends[region_count]]){//Region����ֵ����Region1����ֵ,��Region1����
				region_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] < Region.rle_y[region_row_ends[region_count]]){//Region����ֵС��Region1����ֵ,��Region����
				rle_count++;
			}
		}

		//�ڴ��ͷ�
		delete[] row_starts;
		delete[] row_ends;
		delete[] region_row_starts;
		delete[] region_row_ends;
		dst.len = int(rle_y - last_union_rle_y);
	}

	void union1(const ListHRegion& rle, HRegion& dst){
		ListHRegion temp;
		temp.Region = new HRegion[rle.count];//�����ڴ�
		if (rle.count> 0){
			if (rle.count == 1){
				dst = rle.Region[0];
			}
			else{
				temp.Region[0] = rle.Region[0];
				for (int i = 1; i < rle.count; i++){
					union2(temp.Region[i - 1], rle.Region[i], temp.Region[i]);
				}
				dst = temp.Region[rle.count - 1];
			}
		}
		delete[] temp.Region;
	}

	void union2(const HRegion& rle, const HRegion &Region, HRegion& dst){
		//step1:�жϵ�ǰ����Region�Ƿ����������
		dst.Image_Width = rle.Image_Width;
		dst.Image_Height = rle.Image_Height;
		if (rle.len == 0 && Region.len == 0){
			return;
		}
		else if (rle.len == 0){
			dst = Region;
			return;
		}
		else if (Region.len == 0){
			dst = rle;
			return;
		}

		//step2:���rle��ͬһ�е������г̱���
		short* last_union_rle_y = dst.rle_y;
		int last_id = 0;
		int row_start_idx = 0;
		int* row_starts = new int[5000];//��ǰ�п�ʼ�� ���һ�п�ʼ��ֵ~��һ�н�����ֵ
		int* row_ends = new int[5000];//��ǰ�н�����
		int nrows = 0;
		short* rle_x_start = dst.rle_x_start;
		short* rle_y = dst.rle_y;
		short* rle_x_end = dst.rle_x_end;
		for (int id = 1; id < rle.len; ++id)
		{
			if (rle.rle_y[id] - rle.rle_y[last_id]>0)
			{
				row_starts[nrows] = row_start_idx;
				row_ends[nrows] = last_id;
				nrows++;
				row_start_idx = id;
			}
			last_id = id;
		}
		row_starts[nrows] = row_start_idx;
		row_ends[nrows] = last_id;
		nrows++;

		//step3:���Region��ͬһ�е������г̱���
		int region_last_id = 0;
		int region_row_start_idx = 0;
		int* region_row_starts = new int[5000];//��ǰ�п�ʼ�� ���һ�п�ʼ��ֵ~��һ�н�����ֵ
		int* region_row_ends = new int[5000];//��ǰ�н�����
		int region_nrows = 0;
		for (int Region_id = 1; Region_id < Region.len; ++Region_id)
		{
			if (Region.rle_y[Region_id] - Region.rle_y[region_last_id]>0)
			{
				region_row_starts[region_nrows] = region_row_start_idx;
				region_row_ends[region_nrows] = region_last_id;
				region_nrows++;
				region_row_start_idx = Region_id;
			}
			region_last_id = Region_id;
		}
		region_row_starts[region_nrows] = region_row_start_idx;
		region_row_ends[region_nrows] = region_last_id;
		region_nrows++;

		//step4:������Region������Ϣ��ͬʱ,���㵱ǰ�еĲ���;
		//�в�ͬʱ,ȡ�н�С����
		//���г���Region����ֵ,ȡ��һ��Region��Ϣ
		int rle_count = 0;
		int region_count = 0;
		BOOL end_flag = false;
		while (1)
		{
			if (nrows <= rle_count && region_nrows <= region_count){//����Region��������
				break;
			}
			else if (nrows == rle_count){//rle������,���ʣ�µ�Regionֵ
				for (int index = region_row_starts[region_count]; index <region_row_ends[region_count] + 1; index++){
					*rle_x_start++ = Region.rle_x_start[index];
					*rle_y++ = Region.rle_y[index];
					*rle_x_end++ = Region.rle_x_end[index];
				}
				region_count++;
			}
			else if (region_nrows == region_count){//Region������,���ʣ��rleֵ
				for (int index = row_starts[rle_count]; index <row_ends[rle_count] + 1; index++){
					*rle_x_start++ = rle.rle_x_start[index];
					*rle_y++ = rle.rle_y[index];
					*rle_x_end++ = rle.rle_x_end[index];
				}
				rle_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] == Region.rle_y[region_row_ends[region_count]]){//rle����Ϣ����Region����Ϣ
				union_two_row(rle.rle_x_start, rle.rle_x_end, Region.rle_x_start, Region.rle_x_end,
					&rle_x_start, &rle_y, &rle_x_end, rle.rle_y[row_ends[rle_count]],
					row_starts[rle_count], row_ends[rle_count] + 1, region_row_starts[region_count], region_row_ends[region_count] + 1);
				rle_count++;
				region_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] > Region.rle_y[region_row_ends[region_count]]){//rle��ǰ����Ϣ����Region��ǰ����Ϣ,��õ�ǰ��Regionֵ
				for (int index = region_row_starts[region_count]; index <region_row_ends[region_count] + 1; index++){
					*rle_x_start++ = Region.rle_x_start[index];
					*rle_y++ = Region.rle_y[index];
					*rle_x_end++ = Region.rle_x_end[index];
				}
				region_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] < Region.rle_y[region_row_ends[region_count]]){//rle��ǰ����ϢС��Region��ǰ����Ϣ,��õ�ǰ��rleֵ
				for (int index = row_starts[rle_count]; index <row_ends[rle_count] + 1; index++){
					*rle_x_start++ = rle.rle_x_start[index];
					*rle_y++ = rle.rle_y[index];
					*rle_x_end++ = rle.rle_x_end[index];
				}
				rle_count++;
			}
		}

		//�ڴ��ͷ�
		delete[] row_starts;
		delete[] row_ends;
		delete[] region_row_starts;
		delete[] region_row_ends;
		dst.len = int(rle_y - last_union_rle_y);
	}

	void difference(const HRegion& rle, const HRegion &Region, HRegion& dst){
		//step1:�жϵ�ǰ����Region�Ƿ��������
		dst.Image_Width = rle.Image_Width;
		dst.Image_Height = rle.Image_Height;
		if (rle.len == 0 && Region.len == 0){
			return;
		}
		else if (rle.len == 0){
			for (int i = 0; i < Region.len; i++){
				dst.rle_x_start[i] = Region.rle_x_start[i];
				dst.rle_x_end[i] = Region.rle_x_end[i];
				dst.rle_y[i] = Region.rle_y[i];
			}
			dst.len = Region.len;
			return;
		}
		else if (Region.len == 0){
			for (int i = 0; i < rle.len; i++){
				dst.rle_x_start[i] = rle.rle_x_start[i];
				dst.rle_x_end[i] = rle.rle_x_end[i];
				dst.rle_y[i] = rle.rle_y[i];
			}
			dst.len = rle.len;
			return;
		}

		//step2:���rle��ͬһ�е������г̱���
		short* last_union_rle_y = dst.rle_y;
		int last_id = 0;
		int row_start_idx = 0;
		int* row_starts = new int[5000];
		int* row_ends = new int[5000];
		int nrows = 0;
		short* rle_x_start = dst.rle_x_start;
		short* rle_y = dst.rle_y;
		short* rle_x_end = dst.rle_x_end;
		for (int id = 1; id < rle.len; ++id)
		{
			if (rle.rle_y[id] - rle.rle_y[last_id]>0)
			{
				row_starts[nrows] = row_start_idx;
				row_ends[nrows] = last_id;
				nrows++;
				row_start_idx = id;
			}
			last_id = id;
		}
		row_starts[nrows] = row_start_idx;
		row_ends[nrows] = last_id;
		nrows++;

		//step3:���Region��ͬһ�е������г̱���
		int region_last_id = 0;
		int region_row_start_idx = 0;
		int* region_row_starts = new int[5000];
		int* region_row_ends = new int[5000];
		int region_nrows = 0;
		for (int Region_id = 1; Region_id < Region.len; ++Region_id)
		{
			if (Region.rle_y[Region_id] - Region.rle_y[region_last_id]>0)
			{
				region_row_starts[region_nrows] = region_row_start_idx;
				region_row_ends[region_nrows] = region_last_id;
				region_nrows++;
				region_row_start_idx = Region_id;
			}
			region_last_id = Region_id;
		}
		region_row_starts[region_nrows] = region_row_start_idx;
		region_row_ends[region_nrows] = region_last_id;
		region_nrows++;

		//step4:������Region������Ϣ��ͬʱ,���㵱ǰ�еĲ;
		//ȡrle���ڶ�Region�����ڵ��г̱���
		int rle_count = 0;
		int region_count = 0;
		BOOL end_flag = false;
		while (1)
		{
			if (nrows <= rle_count && region_nrows <= region_count){
				break;
			}
			else if (nrows == rle_count){
				region_count++;
			}
			else if (region_nrows == region_count){
				for (int index = row_starts[rle_count]; index <row_ends[rle_count] + 1; index++){
					*rle_x_start++ = rle.rle_x_start[index];
					*rle_y++ = rle.rle_y[index];
					*rle_x_end++ = rle.rle_x_end[index];
				}
				rle_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] == Region.rle_y[region_row_ends[region_count]]){
				difference_two_row(rle.rle_x_start, rle.rle_x_end, Region.rle_x_start, Region.rle_x_end,
					&rle_x_start, &rle_y, &rle_x_end, rle.rle_y[row_ends[rle_count]],
					row_starts[rle_count], row_ends[rle_count] + 1, region_row_starts[region_count], region_row_ends[region_count] + 1);
				rle_count++;
				region_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] > Region.rle_y[region_row_ends[region_count]]){
				region_count++;
			}
			else if (rle.rle_y[row_ends[rle_count]] < Region.rle_y[region_row_ends[region_count]]){
				for (int index = row_starts[rle_count]; index <row_ends[rle_count] + 1; index++){
					*rle_x_start++ = rle.rle_x_start[index];
					*rle_y++ = rle.rle_y[index];
					*rle_x_end++ = rle.rle_x_end[index];
				}
				rle_count++;
			}
		}

		//step5:�ͷ��ڴ�
		delete[] row_starts;
		delete[] row_ends;
		delete[] region_row_starts;
		delete[] region_row_ends;
		dst.len = int(rle_y - last_union_rle_y);

	}

	void concat_obj(const HRegion&rle, const HRegion &Region, ListHRegion& dst){
		dst.Region[dst.count] = rle;
		dst.count++;
		dst.Region[dst.count] = Region;
		dst.count++;
	}

	void connection(const HRegion& Region, HRegion& ConnectedRegions){
		/*�㷨����:
		1.����һ�������г̱�����
		2.���ڳ��˵�һ���������������ţ��������ǰһ���е������Ŷ�û���غ����������һ���µı��;
		�����������һ����һ�������غ���������һ�е��Ǹ��ŵı�Ÿ�����;
		���������һ�е�2�����ϵ������ص����������ǰ�Ÿ�һ�������ŵ���С��ţ�������һ�е��⼸���ŵı��д��ȼ۶ԣ�˵����������һ��
		3.���ȼ۶�ת��Ϊ�ȼ����У�ÿһ��������Ҫ��һ��ͬ�ı�ţ���Ϊ���Ƕ��ǵȼ۵�
		4.������ʼ�ŵı��,���ҵȼ�����,�����µı��
		*/
		
		//������ʼ��
		ConnectedRegions.Image_Width = Region.Image_Width;
		ConnectedRegions.Image_Height = Region.Image_Height;
		vector<int> runLabels;
		runLabels.assign(Region.len, 0);

		//�����г̱��������ĸ�Region
		int n = labeling(Region.rle_x_start, Region.rle_x_end, Region.rle_y, Region.len, runLabels, 1);

		//��ֵ
		ConnectedRegions = Region;
		ConnectedRegions.len = Region.len;
		ConnectedRegions.runLabels = runLabels;
		ConnectedRegions.runLabels_Count = n;
		runLabels.clear();
	}  

	void select_obj(const HRegion& Region, HRegion& SelectedRegions,int Region_ID){
		SelectedRegions.Image_Width = Region.Image_Width;
		SelectedRegions.Image_Height = Region.Image_Height;
		short* Selected_x_start = SelectedRegions.rle_x_start;
		short* Selected_y = SelectedRegions.rle_y;
		short* Selected_x_end = SelectedRegions.rle_x_end;

		assert(0<Region_ID&&Region_ID<=Region.runLabels_Count);
		for (int i = 0; i < Region.runLabels.size();i++){
			if (Region.runLabels[i] == Region_ID){
				*Selected_x_start++ = Region.rle_x_start[i];
				*Selected_y++ = Region.rle_y[i];
				*Selected_x_end++ = Region.rle_x_end[i];
				SelectedRegions.len++;
			}
		}
	}

	void select_shape(const HRegion& Region, HRegion& SelectedRegions,CondParam& m_CondParam){

		//new
		SelectedRegions.Image_Width = Region.Image_Width;
		SelectedRegions.Image_Height = Region.Image_Height;
		//������ʼ��
		CondParam m_Select_CondParam;
		
		short* Selected_x_start = SelectedRegions.rle_x_start;
		short* Selected_y = SelectedRegions.rle_y;
		short* Selected_x_end = SelectedRegions.rle_x_end;
		
		CondParam *ve_CondParam = new CondParam[Region.runLabels_Count];
		for (int i = 0; i < Region.runLabels_Count;i++){
			ve_CondParam[i].Width.Min = 9999;
			ve_CondParam[i].Height.Min = 9999;
			ve_CondParam[i].Area.Value = 9999;
		}

		int Rle_Number = (int)Region.runLabels.size();

		//��ȼ���,��õ�ǰRegion�������ֵ����С���ֵ,��������
		//�߶ȼ���,��õ�ǰRegion�����߶�ֵ����С�߶�ֵ,��������
		//�������,����ǰRegion�����г̱��볤�ȼ�����

		//���Region�����ֵ/��С���ֵ,���߶�ֵ/��С�߶�ֵ/���
		for (int Index = 0; Index < Rle_Number; Index++){
			if (ve_CondParam[Region.runLabels[Index] - 1].Width.Min>Region.rle_x_start[Index]){
				ve_CondParam[Region.runLabels[Index] - 1].Width.Min = Region.rle_x_start[Index];
			}
			if (ve_CondParam[Region.runLabels[Index] - 1].Width.Max<Region.rle_x_end[Index]){
				ve_CondParam[Region.runLabels[Index] - 1].Width.Max = Region.rle_x_end[Index];
			}
			//ve_CondParam[Region.runLabels[Index] - 1].Width.Name = "width";
			if (ve_CondParam[Region.runLabels[Index] - 1].Height.Min>Region.rle_y[Index]){
				ve_CondParam[Region.runLabels[Index] - 1].Height.Min = Region.rle_y[Index];
			}
			if (ve_CondParam[Region.runLabels[Index] - 1].Height.Max<Region.rle_y[Index]){
				ve_CondParam[Region.runLabels[Index] - 1].Height.Max = Region.rle_y[Index];
			}
			//ve_CondParam[Region.runLabels[Index] - 1].Height.Name = "height";
			ve_CondParam[Region.runLabels[Index] - 1].Area.Value = 
				ve_CondParam[Region.runLabels[Index] - 1].Area.Value + 
				Region.rle_x_end[Index] - Region.rle_x_start[Index] + 1;
			//ve_CondParam[Region.runLabels[Index] - 1].Area.Name = "area";
		}
	
		//���Region�Ŀ��ֵ�͸߶�ֵ
		int Regions_Number = Region.runLabels_Count;
		for (int j = 0; j < Regions_Number; j++){
			ve_CondParam[j].Width.Value = ve_CondParam[j].Width.Max - ve_CondParam[j].Width.Min + 1;
			ve_CondParam[j].Height.Value = ve_CondParam[j].Height.Max - ve_CondParam[j].Height.Min + 1;
		}

		//�ж�m_CondParam width/height/area�Ƿ����(�Ƿ񱻵���)
		if (m_CondParam.Width.Max > 0){
			//m_Select_CondParam.Width.Name = "width";
			m_Select_CondParam.Width.flag = true;
			m_Select_CondParam.Width.Min = m_CondParam.Width.Min;
			m_Select_CondParam.Width.Max = m_CondParam.Width.Max;
		}

		if (m_CondParam.Height.Max > 0){
			//m_Select_CondParam.Height.Name = "height";
			m_Select_CondParam.Height.flag = true;
			m_Select_CondParam.Height.Min = m_CondParam.Height.Min;
			m_Select_CondParam.Height.Max = m_CondParam.Height.Max;
		}

		if (m_CondParam.Area.Max > 0){
			//m_Select_CondParam.Area.Name = "area";
			m_Select_CondParam.Area.flag = true;
			m_Select_CondParam.Area.Min = m_CondParam.Area.Min;
			m_Select_CondParam.Area.Max = m_CondParam.Area.Max;
		}

		int *m_select_runLabels = new int[Rle_Number];
		int m_select_runLabels_count = 0;
		for (int k = 0; k < Rle_Number; k++){
			if (m_Select_CondParam.Width.flag == true){//�жϿ�������Ƿ����
				if (m_Select_CondParam.Width.Min<ve_CondParam[Region.runLabels[k] - 1].Width.Value &&
					m_Select_CondParam.Width.Max>ve_CondParam[Region.runLabels[k] - 1].Width.Value){
					ve_CondParam[Region.runLabels[k] - 1].Width.flag = true;
				}
			}

			if (m_Select_CondParam.Height.flag == true){//�жϸ߶������Ƿ����
				if (m_Select_CondParam.Height.Min<ve_CondParam[Region.runLabels[k] - 1].Height.Value &&
					m_Select_CondParam.Height.Max>ve_CondParam[Region.runLabels[k] - 1].Height.Value){
					ve_CondParam[Region.runLabels[k] - 1].Height.flag = true;
				}
			}

			if (m_Select_CondParam.Area.flag == true){//�ж���������Ƿ����
				if (m_Select_CondParam.Area.Min<ve_CondParam[Region.runLabels[k] - 1].Area.Value &&
					m_Select_CondParam.Area.Max>ve_CondParam[Region.runLabels[k] - 1].Area.Value){
					ve_CondParam[Region.runLabels[k] - 1].Area.flag = true;
				}
			}

			//��������ͬʱ����,������д��Region
			if (m_Select_CondParam.Width.flag == ve_CondParam[Region.runLabels[k] - 1].Width.flag &&
				m_Select_CondParam.Height.flag == ve_CondParam[Region.runLabels[k] - 1].Height.flag &&
				m_Select_CondParam.Area.flag == ve_CondParam[Region.runLabels[k] - 1].Area.flag)
			{
				*Selected_x_start++ = Region.rle_x_start[k];
				*Selected_x_end++ = Region.rle_x_end[k];
				*Selected_y++ = Region.rle_y[k];
				//��ɸѡ�����Region���±���
				if (m_select_runLabels_count == 0){
					m_select_runLabels[0] = Region.runLabels[k];
					m_select_runLabels_count++;
				}else{
					if (IsEqualInt(Region.runLabels[k], m_select_runLabels, m_select_runLabels_count) == -1){
						m_select_runLabels[m_select_runLabels_count] = Region.runLabels[k];
						m_select_runLabels_count++;
					}
				}
				int  matchID = IsEqualInt(Region.runLabels[k], m_select_runLabels, m_select_runLabels_count);
				if (matchID == -1){
				} else{
					SelectedRegions.runLabels.push_back(matchID + 1);
				}
			}
		}

		SelectedRegions.runLabels_Count = m_select_runLabels_count;
		SelectedRegions.len = int(Selected_y - SelectedRegions.rle_y);

		delete[] ve_CondParam;
		delete[] m_select_runLabels;
	}

	void area_center(HRegion& Region, int &Area, float &Row, float &Column){
		Area = Region.Area();

		int Sum_Row = 0;
		int Sum_Column = 0;
		int Sum_count = 0;
		for (int i = 0; i < Region.len;i++){
			Sum_Row += Sum_Row + Region.rle_y[i] * (Region.rle_x_end[i] - Region.rle_x_start[i] + 1);
			Sum_Column += Sum_Column + (Region.rle_x_end[i] + Region.rle_x_start[i])/2 
				*(Region.rle_x_end[i] - Region.rle_x_start[i] + 1);
			Sum_count = Sum_count + 1 * (Region.rle_x_end[i] - Region.rle_x_start[i] + 1);
		}
		Row = (float)Sum_Row / (float)Sum_count;
		Column = (float)Sum_Column / (float)Sum_count;
	}

	void select_shape_max_std(const HRegion& Region, HRegion& SelectedRegions){
		//������ʼ��
		SelectedRegions.Image_Width = Region.Image_Width;
		SelectedRegions.Image_Height = Region.Image_Height;

		CondParam l_CondParam;
		CondParam m_Select_CondParam;
		vector<CondParam> ve_CondParam;

		short* Selected_x_start = SelectedRegions.rle_x_start;
		short* Selected_y = SelectedRegions.rle_y;
		short* Selected_x_end = SelectedRegions.rle_x_end;

		l_CondParam.Area.Value = 0;
		ve_CondParam.assign(Region.runLabels_Count, l_CondParam);

		//���Region���
		int Rle_Number = (int)Region.runLabels.size();
		for (int Index = 0; Index < Rle_Number; Index++){
			ve_CondParam[Region.runLabels[Index] - 1].Area.Value =
				ve_CondParam[Region.runLabels[Index] - 1].Area.Value +
				Region.rle_x_end[Index] - Region.rle_x_start[Index] + 1;
			//ve_CondParam[Region.runLabels[Index] - 1].Area.Name = "area";
		}

		//������������ID
		int SelectedID;
		int Regions_Number = Region.runLabels_Count;
		for (int i = 0; i < Regions_Number;i++){
			if (m_Select_CondParam.Area.Value <ve_CondParam[i].Area.Value){
				m_Select_CondParam.Area.Value = ve_CondParam[i].Area.Value;
				SelectedID = i + 1;
			}
		}

		//����������������г̱���
		for (int k = 0; k < Rle_Number; k++){
			if (SelectedID == Region.runLabels[k]){
				*Selected_x_start++ = Region.rle_x_start[k];
				*Selected_x_end++ = Region.rle_x_end[k];
				*Selected_y++ = Region.rle_y[k];
			}
		}
		SelectedRegions.len = int(Selected_y - SelectedRegions.rle_y);
		ve_CondParam.clear();
	}
}

//erosion_rectangle start
int erosion_blocks(const HRegion& rle, HRegion& dst, int height, int rank)
{
	//region
	int region_nrows = 0;
	HRegion Region;
	Region.Image_Width = rle.Image_Width;
	Region.Image_Height = rle.Image_Height;
	short* Region_x_start = Region.rle_x_start;
	short* Region_y = Region.rle_y;
	short* Region_x_end = Region.rle_x_end;

	//�����Ҫ�������Ĵ���,�ɸ߶Ⱦ���ʱ�临�Ӷ�Ϊ(log2^(n))
	int offset = 1;
	int step = 1;
	while (step < height)
	{
		step <<= 1;
		++offset;
	}

	for (int id = 0; id < rle.len; ++id)
	{
		//ʹ�þ���Ԫ������ʴ,ͬһ�����п�ʼ���н�������ͬ,������Ϣ��ͬ(����Ϣ���2^(n-1),n��ʾ��ǰ����)
		if (rank == 1){//��һ������2^0 = 1
			*Region_y++ = rle.rle_y[id] - 1;
		}
		else if (offset - 1 > rank){//��2~(n-1)��,��2^(n-1)
			*Region_y++ = rle.rle_y[id] - (1 << (rank - 1));
		}
		else{
			if (1 << rank == height){//��n��,��ǰ��Ϊ��ȫ��,��2^(n-1)
				*Region_y++ = rle.rle_y[id] - (1 << (rank - 1));
			}
			else{//��n��,��ǰ��Ϊ����ȫ��,��height-2^(n-1)
				*Region_y++ = rle.rle_y[id] - (height - (1 << (rank - 1)));
			}
		}
		*Region_x_start++ = rle.rle_x_start[id];
		*Region_x_end++ = rle.rle_x_end[id];
	}
	Region.len = rle.len;
	//����Region�Ľ���
	blob::intersection2(rle, Region, dst);
	return dst.len;
}

//��ԭ�㻹ԭ��ԭ��
void translate(const HRegion& rle, HRegion& dst, int offset_x, int offset_y)
{
	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;
	for (int i = 0; i < rle.len; ++i)
	{
		//ȥ��������ͼ��ĸ߶�
		if (rle.rle_y[i] + offset_y >= 0 && rle.rle_y[i] + offset_y<rle.Image_Height){
			*rle_y++ = rle.rle_y[i] + offset_y;
			*rle_x_end++ = rle.rle_x_end[i] + offset_x;
			*rle_x_start++ = rle.rle_x_start[i] + offset_x;
		}
	}
	dst.len = rle_y - dst.rle_y;
}

//ȥ����������Ҫ����г�
int erosion_rectangel_row(const HRegion& rle, HRegion& dst, int width){
	int rle_cnt = rle.len;
	const short* p_rle_start_x = rle.rle_x_start;
	const short* p_rle_y = rle.rle_y;
	const short* p_rle_end_x = rle.rle_x_end;

	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;

	for (int id = 0; id < rle_cnt; ++id)
	{
		//������Ϊwidth���г̱���
		if (p_rle_end_x[id] - p_rle_start_x[id] + 1 >= width)
		{
			//ԭ��Ϊ���
			*rle_x_start++ = rle.rle_x_start[id];
			*rle_x_end++ = rle.rle_x_end[id] - width + 1;//���Ϊ10,�г�Ϊ10,��ʴ����,��1
			*rle_y++ = rle.rle_y[id];
		}
	}

	return int(rle_y - dst.rle_y);
}
//end

//dilation_rectangle1 start

//����ǰ���г̱���������
int dilation_rectangel_row(const HRegion& rle, HRegion& dst, int width){
	int rle_cnt = rle.len;

	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;

	for (int id = 0; id < rle_cnt; ++id)
	{
		//ԭ��Ϊ���
		*rle_x_start++ = rle.rle_x_start[id] - width + 1;//���Ϊ10,�г�Ϊ10,���͹���,�п�ʼ��ǰ�ƶ�width-1
		*rle_x_end++ = rle.rle_x_end[id];
		*rle_y++ = rle.rle_y[id];
	}
	return int(rle_y - dst.rle_y);
}

int dilation_blocks(const HRegion& rle, HRegion& dst, int height, int rank)
{
	//region
	int region_nrows = 0;
	HRegion Region;
	Region.Image_Width = rle.Image_Width;
	Region.Image_Height = rle.Image_Height;
	short* Region_x_start = Region.rle_x_start;
	short* Region_y = Region.rle_y;
	short* Region_x_end = Region.rle_x_end;

	//�����Ҫ�������Ĵ���,�ɸ߶Ⱦ���ʱ�临�Ӷ�Ϊ(log2^(n))
	int offset=1;
	int step = 1;
	while (step < height)
	{
		step <<= 1;
		++offset;
	}

	for (int id = 0; id < rle.len; ++id)
	{
		//ʹ�þ���Ԫ��������,ͬһ�����п�ʼ���н�������ͬ,������Ϣ��ͬ(����Ϣ���2^(n-1),n��ʾ��ǰ����)
		if (rank ==1){//��һ������2^0 = 1
			*Region_y++ = rle.rle_y[id] - 1;
		}
		else if (offset-1 > rank){//��2~(n-1)��,��2^(n-1)
			*Region_y++ = rle.rle_y[id] - (1 << (rank - 1));
		}
		else{
			if (1 << rank == height){//��n��,��ǰ��Ϊ��ȫ��,��2^(n-1)
				*Region_y++ = rle.rle_y[id] - (1 << (rank - 1));
			}else{//��n��,��ǰ��Ϊ����ȫ��,��height-2^(n-1)
				*Region_y++ = rle.rle_y[id] - (height - (1 << (rank - 1)));
			}
		}
		*Region_x_start++ = rle.rle_x_start[id];
		*Region_x_end++ = rle.rle_x_end[id];
	}
	Region.len = rle.len;
	//����Region�Ĳ���
	blob::union2(rle,Region,dst);
	return dst.len;
}

void dilation_union(const HRegion& rle, HRegion& dst){
	//���rle��ͬһ�е������г̱���
	short* last_union_rle_y = dst.rle_y;
	int last_id = 0;
	int row_start_idx = 0;
	int* row_starts = new int[5000];
	int* row_ends = new int[5000];
	int nrows = 0;
	short* rle_x_start = dst.rle_x_start;
	short* rle_y = dst.rle_y;
	short* rle_x_end = dst.rle_x_end;
	for (int id = 1; id < rle.len; ++id)
	{
		if (rle.rle_y[id] - rle.rle_y[last_id] > 0)
		{
			row_starts[nrows] = row_start_idx;
			row_ends[nrows] = last_id;
			nrows++;
			row_start_idx = id;
		}
		last_id = id;
	}
	row_starts[nrows] = row_start_idx;
	row_ends[nrows] = last_id;
	nrows++;
	//��Region�ĵ�ǰ��ȡ����
	for (int rle_count = 0; rle_count < nrows; rle_count++){
		union_two_row(rle.rle_x_start, rle.rle_x_end, rle.rle_x_start, rle.rle_x_end, &rle_x_start, &rle_y, &rle_x_end,
			rle.rle_y[row_ends[rle_count]], row_starts[rle_count], row_ends[rle_count] + 1, row_starts[rle_count], row_ends[rle_count] + 1);
	}
	delete[] row_starts;
	delete[] row_ends;
	dst.len = int(rle_y - last_union_rle_y);
}

void dilation_translate(const HRegion& rle, HRegion& dst, int offset_x, int offset_y){
	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;
	for (int i = 0; i < rle.len; ++i)
	{
		//ȥ��������ͼ��ĸ߶�
		if (rle.rle_y[i] + offset_y >= 0 && rle.rle_y[i] + offset_y<rle.Image_Height){
			//���г̱����ȳ���ͼ����
			if (rle.rle_x_start[i] + offset_x <= 0 && rle.rle_x_end[i] + offset_x >= rle.Image_Width){
				*rle_x_start++ = 0;
				*rle_x_end++ = rle.Image_Width - 1;
			}
			else if (rle.rle_x_start[i] + offset_x <= 0){
				*rle_x_start++ = 0;
				*rle_x_end++ = rle.rle_x_end[i] + offset_x;
			}
			else if (rle.rle_x_end[i] + offset_x >= rle.Image_Width){
				*rle_x_start++ = rle.rle_x_start[i] + offset_x;
				*rle_x_end++ = rle.Image_Width - 1;
			}
			else{//û����ͼ����
				*rle_x_end++ = rle.rle_x_end[i] + offset_x;
				*rle_x_start++ = rle.rle_x_start[i] + offset_x;
			}
			*rle_y++ = rle.rle_y[i] + offset_y;
		}
	}
	dst.len = rle_y - dst.rle_y;
}
//end

//closing_rectangle start 
void closing_translate(const HRegion& rle, HRegion& dst, int offset_x, int offset_y)
{
	dst.len = rle.len;
	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;
	for (int i = 0; i < rle.len; ++i)
	{
		//�����ͺ�ʴ,��������г̱������Ϣ
		*rle_y++ = rle.rle_y[i] + offset_y;
		*rle_x_end++ = rle.rle_x_end[i] + offset_x;
		*rle_x_start++ = rle.rle_x_start[i] + offset_x;
	}
	dst.len = rle_y - dst.rle_y;
}
//end


//erosion_circle start
int erosion_circle_row(const HRegion& rle, HRegion& dst, int start_x, int len, int dy){
	short* last_erosion_row_y = dst.rle_y;
	//�������̵��г�
	int rle_cnt = rle.len;
	const short* p_rle_start_x = rle.rle_x_start;
	const short* p_rle_y = rle.rle_y;
	const short* p_rle_end_x = rle.rle_x_end;

	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;

	for (int id = 0; id < rle_cnt; ++id)
	{
		//������Ϊwidth���г̱���
		if (p_rle_end_x[id] - p_rle_start_x[id] + 1 >= len)
		{
			//��ʴ������п�ʼ���н���,ԭ��Ϊ���ĵ�
			//rle_x_start =a.start()-b.start()
			//rle_x_len =a.len() - b.len() + 1
			//rle_x_end = rle_x_start+rle_x_len-1
			//a.len() = rle.rle_x_end[id] - rle.rle_x_start[id] + 1
			//b.len() = len
			*rle_x_start++ = rle.rle_x_start[id] - start_x;
			*rle_x_end++ = (rle.rle_x_start[id] - start_x) +(rle.rle_x_end[id] - rle.rle_x_start[id] + 1) - len;
			*rle_y++ = rle.rle_y[id] + dy;
		}
	}

	return int(rle_y - last_erosion_row_y);
}

void erosion_intersection(const HRegion& rle, ErosionDilationCircleParam m_ErosionCircleParam, HRegion& dst){
	dst.Image_Width = rle.Image_Width;
	dst.Image_Height = rle.Image_Height;

	//�����Ҫ�������Ĵ���,ʱ�临�Ӷ�Ϊ(log2^(n)),nΪ�߶�
	int step = 1;
	int rank = 1;
	while (step < m_ErosionCircleParam.count)
	{
		step <<= 1;
		++rank;
	}

	//������ʼ��
	HRegion *erosion_temp = new HRegion[rank];
	for (int i = 0; i < rank; ++i)
	{
		erosion_temp[i].Image_Width = rle.Image_Width;
		erosion_temp[i].Image_Height = rle.Image_Height;
	}

	//������ֵ,�Ż��ڴ�
	erosion_temp[0] = rle;

	//step1:��ǰ����ȡ���������һ��������Region
	for (int i = 1; i < rank; ++i)
	{
		erosion_temp[i].len = erosion_blocks(erosion_temp[i - 1], erosion_temp[i], m_ErosionCircleParam.count, i);
	}

	--rank;
	//step2:���г̵�ֵ�ƶ���ԭ���ֵ,ƫ�Ʋ����⴦��
	int offset_x = 0;
	int offset_y = 0;
	circle_translate(erosion_temp[rank], dst, offset_x, offset_y);
	delete[] erosion_temp;
}

void erosion_intersection2(const HRegion& rle, ErosionDilationCircleParam m_ErosionCircleParam, ErosionDilationCircleParam m_ErosionCircleParam1, HRegion& dst, HRegion& dst1){
	dst.Image_Width = rle.Image_Width;
	dst.Image_Height = rle.Image_Height;
	dst1.Image_Width = rle.Image_Width;
	dst1.Image_Height = rle.Image_Height;

	//�����Ҫ�������Ĵ���,ʱ�临�Ӷ�Ϊ(log2^(n)),nΪ�߶�
	int step = 1;
	int rank = 1;
	while (step < m_ErosionCircleParam.count)
	{
		step <<= 1;
		++rank;
	}

	//������ʼ��
	HRegion *erosion_temp = new HRegion[rank];
	for (int i = 0; i < rank; ++i)
	{
		erosion_temp[i].Image_Width = rle.Image_Width;
		erosion_temp[i].Image_Height = rle.Image_Height;
	}

	//������ֵ,�Ż��ڴ�
	erosion_temp[0] = rle;

	//step1:��ǰ����ȡ���������һ��������Region
	for (int i = 1; i < rank; ++i)
	{
		erosion_temp[i].len = erosion_blocks(erosion_temp[i - 1], erosion_temp[i], m_ErosionCircleParam.count, i);
	}

	--rank;

	//step2:���г̵�ֵ�ƶ���ԭ���ֵ,ƫ�Ʋ����⴦��
	int offset_x = 0;
	int offset_y = 0;
	circle_translate(erosion_temp[rank], dst, offset_x, offset_y);
	offset_x = 0;
	offset_y = m_ErosionCircleParam1.row - m_ErosionCircleParam.row;
	circle_translate(erosion_temp[rank], dst1, offset_x, offset_y);
	delete[] erosion_temp;
}

void circle_translate(const HRegion& rle, HRegion& dst, int offset_x, int offset_y)
{
	dst.len = rle.len;
	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;
	for (int i = 0; i < rle.len; ++i)
	{
		//��ȡ��ǰ�г̱���������Ϣ
		*rle_y++ = rle.rle_y[i] + offset_y;
		*rle_x_end++ = rle.rle_x_end[i] + offset_x;
		*rle_x_start++ = rle.rle_x_start[i] + offset_x;
	}
	dst.len = rle_y - dst.rle_y;
}
//end

//dilation_circle_start
int dilation_circle_row(const HRegion& rle, HRegion& dst, int start_x, int len, int dy){
	short* last_erosion_row_y = dst.rle_y;
	//�������̵��г�
	int rle_cnt = rle.len;
	const short* p_rle_start_x = rle.rle_x_start;
	const short* p_rle_y = rle.rle_y;
	const short* p_rle_end_x = rle.rle_x_end;

	short* rle_x_start = dst.rle_x_start;
	short* rle_x_end = dst.rle_x_end;
	short* rle_y = dst.rle_y;

	for (int id = 0; id < rle_cnt; ++id)
	{
		//���͹�����п�ʼ���н���,ԭ��Ϊ���ĵ�
		//rle_x_start =a.start() + b.start()
		//rle_x_len =a.len() + b.len() - 1
		//rle_x_end = rle_x_start+rle_x_len-1
		//a.len() = rle.rle_x_end[id] - rle.rle_x_start[id] + 1
		//b.len() = len
		*rle_x_start++ = rle.rle_x_start[id] + start_x;
		*rle_x_end++ = (rle.rle_x_start[id] + start_x) + (rle.rle_x_end[id] - rle.rle_x_start[id] + 1) + len - 1 - 1;
		*rle_y++ = rle.rle_y[id] + dy;
	}

	return int(rle_y - last_erosion_row_y);
}

void dilation_union1(const HRegion& rle, ErosionDilationCircleParam m_DilationCircleParam, HRegion& dst){
	dst.Image_Width = rle.Image_Width;
	dst.Image_Height = rle.Image_Height;

	//�����Ҫ�������Ĵ���,ʱ�临�Ӷ�Ϊ(log2^(n)),nΪ�߶�
	int step = 1;
	int rank = 1;
	while (step < m_DilationCircleParam.count)
	{
		step <<= 1;
		++rank;
	}

	//������ʼ��
	HRegion *dilation_temp = new HRegion[rank];
	for (int i = 0; i < rank; ++i)
	{
		dilation_temp[i].Image_Width = rle.Image_Width;
		dilation_temp[i].Image_Height = rle.Image_Height;
	}

	//������ֵ,�Ż��ڴ�
	dilation_temp[0] = rle;

	//step1:��ǰ����ȡ���������һ��������Region
	for (int i = 1; i < rank; ++i)
	{
		dilation_temp[i].len = dilation_blocks(dilation_temp[i - 1], dilation_temp[i], m_DilationCircleParam.count, i);
	}

	--rank;
	//step2:���г̵�ֵ�ƶ���ԭ���ֵ,ƫ�Ʋ����⴦��
	int offset_x = 0;
	int offset_y = 0;
	circle_translate(dilation_temp[rank], dst, offset_x, offset_y);
	delete[] dilation_temp;
}

void dilation_union2(const HRegion& rle, ErosionDilationCircleParam m_DilationCircleParam, ErosionDilationCircleParam m_DilationCircleParam1, HRegion& dst, HRegion& dst1){
	dst.Image_Width = rle.Image_Width;
	dst.Image_Height = rle.Image_Height;
	dst1.Image_Width = rle.Image_Width;
	dst1.Image_Height = rle.Image_Height;

	//�����Ҫ�������Ĵ���,ʱ�临�Ӷ�Ϊ(log2^(n)),nΪ�߶�
	int step = 1;
	int rank = 1;
	while (step < m_DilationCircleParam.count)
	{
		step <<= 1;
		++rank;
	}

	//������ʼ��
	HRegion *dilation_temp = new HRegion[rank];
	for (int i = 0; i < rank; ++i)
	{
		dilation_temp[i].Image_Width = rle.Image_Width;
		dilation_temp[i].Image_Height = rle.Image_Height;
	}

	//������ֵ,�Ż��ڴ�
	dilation_temp[0] = rle;

	//step1:��ǰ����ȡ���������һ��������Region
	for (int i = 1; i < rank; ++i)
	{
		dilation_temp[i].len = dilation_blocks(dilation_temp[i - 1], dilation_temp[i], m_DilationCircleParam.count, i);
	}

	--rank;
	//step2:���г̵�ֵ�ƶ���ԭ���ֵ,ƫ�Ʋ����⴦��
	int offset_x = 0;
	int offset_y = 0;
	circle_translate(dilation_temp[rank], dst, offset_x, offset_y);
	offset_x = 0;
	offset_y = m_DilationCircleParam1.row - m_DilationCircleParam.row;
	circle_translate(dilation_temp[rank], dst1, offset_x, offset_y);
	delete[] dilation_temp;
}

//end

//union start
void union_two_row(const short* rle_x, const short* rle_end_x, const short* region_x, const short* region_end_x, short** row_rle_x, short** row_rle_y, short** row_rle_end_x,
	short row, int startA  , int endA, int startB, int endB){
	//��ȡ��һ������Region��Ϊ�յ�ֵΪ��ʼֵ,�ڶ�������Region��Ϊ�յ�ֵΪ����ֵ
	int i = startA;
	int j = startB;
	/*ָ��*/
	short* p_row_rle_x = *row_rle_x;
	short* p_row_rle_y = *row_rle_y;
	short* p_row_rle_end_x = *row_rle_end_x;

	//step1:������ʼ��
	//�Աȵ�ǰ����Region���п�ʼֵ,��ý�СRegion�ĵ�ǰ�п�ʼֵ���н���ֵ
	int last_rle_x;
	int last_rle_end_x;
	if (rle_x[i] < region_x[j]){
		last_rle_x = rle_x[i];
		last_rle_end_x = rle_end_x[i] + 1;//��1,124~130��131~140����ȡ����
		i++;
	}
	else{
		last_rle_x = region_x[j];
		last_rle_end_x = region_end_x[j] + 1;//��1,124~130��131~140����ȡ����
		j++;
	}

	//step2:��������Region�ĵ�ǰ������Union����Ϣ
	//rle��Region�ĵ�ǰ�еĽ���ֵ
	while (i != endA && j != endB){
		//�Աȵ�ǰ����Region���п�ʼֵ,��ý�СRegion�ĵ�ǰ�п�ʼֵ���н���ֵ
		int cur_rle_x;
		int cur_rle_end_x;
		if (rle_x[i]<region_x[j]){
			cur_rle_x = rle_x[i];
			cur_rle_end_x = rle_end_x[i] + 1;
			i++;
		}
		else{
			cur_rle_x = region_x[j];
			cur_rle_end_x = region_end_x[j] + 1;
			j++;
		}

		int col_start = cur_rle_x > last_rle_x ? cur_rle_x : last_rle_x;//�Ա������г̱�����п�ʼֵ,��������г̱�����п�ʼֵ�����ֵ
		int col_end = cur_rle_end_x > last_rle_end_x ? last_rle_end_x : cur_rle_end_x;//�Ա������г̱�����н���ֵ,��������г̱�����н���ֵ����Сֵ
		if (col_end >= col_start){//��ǰ�н���ֵ��Сֵ���ڵ��ڵ�ǰ�п�ʼֵ���ֵ,��Ϊ��
			int start_ = cur_rle_x > last_rle_x ? last_rle_x : cur_rle_x;//�Ա������г̱�����п�ʼֵ,��������г̱�����п�ʼֵ����Сֵ
			int end_ = cur_rle_end_x > last_rle_end_x ? cur_rle_end_x : last_rle_end_x;//�Ա������г̱�����н���ֵ,��������г̱�����н���ֵ�����ֵ
			last_rle_x = start_;
			last_rle_end_x = end_;
		}
		else{//��ǰ�н���ֵ��СֵС�ڵ�ǰ�п�ʼֵ���ֵ,Ϊ��,��д���������ֵ
			*p_row_rle_x++ = last_rle_x;
			*p_row_rle_y++ = row;
			*p_row_rle_end_x++ = last_rle_end_x - 1;
			last_rle_x = cur_rle_x;
			last_rle_end_x = cur_rle_end_x;
		}
	}

	int cur_rle_x;
	int cur_rle_end_x;
	while (i != endA){
		//���rle�ĵ�ǰ�п�ʼֵ���н���ֵ
		cur_rle_x = rle_x[i];
		cur_rle_end_x = rle_end_x[i] + 1;
		int col_start = cur_rle_x > last_rle_x ? cur_rle_x : last_rle_x;//�Ա������г̱�����п�ʼֵ,��������г̱�����п�ʼֵ�����ֵ
		int col_end = cur_rle_end_x > last_rle_end_x ? last_rle_end_x : cur_rle_end_x;//�Ա������г̱�����н���ֵ,��������г̱�����н���ֵ����Сֵ
		if (col_end >= col_start){//��ǰ�н���ֵ��Сֵ���ڵ��ڵ�ǰ�п�ʼֵ���ֵ,��Ϊ��
			int start_ = cur_rle_x > last_rle_x ? last_rle_x : cur_rle_x;//�Ա������г̱�����п�ʼֵ,��������г̱�����п�ʼֵ����Сֵ
			int end_ = cur_rle_end_x > last_rle_end_x ? cur_rle_end_x : last_rle_end_x;//�Ա������г̱�����н���ֵ,��������г̱�����н���ֵ�����ֵ
			last_rle_x = start_;
			last_rle_end_x = end_;
			i++;
		}
		else{
			break;
		}
	}
	while (j != endB){
		//���Region�ĵ�ǰ�п�ʼֵ���н���ֵ
		cur_rle_x = region_x[j];
		cur_rle_end_x = region_end_x[j] + 1;
		int col_start = cur_rle_x > last_rle_x ? cur_rle_x : last_rle_x;//�Ա������г̱�����п�ʼֵ,��������г̱�����п�ʼֵ�����ֵ
		int col_end = cur_rle_end_x > last_rle_end_x ? last_rle_end_x : cur_rle_end_x;//�Ա������г̱�����н���ֵ,��������г̱�����н���ֵ����Сֵ
		if (col_end >= col_start){//��ǰ�н���ֵ��Сֵ���ڵ��ڵ�ǰ�п�ʼֵ���ֵ,��Ϊ��
			int start_ = cur_rle_x > last_rle_x ? last_rle_x : cur_rle_x;//�Ա������г̱�����п�ʼֵ,��������г̱�����п�ʼֵ����Сֵ
			int end_ = cur_rle_end_x > last_rle_end_x ? cur_rle_end_x : last_rle_end_x;//�Ա������г̱�����н���ֵ,��������г̱�����н���ֵ�����ֵ
			last_rle_x = start_;
			last_rle_end_x = end_;
			j++;
		}
		else{
			break;
		}
	}
	*p_row_rle_x++ = last_rle_x;
	*p_row_rle_y++ = row;
	*p_row_rle_end_x++ = last_rle_end_x - 1;

	//û�б�����rle_x��ǰ�еĽ���ֵ
	while (i != endA){
		last_rle_x = rle_x[i];
		last_rle_end_x = rle_end_x[i];
		*p_row_rle_x++ = last_rle_x;
		*p_row_rle_y++ = row;
		*p_row_rle_end_x++ = last_rle_end_x;
		i++;
	}

	//û�б�����Region��ǰ�еĽ���ֵ
	while (j != endB){
		last_rle_x = region_x[j];
		last_rle_end_x = region_end_x[j];
		*p_row_rle_x++ = last_rle_x;
		*p_row_rle_y++ = row;
		*p_row_rle_end_x++ = last_rle_end_x;
		j++;
	}

	*row_rle_x = p_row_rle_x;
	*row_rle_y = p_row_rle_y;
	*row_rle_end_x = p_row_rle_end_x;
}
//end

//intersect start 
void intersect_two_row(const short* rle_x, const short* rle_end_x, const short* region_x, const short* region_end_x, short** row_rle_x, short** row_rle_y, short** row_rle_end_x,
	short row, int startA, int endA, int startB, int endB)
{
	//�Ա�����Region��ͬʱ���ڵ�ֵ,��Ϊ����
	int i = startA;
	int j = startB;
	short* p_row_rle_x = *row_rle_x;
	short* p_row_rle_y = *row_rle_y;
	short* p_row_rle_end_x = *row_rle_end_x;

	//step1:������ʼ��
	//������Region�ϲ���һ��,��������,��õ�һ����С���н�����ֵ
	int xe = rle_x[i] <= region_x[j] ? (rle_end_x[i++] + 1) : (region_end_x[j++] + 1);
	int last_rle_x;
	int last_rle_end_x;
	//�����һ����С���п�ʼֵ���н���ֵ
	if (i == endA){
		last_rle_x = region_x[j];
		last_rle_end_x = region_end_x[j];
	}
	else if (j == endB){
		last_rle_x = rle_x[i];
		last_rle_end_x = rle_end_x[i];
	}
	else{
		last_rle_x = rle_x[i] <= region_x[j] ? rle_x[i] : region_x[j];
		last_rle_end_x = rle_x[i] <= region_x[j] ? rle_end_x[i] : region_end_x[j];
	}

	//step2:��������Region�����ڵĽ���
	while (i != endA && j != endB){
		//��õ�ǰ�����ĳ���
		int len = intersection(xe, last_rle_x, last_rle_end_x - last_rle_x + 1);
		if (len > 0){
			*p_row_rle_x++ = last_rle_x;//��ǰ���п�ʼֵ
			*p_row_rle_y++ = row;
			*p_row_rle_end_x++ = last_rle_x + len - 1;//��ǰ���п�ʼֵ+��ǰ�����ĳ���
		}
		//�����һ����С���п�ʼֵ���н���ֵ
		int a = rle_x[i] <= region_x[j] ? (i++) : (j++);
		if (i == endA){
			last_rle_x = region_x[j];
			last_rle_end_x = region_end_x[j];
			break;
		}
		else if (j == endB){
			last_rle_x = rle_x[i];
			last_rle_end_x = rle_end_x[i];
			break;
		}
		else{
			last_rle_x = rle_x[i] <= region_x[j] ? rle_x[i] : region_x[j];
			last_rle_end_x = rle_x[i] <= region_x[j] ? rle_end_x[i] : region_end_x[j];
		}
	}

	//û�б�����rle_x��ǰ�еĽ���ֵ
	while (i != endA){
		//��õ�ǰ�����ĳ���
		int len = intersection(xe, last_rle_x, last_rle_end_x - last_rle_x + 1);
		if (len > 0){
			*p_row_rle_x++ = last_rle_x;
			*p_row_rle_y++ = row;
			*p_row_rle_end_x++ = last_rle_x + len - 1;
		}
		//�����һ����С���п�ʼֵ���н���ֵ
		i++;
		last_rle_x = rle_x[i];
		last_rle_end_x = rle_end_x[i];
	}

	//û�б�����region_x��ǰ�еĽ���ֵ
	while (j != endB){
		//��õ�ǰ�����ĳ���
		int len = intersection(xe, last_rle_x, last_rle_end_x - last_rle_x + 1);
		if (len > 0){
			*p_row_rle_x++ = last_rle_x;
			*p_row_rle_y++ = row;
			*p_row_rle_end_x++ = last_rle_x + len - 1;
		}
		//�����һ����С���п�ʼֵ���н���ֵ
		j++;
		last_rle_x = region_x[j];
		last_rle_end_x = region_end_x[j];
	}

	*row_rle_x = p_row_rle_x;
	*row_rle_y = p_row_rle_y;
	*row_rle_end_x = p_row_rle_end_x;
}

int intersection(int & xe, int const & r_start, int const & r_len){
	int xs = r_start;
	int len = r_len;
	int xt = r_start + r_len;

	if (xe <= xs) {//��ǰ�п�ʼֵ������С���н���ֵ
		xe = r_start + r_len;//ʹ�õ�ǰ�н���ֵ�޸���С�н���ֵ
		return -1;
	}
	if (xe < xt) {//��ǰ�п�ʼֵС����С���н���ֵ���ҵ�ǰ�н���ֵ������С����ֵ
		len = xe - r_start;//��õ�ǰ�����ĳ���
		xe = r_start + r_len;//ʹ�õ�ǰ�н���ֵ�޸���С�н���ֵ
	}
	return len;
}
//end

//difference start
void difference_two_row(const short* rle_x, const short* rle_end_x, const short* region_x, const short* region_end_x, short** row_rle_x, short** row_rle_y, short** row_rle_end_x,
	short row, int startA, int endA, int startB, int endB)
{
	//Ѱ��rle���ڵ��г̱����Region�����ڵ��г̱���
	int i = startA;
	int j = startB;
	/*ָ��*/
	short* p_row_rle_x = *row_rle_x;
	short* p_row_rle_y = *row_rle_y;
	short* p_row_rle_end_x = *row_rle_end_x;

	while (i != endA && j != endB){
		//��ֵ
		int d_rle_x = rle_x[i];
		int d_rle_end_x = rle_end_x[i];
		i++;
		do{//�����ֿ���
			if (d_rle_end_x <= region_end_x[j])//d_rle_xΪregion_end_x[j]��һ�ֿ���
			{
				if (region_x[j] > d_rle_x){//Region���п�ʼֵ���ڵ�ǰ����,d_rle_xΪrle_x[i]���ֿ���
					*p_row_rle_x++ = d_rle_x;
					*p_row_rle_y++ = row;
					int len = min(d_rle_end_x - d_rle_x, region_x[j] - d_rle_x - 1);//�ж���С����
					*p_row_rle_end_x++ = d_rle_x + len;
				}

				//�����һ��������ֵ
				if (i != endA){
					d_rle_x = rle_x[i];
					d_rle_end_x = rle_end_x[i];
					i++;
				}
				else{
					d_rle_x = region_end_x[j] + 1;
					break;
				}
			}
			else
			{
				if (d_rle_x < region_x[j]){//Region���п�ʼֵ���ڵ�ǰ����,d_rle_xΪrle_x[i]һ�ֿ���
					*p_row_rle_x++ = d_rle_x;
					*p_row_rle_y++ = row;
					int len = region_x[j] - d_rle_x - 1;
					*p_row_rle_end_x++ = d_rle_x + len;
				}

				//�����һ��������ֵ
				if (region_end_x[j] >= d_rle_x){
					d_rle_x = region_end_x[j] + 1;
				}
				++j;
			}
		} while (j != endB);

		//��ȡһ���������ֵ
		if (d_rle_end_x - d_rle_x >= 0){
			*p_row_rle_x++ = d_rle_x;
			*p_row_rle_y++ = row;
			*p_row_rle_end_x++ = d_rle_end_x;
			d_rle_x = rle_x[i];
			d_rle_end_x = rle_end_x[i];
		}
		else{
			break;
		}

		//rleû����������ֵ,ȡʣ�µ�����ֵ
		while (1){
			if (i == endA){
				break;
			}
			*p_row_rle_x++ = d_rle_x;
			*p_row_rle_y++ = row;
			*p_row_rle_end_x++ = d_rle_end_x;
			i++;
			d_rle_x = rle_x[i];
			d_rle_end_x = rle_end_x[i];
		}
	}

	*row_rle_x = p_row_rle_x;
	*row_rle_y = p_row_rle_y;
	*row_rle_end_x = p_row_rle_end_x;
}
//end



//connection start 
//offset = 0 -> 4��ͨ
//offset = 1 -> 8��ͨ
int labeling(const short* rle_x, const short* rle_end_x, const short* rle_y, int len,vector<int>& runLabels, int offset){
	int idxLabel = 1;
	int curRowIdx = 0;
	int firstRunOnCur = 0;
	int firstRunOnPre = 0;
	int lastRunOnPre = -1;
	int NumberOfRuns = len;

	//����ŵı�Ǻ͵ȼ۶��б������,��P�ĳɵȼ۶�����С��ֵ
	int *P = new int[NumberOfRuns];
	//first label is for background pixels
	P[0] = 0;

	for (int i = 0; i < NumberOfRuns;i++){

		if (rle_y[i] != curRowIdx){
			curRowIdx = rle_y[i];
			firstRunOnPre = firstRunOnCur;
			lastRunOnPre = i - 1;
			firstRunOnCur = i;
		}

		for (int j = firstRunOnPre; j <= lastRunOnPre; j++){
			if (rle_x[i] <= rle_end_x[j] + offset && rle_end_x[i] >= rle_x[j] - offset && rle_y[i] == rle_y[j] + 1)
			{
				if (runLabels[i] == 0) // û�б���Ź�
					runLabels[i] = runLabels[j];
				else if (runLabels[i] != runLabels[j])// �Ѿ������      
					runLabels[i] = set_union(P, runLabels[i], runLabels[j]);
			}
		}
		if (runLabels[i] == 0) // û����ǰһ�е��κ�run�غ�
		{
			P[idxLabel] = idxLabel;
			runLabels[i] = idxLabel++;
		}
	}

	//analysis ��������
	int nLabels = flattenL(P, idxLabel);

	for (int i = 0; i < runLabels.size(); ++i)
	{
		//int & l = runLabels.at(i);
		int l = runLabels.at(i);
		runLabels[i] = P[l];
		//l = P[l];
	}

	delete[]  P;
	return nLabels;
}

//Find the root of the tree of node i
template<typename LabelT>
inline static
LabelT findRoot(const LabelT *P, LabelT i){
	LabelT root = i;
	while (P[root] < root){
		root = P[root];
	}
	return root;
}

//Make all nodes in the path of node i point to root
template<typename LabelT>
inline static
void setRoot(LabelT *P, LabelT i, LabelT root){
	while (P[i] < i){
		LabelT j = P[i];
		P[i] = root;
		i = j;
	}
	P[i] = root;
}

//Find the root of the tree of the node i and compress the path in the process
template<typename LabelT>
inline static
LabelT find(LabelT *P, LabelT i){
	LabelT root = findRoot(P, i);
	setRoot(P, i, root);
	return root;
}

//unite the two trees containing nodes i and j and return the new root
template<typename LabelT>
inline static
LabelT set_union(LabelT *P, LabelT i, LabelT j){
	LabelT root = findRoot(P, i);
	if (i != j){
		LabelT rootj = findRoot(P, j);
		if (root > rootj){
			root = rootj;
		}
		setRoot(P, j, root);
	}
	setRoot(P, i, root);
	return root;
}

//Flatten the Union Find tree and relabel the components
template<typename LabelT>
inline static
LabelT flattenL(LabelT *P, LabelT length){
	LabelT k = 1;
	for (LabelT i = 1; i < length; ++i){
		if (P[i] < i){
			P[i] = P[P[i]];
		}
		else{
			P[i] = k; k = k + 1;
		}
	}
	return k - 1;
}
//end

//select_shape start 

//�鲢����
void Merge(int A[], int left, int mid, int right)// �ϲ��������ź��������A[left...mid]��A[mid+1...right]
{
	int len = right - left + 1;
	int *temp = new int[len];       // �����ռ�O(n)
	int index = 0;
	int i = left;                   // ǰһ�������ʼԪ��
	int j = mid + 1;                // ��һ�������ʼԪ��
	while (i <= mid && j <= right)
	{
		temp[index++] = A[i] <= A[j] ? A[i++] : A[j++];  // ���Ⱥű�֤�鲢������ȶ���
	}
	while (i <= mid)
	{
		temp[index++] = A[i++];
	}
	while (j <= right)
	{
		temp[index++] = A[j++];
	}
	for (int k = 0; k < len; k++)
	{
		A[left++] = temp[k];
	}
}

void MergeSortRecursion(int A[], int left, int right)    // �ݹ�ʵ�ֵĹ鲢����(�Զ�����)
{
	if (left == right)    // ������������г���Ϊ1ʱ���ݹ鿪ʼ���ݣ�����merge����
		return;
	int mid = (left + right) / 2;
	MergeSortRecursion(A, left, mid);
	MergeSortRecursion(A, mid + 1, right);
	Merge(A, left, mid, right);
}

void MergeSortIteration(int A[], int len)    // �ǵݹ�(����)ʵ�ֵĹ鲢����(�Ե�����)
{
	int left, mid, right;// ����������,ǰһ��ΪA[left...mid]����һ��������ΪA[mid+1...right]
	for (int i = 1; i < len; i *= 2)        // ������Ĵ�Сi��ʼΪ1��ÿ�ַ���
	{
		left = 0;
		while (left + i < len)              // ��һ�����������(��Ҫ�鲢)
		{
			mid = left + i - 1;
			right = mid + i < len ? mid + i : len - 1;// ��һ���������С���ܲ���
			Merge(A, left, mid, right);
			left = right + 1;               // ǰһ����������������ƶ�
		}
	}
}

int IsEqualInt(int number, int m_runLabels_count[], int len){
	for (int i = 0; i < len; i++){
		if (m_runLabels_count[i] == number){
			return i;
		}
	}
	return -1;
}
//end

