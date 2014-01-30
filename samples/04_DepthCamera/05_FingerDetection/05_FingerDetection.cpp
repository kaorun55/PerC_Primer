// 05_FingerDetection.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <Windows.h>
#include <iostream>

#include "util_pipeline.h"
#include <opencv2\opencv.hpp>

class Pipeline: public UtilPipeline {
protected:

  static const int Width = 640;
  static const int Height = 480;
  //static const int Width = 1280;
  //static const int Height = 720;

  static const int DEPTH_WIDTH = 320;
  static const int DEPTH_HEIGHT = 240;

  PXCImage::ColorFormat colorFormat;

public:

  // 1.コンストラクター
  Pipeline(void)
    : UtilPipeline()
  {
    // 必要なデータを有効にする
    EnableGesture();
    EnableImage(PXCImage::COLOR_FORMAT_RGB32, Width, Height);
  }

  // 2. 新しいフレームの更新イベント
  virtual bool OnNewFrame(void)
  {
    try {
      // フレームを取得する
      auto colorFrame = QueryImage( PXCImage::IMAGE_TYPE_COLOR );
      auto depthFrame = QueryImage( PXCImage::IMAGE_TYPE_DEPTH );
      auto gestureFrame = QueryGesture();

      // フレームデータを取得する
      cv::Mat colorImage( Height, Width, CV_8UC4 );
      cv::Mat depthImage;
      getColorData( colorImage, colorFrame );
      getDepthData( depthImage, depthFrame );
      getGestureData( colorImage, gestureFrame, colorFrame, depthFrame );

      // 表示
      cv::imshow( "Color Camera", colorImage );
      cv::imshow( "Depth Camera", depthImage );
    }
    catch ( std::exception& ex ) {
      std::cout << ex.what() << std::endl;
    }

    auto key = cv::waitKey( 10 );
    return key != 'q';
  }

  // Colorカメラの画像を取得する
  void getColorData( cv::Mat& colorImage, PXCImage* colorFrame )
  {
    // Colorデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = colorFrame->AcquireAccess( PXCImage::ACCESS_READ,
                                          PXCImage::COLOR_FORMAT_RGB32, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // RGBデータを取得する
    memcpy( colorImage.data, data.planes[0], data.pitches[0] * Height );

    // Colorデータを解放する
    colorFrame->ReleaseAccess( &data );
  }

  // Depthカメラのデータを取得する
  void getDepthData( cv::Mat& depthImage, PXCImage* depthFrame )
  {
    // Depthデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = depthFrame->AcquireAccess( PXCImage::ACCESS_READ,
                                          PXCImage::COLOR_FORMAT_RGB32, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // 32bitのRGB画像を作成する
    depthImage = cv::Mat( DEPTH_HEIGHT, DEPTH_WIDTH, CV_8UC4 );

    // Depthデータを可視化する
    memcpy( depthImage.data, data.planes[0], data.pitches[0] * DEPTH_HEIGHT );

    // Depthデータを解放する
    depthFrame->ReleaseAccess( &data );
  }

  // 3. ジェスチャーデータを取得する
  void getGestureData( cv::Mat& colorImage, PXCGesture* gestureFrame,
                       PXCImage* colorFrame, PXCImage* depthFrame )
  {
    getHandPosition( colorImage, gestureFrame, colorFrame, depthFrame,
                     PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY );
    getHandPosition( colorImage, gestureFrame, colorFrame, depthFrame,
                     PXCGesture::GeoNode::LABEL_BODY_HAND_SECONDARY );
  }

  // 4. 手の位置を取得する
  void getHandPosition( cv::Mat& colorImage, PXCGesture* gestureFrame, PXCImage* colorFrame,
                        PXCImage* depthFrame, PXCGesture::GeoNode::Label hand )
  {
    // 手の位置
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 0, 0 ), hand );

#if 1
    // 具体的な指の位置
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 255, 0 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_THUMB );
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 0, 255 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_INDEX );
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 255, 0 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_MIDDLE );
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 0, 255 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_RING );
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 255, 255 ),
      hand | PXCGesture::GeoNode::LABEL_FINGER_PINKY );
#else
    // 抽象的な指の位置
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 255, 0 ),
      hand | PXCGesture::GeoNode::LABEL_HAND_FINGERTIP );
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 0, 0, 255 ),
      hand | PXCGesture::GeoNode::LABEL_HAND_UPPER );
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 255, 0 ),
      hand | PXCGesture::GeoNode::LABEL_HAND_MIDDLE );
    getLabel( colorImage, gestureFrame, colorFrame, depthFrame, cv::Scalar( 255, 0, 255 ),
      hand | PXCGesture::GeoNode::LABEL_HAND_LOWER );
#endif
  }

  // 指の位置を取得する
  void getLabel( cv::Mat& colorImage, PXCGesture* gestureFrame, PXCImage* colorFrame,
                 PXCImage* depthFrame, cv::Scalar color, PXCGesture::GeoNode::Label label )
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
    PXCImage::ImageInfo colorInfo = { 0, 0, 0 };
    PXCImage::ImageInfo depthInfo = { 0, 0, 0 };
    colorFrame->QueryInfo( &colorInfo );
    depthFrame->QueryInfo( &depthInfo );
    auto x = nodeData.positionImage.x;
    auto y = nodeData.positionImage.y;
    MapXY( x, y, &depthData, &depthInfo, &colorInfo );

    // 画面上に位置に点を描画する
    cv::circle( colorImage, cv::Point( x, y ), 10, color, -1 );

    // Depthデータを解放する
    depthFrame->ReleaseAccess( &depthData );
  }

  // 3次元座標を2次元座標に変換する
  // http://software.intel.com/sites/landingpage/perceptual_computing/documentation/html/
  static void MapXY( float &x, float &y, PXCImage::ImageData *depthData,
                     PXCImage::ImageInfo *depthInfo, PXCImage::ImageInfo *colorInfo)
  {
    int index = (int)((((int)y) * depthInfo->width) + x) * 2;

    float* uvmap = (float*)depthData->planes[2];
    x = uvmap[index] * colorInfo->width;
    y = uvmap[index + 1] * colorInfo->height;

    // X座標を反転する
    //x = abs(x - rgbInfo->width);
  }
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

