/*#include<iostream>
#include"CImg.h"

using namespace std;
using namespace cimg_library;

int main() {
	// 读入需旋转的原图
	CImg<unsigned char> src("1.bmp");
	float x = src._width;
	float y = src._height;
	
	//旋转37度，缩小0.5倍的结果
	CImg<unsigned char> newimg = src.get_rotate(37.0, x / 2, y / 2, 0.5, 2, 1);
	newimg.display("旋转37度，缩小0.5倍的结果");

	//旋转52度, 放大2倍的结果
	CImg<unsigned char> newimg2 = src.get_rotate(52.0, x / 2, y / 2, 2, 2, 1);
	newimg2.display("旋转52度, 放大2倍的结果");
	

	// 绘制三角形等。
	CImg<unsigned char> src2("3.bmp");
	float xx = src2._width;
	float yy = src2._height;
	unsigned char black[] = { 0, 0, 0 };
	//绘制矩形
	src2.draw_rectangle(100, 100, 300, 400, black);
	//绘制圆形
	src2.draw_circle(250, 550, 100, black);
	//绘制三角形
	src2.draw_triangle(500, 700, 500, 800, 700, 800, black);

	src2.display("作业2");

	return 0;
}*/