/*#include<iostream>
#include"CImg.h"

using namespace std;
using namespace cimg_library;

int main() {
	// ��������ת��ԭͼ
	CImg<unsigned char> src("1.bmp");
	float x = src._width;
	float y = src._height;
	
	//��ת37�ȣ���С0.5���Ľ��
	CImg<unsigned char> newimg = src.get_rotate(37.0, x / 2, y / 2, 0.5, 2, 1);
	newimg.display("��ת37�ȣ���С0.5���Ľ��");

	//��ת52��, �Ŵ�2���Ľ��
	CImg<unsigned char> newimg2 = src.get_rotate(52.0, x / 2, y / 2, 2, 2, 1);
	newimg2.display("��ת52��, �Ŵ�2���Ľ��");
	

	// ���������εȡ�
	CImg<unsigned char> src2("3.bmp");
	float xx = src2._width;
	float yy = src2._height;
	unsigned char black[] = { 0, 0, 0 };
	//���ƾ���
	src2.draw_rectangle(100, 100, 300, 400, black);
	//����Բ��
	src2.draw_circle(250, 550, 100, black);
	//����������
	src2.draw_triangle(500, 700, 500, 800, 700, 800, black);

	src2.display("��ҵ2");

	return 0;
}*/