// 05_FingerDetection.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <Windows.h>
#include <iostream>

#include "util_pipeline.h"
#include <opencv2\opencv.hpp>

class Pipeline: public UtilPipeline {

public:

  Pipeline(void)
    : UtilPipeline()
  {
    // 必要なデータを有効にする
    EnableGesture();
    EnableImage(PXCImage::COLOR_FORMAT_RGB32, Width, Height);
  }

  // 新しいフレーム
  virtual bool OnNewFrame(void)
  {
    try {
      // フレームを取得する
      auto colorFrame = QueryImage( PXCImage::IMAGE_TYPE_COLOR );
      auto depthFrame = QueryImage(PXCImage::IMAGE_TYPE_DEPTH);
      auto gestureFrame = QueryGesture();

      // 取得したデータを表示する
      cv::Mat colorImage( Height, Width, CV_8UC4 );
      cv::Mat depthImage( DEPTH_HEIGHT, DEPTH_WIDTH, CV_8U );
      showColorFrame( colorImage, colorFrame );
      showDepthFrame( depthImage, depthFrame );
      showGestureFrame( colorImage, gestureFrame, colorFrame, depthFrame );

      // 表示
      cv::imshow( "Intel Perceptual Computing SDK", colorImage );
    }
    catch ( std::exception& ex ) {
      std::cout << ex.what() << std::endl;
    }

    auto key = cv::waitKey( 10 );
    return key != 'q';
  }

  // Colorデータを表示する
  void showColorFrame( cv::Mat& colorImage, PXCImage* colorFrame )
  {
    // Colorデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = colorFrame->AcquireAccess( PXCImage::ACCESS_READ, PXCImage::COLOR_FORMAT_RGB32, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // RGBデータを取得する
    memcpy( colorImage.data, data.planes[0], data.pitches[0] * Height );

    // Colorデータを解放する
    colorFrame->ReleaseAccess( &data );
  }

  // Depthデータを表示する
  void showDepthFrame( cv::Mat& depthImage, PXCImage* depthFrame )
  {
    // Depthデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = depthFrame->AcquireAccess( PXCImage::ACCESS_READ, PXCImage::COLOR_FORMAT_DEPTH, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // Depthデータを可視化する
    ushort* srcDepth = (ushort*)data.planes[0];
    uchar* dstDepth = (uchar*)depthImage.data;
    for ( int i = 0; i < DEPTH_WIDTH * DEPTH_HEIGHT; ++i ) {
      if ( (150 <= srcDepth[i]) && (srcDepth[i] < 800) ) {
        dstDepth[i] = srcDepth[i] * 0xFF / 1000;
      }
      else {
        dstDepth[i] = 0xFF;
      }
    }

    // Depthデータを解放する
    depthFrame->ReleaseAccess( &data );
  }

  // ジェスチャーフレームを表示する
  void showGestureFrame( cv::Mat& colorImage, PXCGesture* gestureFrame, PXCImage* colorFrame, PXCImage* depthFrame )
  {
    showHand( colorImage, gestureFrame, colorFrame, depthFrame, PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY );
    showHand( colorImage, gestureFrame, colorFrame, depthFrame, PXCGesture::GeoNode::LABEL_BODY_HAND_SECONDARY );
  }

  // 手の位置を表示する
  void showHand( cv::Mat& colorImage, PXCGesture* gestureFrame, PXCImage* colorFrame, PXCImage* depthFrame, PXCGesture::GeoNode::Label hand )
  {
    // 手の位置
    showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 0, 0 ), hand );

    // 具体的な指の位置
    showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 255, 0 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_THUMB );
    showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 0, 255 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_INDEX );
    showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 255, 0 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_MIDDLE );
    showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 0, 255 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_RING );
    showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 255, 255 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_PINKY );

    // 抽象的な指の位置
    //showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 255, 0 ),
    //  hand | PXCGesture::GeoNode::LABEL_HAND_FINGERTIP );
    //showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 0, 255 ),
    //  hand | PXCGesture::GeoNode::LABEL_HAND_UPPER );
    //showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 255, 0 ),
    //  hand | PXCGesture::GeoNode::LABEL_HAND_MIDDLE );
    //showLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 0, 255 ),
    //  hand | PXCGesture::GeoNode::LABEL_HAND_LOWER );
  }

  // 指の位置を表示する
  void showLabel( cv::Mat& colorImage, PXCGesture* gestureFrame, PXCImage* colorFrame, PXCImage* depthFrame,
    cv::Scalar color, PXCGesture::GeoNode::Label label )
  {
    // ジェスチャーデータを取得する
    PXCGesture::GeoNode nodeData = { 0 };
    auto sts = gestureFrame->QueryNodeData( 0 , label, &nodeData );
    if ( sts < PXC_STATUS_NO_ERROR) {
      return;
    }

    // Depthデータを取得する
    PXCImage::ImageData depthData = { 0 };
    sts = depthFrame->AcquireAccess( PXCImage::ACCESS_READ, &depthData );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // X,Y座標を画面の座標に変換する
    PXCImage::ImageInfo rgbInfo = { 0, 0, 0 };
    PXCImage::ImageInfo depthInfo = { 0, 0, 0 };
    colorFrame->QueryInfo( &rgbInfo );
    depthFrame->QueryInfo( &depthInfo );
    auto x = nodeData.positionImage.x;
    auto y = nodeData.positionImage.y;
    MapXY( x, y, &depthData, &depthInfo, &rgbInfo );

    // 画面上に位置に点を描画する
    cv::circle( colorImage, cv::Point( x, y ), 5, color, -1 );

    // Depthデータを解放する
    depthFrame->ReleaseAccess( &depthData );
  }

  // 3次元座標を2次元座標に変換する
  static void MapXY(float &x, float &y, PXCImage::ImageData *data, PXCImage::ImageInfo *depthInfo, PXCImage::ImageInfo *rgbInfo) {
    if (data->planes[2]) {
      if (x>=0 && y>=0 &&  x<depthInfo->width && y<depthInfo->height) {
        int index=(int)(((int)y)*depthInfo->width+x);
        int index2=2*index;

        x=((float*)data->planes[2])[index2]*rgbInfo->width;
        y=((float*)data->planes[2])[index2+1]*rgbInfo->height;

        // X座標を反転する
        //x = abs(x - rgbInfo->width);
      }
    } else {
      if (depthInfo->width>0 && depthInfo->height>0) {
        x=x/depthInfo->width*rgbInfo->width;
        y=y/depthInfo->height*rgbInfo->height;
      }
    }
  }

protected:

  //static const int Width = 640;
  //static const int Height = 480;
  static const int Width = 1280;
  static const int Height = 720;

  static const int DEPTH_WIDTH = 320;
  static const int DEPTH_HEIGHT = 240;

  PXCImage::ColorFormat colorFormat;
};

int _tmain(int argc, _TCHAR* argv[])
{
  try {
    Pipeline pipeline;
    pipeline.LoopFrames();
  }
  catch ( std::exception& ex ) {
    std::cout << ex.what() << std::endl;
  }

  return 0;
}

