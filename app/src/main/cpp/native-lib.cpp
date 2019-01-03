#include <jni.h>
#include <string>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
    #define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"zjf",FORMAT,##__VA_ARGS__);
    #define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"zjf",FORMAT,##__VA_ARGS__);
};

/**
 *  1、创建接口对象

    2、设置混音器

    3、创建播放器（录音器）

    4、设置缓冲队列和回调函数

    5、设置播放状态

    6、启动回调函数

 */

SLObjectItf engineObject = NULL;
SLEngineItf engineEngine = NULL;

SLObjectItf outputMixObject = NULL;
SLEnvironmentalReverbItf outputMixEnvReb = NULL;
SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

SLObjectItf pcmPlayerObject = NULL;
SLPlayItf pcmPlayerPlay = NULL;

SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;


FILE *pcmFile;
void *buffer;
uint8_t *out_buffer;


int getPcmData(void **pcm){
    int size = 0;
    while (!feof(pcmFile)){
        size = fread(out_buffer,1,44100 * 2 * 2,pcmFile);
        if (out_buffer == NULL) {
            LOGD("READ ENG...");
            break;
        } else {
            LOGD("READING...");
        }
        *pcm = out_buffer;
        break;
    }
    return size;
}

void pcmBufferCallback(SLAndroidSimpleBufferQueueItf bf,void *context){
    int size = getPcmData(&buffer);
    if (buffer != NULL){
        (*pcmBufferQueue)->Enqueue(pcmBufferQueue,buffer,size);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_zjf_androidopenslaudio_MainActivity_palyPcm(JNIEnv *env, jobject instance,
                                                             jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    pcmFile = fopen(url,"r");
    if (pcmFile == NULL) {
        LOGE("打不开文件 url === %s",url);
        return;
    }

    out_buffer = (uint8_t *)malloc(44100 * 2 * 2);
    // 1、创建一个引擎：三部曲 slCreateEngine->Realize->GetInterface
    slCreateEngine(&engineObject,0,0,0,0,0);
    (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineEngine);


    // 2、设置混音器：创建混音器引擎->设置混音器属性
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mrep[1] = {SL_BOOLEAN_FALSE};

    (*engineEngine)->CreateOutputMix(engineEngine,&outputMixObject,1,mids,mrep);
    (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    (*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB,&outputMixEnvReb);

    (*outputMixEnvReb)->SetEnvironmentalReverbProperties(outputMixEnvReb,&reverbSettings);


    // 3、创建播放器
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};//一个队列
    SLDataFormat_PCM format_pcm = {//设置PCM播放时的属性
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource slDataSource ={&android_queue,&format_pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,outputMixObject};
    SLDataSink audioSink = {&outputMix,NULL};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine,&pcmPlayerObject,&slDataSource,&audioSink,1,ids,req);
    (*pcmPlayerObject)->Realize(pcmPlayerObject,SL_BOOLEAN_FALSE);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_PLAY,&pcmPlayerPlay);


    //4、设置缓冲队列和回调函数
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_BUFFERQUEUE,&pcmBufferQueue);
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue,pcmBufferCallback,NULL);


    //5、设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_PLAYING);

    pcmBufferCallback(pcmBufferQueue,NULL);


    env->ReleaseStringUTFChars(url_, url);
}