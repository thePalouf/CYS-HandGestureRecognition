#include "imageprocessing_instance.h"
#include "singleton_factory.hpp"
#include <vector>
#include <thread>
#include <functional>

pp::Module* pp::CreateModule() 
{
  return new InstanceFactory<ImageProcessingInstance>();
}


void ImageProcessingInstance::SendStatus(const std::string& status) 
{
  pp::VarDictionary msg;
  msg.Set( "Type", "status" );
  msg.Set( "Message", status );
  //PostMessage( msg );
}


void ImageProcessingInstance::HandleMessage( const pp::Var& var_message ) 
{
  // Interface: receive a { cmd: ..., args... } dictionary  
  pp::VarDictionary var_dict( var_message );
  auto cmd = var_dict.Get( "cmd" ).AsString();
// Message is number of simulations to run
    auto width  = var_dict.Get("width").AsInt();
    auto height = var_dict.Get("height").AsInt();
    auto data   = pp::VarArrayBuffer( var_dict.Get("data") );

    // Convert data to CMat
    uint8_t* byteData = static_cast<uint8_t*>(data.Map());
    auto Img = cv::Mat(height, width, CV_8UC4, byteData );
  if ( cmd == "process" ) { //same image only state
	pp::VarDictionary msg;

	msg.Set( "Type", "completed" );
	msg.Set( "Data", data );
	PostMessage( msg );

	//Process(Img);
  } else if ( cmd == "background" ) { //load background in camera
    ProcessBackground(Img);
  } else if ( cmd == "askskin" ) { //return blobs image
    ProcessBinary(Img);
  }else if ( cmd == "askcontour" ) { //return contour image (convex etc)   
    ProcessWImg(Img);
  }else {
    // Disable simulation - background thread will see this at start of
    // next iteration and terminate early
    run_simulation_ = false;
  }
}


void ImageProcessingInstance::Process( cv::Mat & im) 
{
	cv::cvtColor(im,im,CV_RGBA2BGR);
	auto binary = camera.execute(im);
	detection.execute(binary);

	StateHand left = detection.getStateLeft();
    	StateHand right = detection.getStateRight();
	
	pp::VarDictionary msg;

	msg.Set( "Type", "completed" );
	msg.Set( "Left", left );
	msg.Set( "Right", right );
	PostMessage( msg );
}

void ImageProcessingInstance::ProcessWImg( cv::Mat & im) 
{
	cv::cvtColor(im,im,CV_RGBA2BGR);
	auto binary = camera.execute(im);
	detection.execute(binary);
	
	std::vector<std::vector<cv::Point>> vector_contour = detection.getVectorContours();
	std::vector<cv::Point2f> vector_center_max = detection.getVectorCenterMax();
	std::vector<float> vector_radius_max = detection.getVectorRadiusMax();

	std::vector<cv::Point2f> vector_center_min = detection.getVectorCenterMin();
	std::vector<float> vector_radius_min = detection.getVectorRadiusMin();
	std::vector<std::vector<int>> vector_hullInt = detection.getVectorConvexIndex();

	//convexity defects for all contours
	std::vector<std::vector<cv::Point>> vector_fingers = detection.getVectorFingersDirection();

	if (!vector_contour.empty()) {
		for (unsigned int i = 0; i < vector_contour.size(); i++) {
		    circle(im,vector_center_max[i],4,cv::Scalar(0,0,255));
		    circle(im,vector_center_min[i],4,cv::Scalar(255,0,0));
		}

		//affichage doigts
		for (unsigned int i = 0; i < vector_fingers.size(); i++) {
		    for (unsigned int j = 0; j < vector_fingers[i].size(); j+=3) {
			circle(im,vector_fingers[i][j],4,cv::Scalar(255,0,0));
			circle(im,vector_fingers[i][j+1],6,cv::Scalar(255,0,0));
			circle(im,vector_fingers[i][j+2],4,cv::Scalar(255,0,0));
			cv::Point p((vector_fingers[i][j].x+vector_fingers[i][j+2].x)/2,(vector_fingers[i][j].y+vector_fingers[i][j+2].y)/2);
			line(im,vector_fingers[i][j+1],vector_fingers[i][j+1]+cv::Point(vector_fingers[i][j+1].x-p.x,vector_fingers[i][j+1].y-p.y),cv::Scalar(255,0,255),2);
		    }

		}
	}
     	StateHand left = detection.getStateLeft();
    	StateHand right = detection.getStateRight();

	cv::cvtColor(im,im,CV_BGR2RGBA);
	auto nBytes = im.elemSize() * im.total();
	
	pp::VarDictionary msg;
	pp::VarArrayBuffer data(nBytes);
	uint8_t* copy = static_cast<uint8_t*>( data.Map());
	memcpy( copy, im.data, nBytes );

	msg.Set( "Type", "completed" );
	msg.Set( "Data", data );
	msg.Set( "Left", left );
	msg.Set( "Right", right );
	//msg.Set( "Hand", vector_center_max[0].x); 
	PostMessage( msg );
}

void ImageProcessingInstance::ProcessBinary(cv::Mat& im)
{
	cv::cvtColor(im,im,CV_RGBA2BGR);
	auto binary = camera.execute(im);

	cv::cvtColor(binary,binary,CV_GRAY2RGBA);
	auto nBytes = binary.elemSize() * binary.total();
	
	pp::VarDictionary msg;
	pp::VarArrayBuffer data(nBytes);
	uint8_t* copy = static_cast<uint8_t*>( data.Map());
	memcpy( copy, binary.data, nBytes );

	msg.Set( "Type", "completed" );
	msg.Set( "Data", data );
	PostMessage( msg );
}

void ImageProcessingInstance::ProcessBackground(cv::Mat& im)
{
	pp::VarDictionary msg;
	cv::cvtColor(im,im,CV_RGBA2BGR);
	//camera.applyBGS(im,-1);
	camera.loadImageBackground(im);
	msg.Set( "Type", "learning" );
	PostMessage( msg );
}
