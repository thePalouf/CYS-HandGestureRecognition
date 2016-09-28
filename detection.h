#ifndef DETECTION_H
#define DETECTION_H
#include <opencv2/opencv.hpp>
#include "timer.h"

enum StateHand {e_nothing, //nothing detected
               e_gun, //2 fingers 80°<angle<100°
               e_pinch, //2 fingers angle<100°
               e_open, //4-5 fingers, center far or near (depending on arm presence or not)
               e_flat, //0-1-2 fingers ?, center far or near (depending on arm presence or not)
               e_close, //0-1 finger, center near or far (depending on arm presence or not)
               e_claw, //4-5 fingers, center medium
               e_pointing}; //pointing with one finger

class Detection
{
public:
    Detection();
    //execute detection on given image
    void execute(const cv::Mat & currentBinaryFrame);

    //calculate angle between start and end
    float calculateAngle(cv::Point start, cv::Point end, cv::Point farthest) const;

    //all getter from detection class
    StateHand getStateLeft() const;
    StateHand getStateRight() const;
    void setStateLeft(const StateHand & state);
    void setStateRight(const StateHand & state);

    std::vector<std::vector<cv::Point>> getVectorContours() const;
    std::vector<cv::Point2f> getVectorCenterMax() const;
    std::vector<cv::Point2f> getVectorCenterMin() const;
    std::vector<float> getVectorRadiusMax() const;
    std::vector<float> getVectorRadiusMin() const;
    std::vector<std::vector<int>> getVectorConvexIndex() const;
    std::vector<std::vector<cv::Vec4i>> getVectorConvexityDefects() const;
    std::vector<std::vector<cv::Point>> getVectorFingersPosition() const;
    std::vector<std::vector<cv::Point>> getVectorFingersDirection() const;


private:
    //image
    cv::Mat                                 m_blobs;            //binary image from camera module
    //vectors
    std::vector<std::vector<cv::Point>>     m_contours;         //contours from binary image
    std::vector<cv::Point2f>                m_centerMax;        //centers of maximum inscribed circles (at least 2 from the 2 hands)
    std::vector<cv::Point2f>                m_centerMin;        //centers of minimum englobing circles
    std::vector<float>                      m_radiusMax;        //radius from maximum inscribed circles
    std::vector<float>                      m_radiusMin;        //radius from minimum englobing circles
    std::vector<std::vector<cv::Point>>     m_convexHullPoints; //convex hull of contours (drawing)
    std::vector<std::vector<int>>           m_convexHullIndex;  //index points of convex hull from contours
    std::vector<std::vector<cv::Vec4i>>     m_convexityDefects; //convexity defects of contours/convex hull
    std::vector<std::vector<cv::Point>>     m_fingersPosition;  //fingertips location and farthest point in hull
    std::vector<std::vector<cv::Point>>     m_fingersDirection; //fingers direction

    //states
    StateHand m_stateLeft;
    StateHand m_stateRight;

    //timers
    Timer m_timerLeft;
    Timer m_timerRight;


    //change size of vector based on number of
    void resizeAll();
    void loadBinary(const cv::Mat & binary);
    void extractContours(cv::Mat & image);
    void contoursApproximation(int index);
    void findMaximumInscribebCircle(int index);
    void setRegionOfInterest();
    void findMinimumEnglobingCircle(int index);
    void findConvexHullIndex(int index);
    void findConvexHullPoints(int index);
    void extractFingers(int index, int K, int fwindow);
    void gestureRecognition();
    void updateStates(StateHand left, StateHand right);
    //clear all the vectors
    void clearAll();
};

#endif // DETECTION_H

