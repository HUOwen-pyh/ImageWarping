#pragma once

#include <Eigen/Dense>
#include <QtWidgets>
#include <iostream>

using namespace Eigen;

namespace RadialFunction {
	float R(QPoint const& p1, QPoint const& p2) {
		float x1 = (float)p1.x();
		float x2 = (float)p2.x();
		float y1 = (float)p1.y();
		float y2 = (float)p2.y();
		float dx = x1 - x2;
		float dy = y1 - y2;
		float res = dx * dx + dy * dy;
		return res;
	}
}

namespace IDWInterpolation {
	void D(MatrixXf* Ma,
		std::vector<std::vector<float>> const& Sigma,
		std::vector<std::vector<float>> const& diffVecX1,
		std::vector<std::vector<float>> const& diffVecX2,
		std::vector<std::vector<float>> const& diffVecY1,
		std::vector<std::vector<float>> const& diffVecY2,
		int i)
	{
		MatrixXf& ma = *Ma;
		size_t len = Sigma.size();
		if (len == 1) {
			ma(0, 0) = 1;
			ma(0, 1) = 0;
			ma(1, 0) = 0;
			ma(1, 1) = 1;
			return;
		}
		MatrixXf tmpA(2, len - 1);
		MatrixXf tmpB(len - 1, 2);
		MatrixXf tmpC1(len - 1, 1);
		MatrixXf tmpC2(len - 1, 1);
		int tmpidx = 0;
		for (size_t j = 0; j < len; j++) {
			if (j != i) {
				tmpA(0, tmpidx) = Sigma[i][j] * diffVecX1[j][i];
				tmpA(1, tmpidx) = Sigma[i][j] * diffVecX2[j][i];
				tmpB(tmpidx, 0) = diffVecX1[j][i];
				tmpB(tmpidx, 1) = diffVecX2[j][i];
				tmpC1(tmpidx, 0) = diffVecY1[j][i];
				tmpC2(tmpidx, 0) = diffVecY2[j][i];
				tmpidx++;
			}
		}

		MatrixXf A(2, 2);
		MatrixXf c1(2, 1);
		MatrixXf c2(2, 1);
		A = tmpA * tmpB;
		c1 = tmpA * tmpC1;
		c2 = tmpA * tmpC2;

		MatrixXf row1(2, 1);
		MatrixXf row2(2, 1);

		row1 = A.colPivHouseholderQr().solve(c1);
		row2 = A.colPivHouseholderQr().solve(c2);

		ma(0, 0) = row1(0, 0);
		ma(0, 1) = row1(1, 0);
		ma(1, 0) = row2(0, 0);
		ma(1, 1) = row2(1, 0);
		return;
	}

	float sigma(int i, QPoint const& p, std::vector<QPoint> const& PointX) {

		if (p.x() != PointX[i].x() || p.y() != PointX[i].y()) {
			float dis = RadialFunction::R(p, PointX[i]);
			float res = 1 / dis;
			return res;
		}

		return 0;
	}

	float omega(int i, QPoint const& p, std::vector<QPoint> const& PointX) {
		float si = IDWInterpolation::sigma(i, p, PointX);
		if (si != 0) {

			float sum = si;
			for (int j = 0; j < PointX.size(); j++) {

				if (j != i) {
					float addval = sigma(j, p, PointX);
					if (addval == 0) {
						return 0;
					}
					sum += addval;
				}
			}

			float res = si / sum;
			return res;
		}
		return 1;
	}
}

namespace RBFInterpolation {

	void alpha(MatrixXf& ma,
		std::vector<QPoint> const& PointX,
		std::vector<QPoint> const& PointY,
		size_t numOfPointsOnImage)
	{

		MatrixXf b(numOfPointsOnImage + 3, 2);
		for (size_t i = 0; i < numOfPointsOnImage; i++) {
			b(i, 0) = PointY[i].x();
			b(i, 1) = PointY[i].y();
		}
		for (size_t i = 0; i < 3; i++) {
			b(i + numOfPointsOnImage, 0) = 0;
			b(i + numOfPointsOnImage, 1) = 0;
		}

		MatrixXf A(numOfPointsOnImage + 3, numOfPointsOnImage + 3);
		for (size_t i = 0; i < numOfPointsOnImage; i++) {

			for (size_t j = 0; j < numOfPointsOnImage; j++) {

				if (i == j) {
					A(i, j) = 0;
				}
				else if (i > j) {
					A(i, j) = A(j, i);
				}
				else {
					A(i, j) = RadialFunction::R(PointX[i], PointX[j]);
				}
			}
		}

		for (size_t i = 0; i < numOfPointsOnImage; i++) {

			A(i, numOfPointsOnImage) = PointX[i].x();
			A(i, numOfPointsOnImage + 1) = PointX[i].y();
			A(i, numOfPointsOnImage + 2) = 1;
		}

		for (size_t i = 0; i < numOfPointsOnImage; i++) {

			A(numOfPointsOnImage, i) = PointX[i].x();
			A(numOfPointsOnImage + 1, i) = PointX[i].y();
			A(numOfPointsOnImage + 2, i) = 1;

		}

		for (size_t i = numOfPointsOnImage; i < numOfPointsOnImage + 3; i++) {
			for (size_t j = numOfPointsOnImage; j < numOfPointsOnImage + 3; j++) {

				A(i, j) = 0;
			}
		}

		ma = A.colPivHouseholderQr().solve(b);


		//log to tell the rank of A
		/*
		FullPivLU<MatrixXf> lu_decomp(A);
		auto rank = lu_decomp.rank();
		std::cout << rank << "\n";
		*/


		return;
	}

	void f(MatrixXf const& ma,
		int& tx, int& ty,
		int i, int j,
		std::vector<QPoint> const& PointX,
		size_t numOfPointsOnImage)
	{

		MatrixXf a(1, numOfPointsOnImage + 3);
		QPoint tmpP(i, j);
		for (size_t len = 0; len < numOfPointsOnImage; len++) {
			a(0, len) = RadialFunction::R(tmpP, PointX[len]);
		}
		a(0, numOfPointsOnImage) = i;
		a(0, numOfPointsOnImage + 1) = j;
		a(0, numOfPointsOnImage + 2) = 1;

		MatrixXf res = a * ma;
		tx = (int)res(0, 0);
		ty = (int)res(0, 1);
		return;
	}

}