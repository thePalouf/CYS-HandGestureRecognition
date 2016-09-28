#ifndef IMAGEPROCESSING_INSTANCE
#define IMAGEPROCESSING_INSTANCE

#include "singleton_factory.hpp"
#include "instance_factory.hpp"
#include "camera.h"
#include "detection.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/cpp/var_array.h"
#include "ppapi/cpp/var_array_buffer.h"
#include "ppapi/utility/completion_callback_factory.h"
#include "ppapi/utility/threading/simple_thread.h"

// The ImageProcInstance that stores 
class ImageProcessingInstance : public pp::Instance {
  public:
    explicit ImageProcessingInstance( PP_Instance instance ) 
      : pp::Instance(instance), callback_factory_(this), proc_thread_(this) {};
    virtual ~ImageProcessingInstance( ){ proc_thread_.Join(); };
    virtual void HandleMessage( const pp::Var& );
    virtual bool Init(uint32_t /*argc*/,
        const char * /*argn*/ [],
        const char * /*argv*/ []) {
      proc_thread_.Start();
      proc_thread_.message_loop().PostWork( 
        callback_factory_.NewCallback( &ImageProcessingInstance::Version ));
      return true;
    }

  private:
    bool run_simulation_;
    Camera camera;
    Detection detection;
    pp::CompletionCallbackFactory<ImageProcessingInstance> callback_factory_;
    pp::SimpleThread proc_thread_; // Thread for image processor 
    void SendStatus(const std::string& status); 
    void Version( int32_t ) {
	auto processorFactoryCamera = SingletonFactory<std::function<Camera()>>::getInstance();
	auto processorFactoryDetection = SingletonFactory<std::function<Detection()>>::getInstance();
      pp::VarDictionary msg;
      msg.Set( "Type", "version" );
      msg.Set( "Version", "Image Processor 0.1" );
      PostMessage( msg );
    }
    void Process(cv::Mat&);
    void ProcessWImg(cv::Mat&);
    void ProcessBinary(cv::Mat&);
    void ProcessBackground(cv::Mat&);
};

#endif

