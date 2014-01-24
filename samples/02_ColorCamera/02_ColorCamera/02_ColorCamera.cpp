// 02_ColorCamera.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <iostream>

// 1. ヘッダーファイル
#include "util_pipeline.h"
#include <opencv2\opencv.hpp>

// 2. UtilPipelineから継承したクラス
class Pipeline: public UtilPipeline {
protected:

  // 3. 定数、変数の宣言 
  //static const int Width = 640;
  //static const int Height = 480;
  static const int Width = 1280;
  static const int Height = 720;

  PXCImage::ColorFormat colorFormat;

public:

  // 4. コンストラクタ
  Pipeline()
    : UtilPipeline()
    , colorFormat( PXCImage::COLOR_FORMAT_RGB32 )
  {
    // 必要なデータを有効にする
    EnableImage( colorFormat, Width, Height );
  }

  // 5. 新しいフレームの更新イベント
  virtual bool OnNewFrame()
  {
    try {
      // フレームを取得する
      auto colorFrame = QueryImage( PXCImage::IMAGE_TYPE_COLOR );

      // データを取得する
      cv::Mat colorImage( Height, Width, CV_8UC4 );
      getColorData( colorImage, colorFrame );

      // 画像を左右反転する
      //cv::flip( colorImage, colorImage, 1 );

      // 表示する
      cv::imshow( "Color Camera", colorImage );
    }
    catch ( std::exception& ex ) {
      std::cout << ex.what() << std::endl;
    }

    auto key = cv::waitKey( 10 );
    return key != 'q';
  }

  // 6. Colorデータを取得する
  void getColorData( cv::Mat& colorImage, PXCImage* colorFrame )
  {
    // Colorデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = colorFrame->AcquireAccess( PXCImage::ACCESS_READ, colorFormat, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // Colorデータを可視化する
    memcpy( colorImage.data, data.planes[0], data.pitches[0] * Height );

    // Colorデータを解放する
    colorFrame->ReleaseAccess( &data );
  }
};

// 7. main関数
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

