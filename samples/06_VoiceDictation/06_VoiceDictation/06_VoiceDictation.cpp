// 06_VoiceDictation.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <iostream>
#include <Windows.h>

#include "util_pipeline.h"

class Pipeline: public UtilPipeline {
public:

  // 1.コンストラクタ
  Pipeline(void)
    : UtilPipeline()
  {
    // 必要なデータを有効にする
    EnableVoiceRecognition();
  }

  // 2.音声エンジンのセットアップを行う
  virtual void  OnVoiceRecognitionSetup(PXCVoiceRecognition::ProfileInfo * finfo)
  {
    // 日本語の音声エンジンを探す
    auto voiceRecognition = QueryVoiceRecognition();
    for ( int i = 0; ; ++i ) {
      PXCVoiceRecognition::ProfileInfo pinfo = { 0 };
      auto ret = voiceRecognition->QueryProfile( i, &pinfo );
      if ( ret != PXC_STATUS_NO_ERROR ) {
        break;
      }

      if ( pinfo.language == PXCVoiceRecognition::ProfileInfo::LANGUAGE_JP_JAPANESE ) {
        *finfo = pinfo;
        std::cout << "日本語の音声エンジンを設定しました" << std::endl;
      }
    }

    std::cout << "音声認識を開始します" << std::endl;
  }

  // 3.音声認識されたテキストを取得する
  virtual void PXCAPI OnRecognized( PXCVoiceRecognition::Recognition *cmd ) {
    std::wcout << L"認識した文: " << cmd->dictation << std::endl;
  }
};

int _tmain(int argc, _TCHAR* argv[])
{
  try {
    // 4.wide-charactorを表示できるようにする
    std::locale::global(std::locale("japanese"));

    Pipeline pipeline;
    pipeline.LoopFrames();
  }
  catch ( std::exception& ex ) {
    std::cout << ex.what() << std::endl;
  }

  return 0;
}

