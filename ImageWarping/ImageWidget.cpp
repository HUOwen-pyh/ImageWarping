#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>

#include "WarpingMethod.h"

using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	ptr_image_ = new QImage();
	ptr_image_backup_ = new QImage();
	PointX.clear();
	PointY.clear();
	execAddPoints = false;
	isNewPointIn = false;
	numOfPointsOnImage = 0;
}


ImageWidget::~ImageWidget(void)
{
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect((width() - ptr_image_->width()) / 2,
		(height() - ptr_image_->height()) / 2,
		ptr_image_->width(),
		ptr_image_->height());

	painter.drawImage(rect, *ptr_image_);



	painter.setPen(QPen(Qt::red, 3));
	QPoint tmpP((width() - ptr_image_->width()) / 2, (height() - ptr_image_->height()) / 2);
	for (size_t i = 0; i < numOfPointsOnImage; i++) {
		painter.drawLine(PointX[i] + tmpP, PointY[i] + tmpP);
	}


	painter.end();

	update();
}

void ImageWidget::Open()
{
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
	}

	//ptr_image_->invertPixels(QImage::InvertRgb);
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	//*(ptr_image_) = ptr_image_->rgbSwapped();
	cout << "image size: " << ptr_image_->width() << ' ' << ptr_image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	ptr_image_->save(filename);
}

void ImageWidget::AddPointPairs() {
	execAddPoints = true;
}

void ImageWidget::RBFMethod() {

	if (execAddPoints && numOfPointsOnImage > 0) {

		MatrixXf ma(numOfPointsOnImage + 3, 2);
		RBFInterpolation::alpha(ma, PointX, PointY, numOfPointsOnImage);


		QImage image_tmp(*(ptr_image_));
		std::vector<std::vector<bool>> PointColored(ptr_image_->width(), std::vector<bool>(ptr_image_->height(), false));
		for (int i = 0; i < ptr_image_->width(); i++) {

			for (int j = 0; j < ptr_image_->height(); j++) {

				int tx;
				int ty;
				RBFInterpolation::f(ma, tx, ty, i, j, PointX, numOfPointsOnImage);
				int txArr[2] = { tx,tx + 1 };
				int tyArr[2] = { ty,ty + 1 };

				for (int idxx = 0; idxx < 2; idxx++) {
					for (int idxy = 0; idxy < 2; idxy++) {
						if (txArr[idxx] >= 0 &&
							txArr[idxx] < ptr_image_->width() &&
							tyArr[idxy] >= 0 &&
							tyArr[idxy] < ptr_image_->height())
						{

							ptr_image_->setPixel(txArr[idxx], tyArr[idxy], image_tmp.pixel(i, j));
							PointColored[txArr[idxx]][tyArr[idxy]] = true;
						}
					}
				}

			}
		}

		for (int i = 0; i < ptr_image_->width(); i++) {

			for (int j = 0; j < ptr_image_->height(); j++) {

				if (!PointColored[i][j]) {

					ptr_image_->setPixel(i, j, qRgb(255, 255, 255));
				}
			}
		}


		ClearPointXY();
	}

	update();
}

void ImageWidget::IDWMethod() {

	if (execAddPoints && numOfPointsOnImage > 0) {
		std::vector<std::vector<float>> diffVecX1(numOfPointsOnImage, std::vector<float>(numOfPointsOnImage, 0));
		std::vector<std::vector<float>> diffVecX2(numOfPointsOnImage, std::vector<float>(numOfPointsOnImage, 0));
		std::vector<std::vector<float>> diffVecY1(numOfPointsOnImage, std::vector<float>(numOfPointsOnImage, 0));
		std::vector<std::vector<float>> diffVecY2(numOfPointsOnImage, std::vector<float>(numOfPointsOnImage, 0));
		for (int i = 0; i < numOfPointsOnImage; i++) {
			for (int j = 0; j < numOfPointsOnImage; j++) {
				diffVecX1[i][j] = PointX[i].x() - PointX[j].x();
				diffVecX2[i][j] = PointX[i].y() - PointX[j].y();
				diffVecY1[i][j] = PointY[i].x() - PointY[j].x();
				diffVecY2[i][j] = PointY[i].y() - PointY[j].y();
			}
		}

		std::vector<std::vector<float>> Sigma(numOfPointsOnImage, std::vector<float>(numOfPointsOnImage, 0));
		for (int i = 0; i < numOfPointsOnImage; i++) {
			for (int j = 0; j < numOfPointsOnImage; j++) {
				if (i == j) {
					Sigma[i][j] = 0;
				}
				else if (i > j) {
					Sigma[i][j] = Sigma[j][i];
				}
				else {
					float dis = RadialFunction::R(PointX[i], PointX[j]);
					Sigma[i][j] = 1 / dis;
				}
			}
		}

		std::vector<MatrixXf*> MatrixVec(numOfPointsOnImage);
		std::vector<MatrixXf*> MatrixX(numOfPointsOnImage);
		std::vector<MatrixXf*> MatrixY(numOfPointsOnImage);
		for (int i = 0; i < numOfPointsOnImage; i++) {
			MatrixXf* ma = new MatrixXf(2, 2);
			MatrixXf* mx = new MatrixXf(2, 1);
			MatrixXf* my = new MatrixXf(2, 1);
			IDWInterpolation::D(ma, Sigma, diffVecX1, diffVecX2, diffVecY1, diffVecY2, i);
			(*mx)(0, 0) = (float)PointX[i].x();
			(*mx)(1, 0) = (float)PointX[i].y();
			(*my)(0, 0) = (float)PointY[i].x();
			(*my)(1, 0) = (float)PointY[i].y();
			MatrixVec[i] = ma;
			MatrixX[i] = mx;
			MatrixY[i] = my;
		}

		std::vector<std::vector<bool>> PointColored(ptr_image_->width(), std::vector<bool>(ptr_image_->height(), false));

		QImage image_tmp(*(ptr_image_));
		for (int i = 0; i < ptr_image_->width(); i++) {

			for (int j = 0; j < ptr_image_->height(); j++) {

				MatrixXf cPoint(2, 1);
				cPoint(0, 0) = (float)i;
				cPoint(1, 0) = (float)j;
				MatrixXf target(2, 1);
				target(0, 0) = (float)0;
				target(1, 0) = (float)0;
				for (int k = 0; k < numOfPointsOnImage; k++) {
					QPoint tmpPoint(i, j);
					MatrixXf AfterProduct(2, 1);
					MatrixXf AfterDiff(2, 1);
					MatrixXf fk(2, 1);
					AfterDiff = (cPoint - (*MatrixX[k]));
					AfterProduct = (*MatrixVec[k]) * AfterDiff;
					fk = (*MatrixY[k]) + AfterProduct;
					target += IDWInterpolation::omega(k, tmpPoint, PointX) * fk;

				}



				int tx[2] = { (int)target(0, 0),(int)target(0, 0) + 1 };
				int ty[2] = { (int)target(1, 0),(int)target(1, 0) + 1 };
				for (int idxx = 0; idxx < 2; idxx++) {
					for (int idxy = 0; idxy < 2; idxy++) {
						if (tx[idxx] >= 0 &&
							tx[idxx] < ptr_image_->width() &&
							ty[idxy] >= 0 &&
							ty[idxy] < ptr_image_->height())
						{

							ptr_image_->setPixel(tx[idxx], ty[idxy], image_tmp.pixel(i, j));
							PointColored[tx[idxx]][ty[idxy]] = true;
						}
					}
				}
			}
		}

		for (int i = 0; i < ptr_image_->width(); i++) {

			for (int j = 0; j < ptr_image_->height(); j++) {

				if (!PointColored[i][j]) {

					ptr_image_->setPixel(i, j, qRgb(255, 255, 255));
				}
			}
		}

		for (int i = 0; i < numOfPointsOnImage; i++) {
			delete MatrixVec[i];
			delete MatrixX[i];
			delete MatrixY[i];
			MatrixVec[i] = NULL;
			MatrixX[i] = NULL;
			MatrixY[i] = NULL;
		}
		ClearPointXY();
	}

	update();
}

void ImageWidget::Restore()
{
	if (execAddPoints) {
		ClearPointXY();
	}

	*(ptr_image_) = *(ptr_image_backup_);
	update();
}

void ImageWidget::mousePressEvent(QMouseEvent* event) {
	if (Qt::LeftButton == event->button() && execAddPoints) {
		QPoint start_point = event->pos();
		int start_point_x = start_point.x();
		int start_point_y = start_point.y();
		if (start_point_x >= (width() - ptr_image_->width()) / 2 &&
			start_point_x <= (width() + ptr_image_->width()) / 2 &&
			start_point_y >= (height() - ptr_image_->height()) / 2 &&
			start_point_y <= (height() + ptr_image_->height()) / 2)
		{

			start_point.setX(start_point_x - (width() - ptr_image_->width()) / 2);
			start_point.setY(start_point_y - (height() - ptr_image_->height()) / 2);
			PointX.push_back(start_point);
			PointY.push_back(start_point);
			numOfPointsOnImage++;
			isNewPointIn = true;
		}

	}

	update();
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event) {
	if (execAddPoints && isNewPointIn) {
		QPoint end_point = event->pos();
		int end_point_x = end_point.x();
		int end_point_y = end_point.y();
		if (end_point_x >= (width() - ptr_image_->width()) / 2 &&
			end_point_x <= (width() + ptr_image_->width()) / 2 &&
			end_point_y >= (height() - ptr_image_->height()) / 2 &&
			end_point_y <= (height() + ptr_image_->height()) / 2)
		{
			end_point.setX(end_point_x - (width() - ptr_image_->width()) / 2);
			end_point.setY(end_point_y - (height() - ptr_image_->height()) / 2);
			PointY.back() = end_point;
		}
		else {
			PointX.pop_back();
			PointY.pop_back();
			isNewPointIn = false;
			numOfPointsOnImage--;
		}

	}
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (execAddPoints && isNewPointIn) {
		isNewPointIn = false;
	}
}

void ImageWidget::ClearPointXY() {
	PointX.clear();
	PointY.clear();
	execAddPoints = false;
	numOfPointsOnImage = 0;
}