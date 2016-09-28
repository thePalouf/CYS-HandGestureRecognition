#ifndef CAMERA_H
#define CAMERA_H
#include <opencv2/opencv.hpp>
//#include "opencv2/highgui/highgui.hpp"


class Camera
{
public:
    //Create a new camera object
    Camera();

    //Launch the camera module process
    cv::Mat execute(const cv::Mat & currentFrame);
    //Load background user into 3 Mat to apply a background subtraction later
    void loadImageBackground(const cv::Mat & imageBackgroundToLoad);
    //Get the current BGR image
    cv::Mat getBGRimage() const;
    void applyBGS(cv::Mat&,double);

private:
    cv::Mat                 m_imageWebcamBGR;   //input image for reference
    cv::Mat                 m_imageYCrCb;       //image being processed
    std::vector<cv::Mat>    m_backgroundVector; //vector of background YCrcb image
    cv::CascadeClassifier   m_faceClassifier;   //classifier for face detection
    cv::Mat m_maskMOG2;
    cv::Ptr<cv::BackgroundSubtractor> m_pMOG2;

	bool xmlLoaded;

    std::string getClassifier();
    //Load current webcam image into class
    void loadImageWebcam(const cv::Mat & imageWebcamToLoad);
    //Convert a BGR image into a YCrCb one
    void bgrToYcrcb();
    //Apply a background subtraction to keep only new objets entering the scene
    void backgroundSub(int sY, int sCr, int sCb);
    //Detect faces in current frame
    std::vector<cv::Rect> faceDetection();
    //Delete faces in current frame
    void faceRemoval(const std::vector<cv::Rect>& faces);
    //Apply a canny edge segmentation when face not detected
    cv::Mat cannyEdge(int alpha, int beta, int lowThreshold, int highThreshold, int kernelSize);
    //Extract skin from current image after all process
    cv::Mat skinExtraction(int sYb, int sYh, int sCrb, int sCrh, int sCbb, int sCbh) const;
    //Apply a morphology operator (opening -> erode then dilate) on current frame
    void morphologyOpening(cv::Mat & image, const cv::Mat & element, int nbErode, int nbDilate) const;
    //Apply a morphology operator (closing -> dilate then erode) on current frame
    void morphologyClosing(cv::Mat & image, const cv::Mat & element, int nbDilate,int nbErode) const;
};

#endif // CAMERA_H

