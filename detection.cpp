#include "detection.h"
//#include "opencv2/highgui/highgui.hpp"


///used to compare easily in a vector
cv::Rect minRectangleContour(const std::vector<cv::Point>& c);
bool compareVectorX(const cv::Point& a,const cv::Point& b);
bool compareVectorY(const cv::Point& a,const cv::Point& b);
bool compareVectorArea(const std::vector<cv::Point>& a,const std::vector<cv::Point>& b);
bool compareAreaBelow(const std::vector<cv::Point>& a);



Detection::Detection() {
    setStateLeft(e_nothing);
    setStateRight(e_nothing);
    m_timerLeft.start();
    m_timerRight.start();
}

void Detection::execute(const cv::Mat & currentBinaryFrame){
    loadBinary(currentBinaryFrame);
cv::Mat contour_detection = cv::Mat::zeros(m_blobs.rows, m_blobs.cols, CV_8UC1);

    clearAll();

    /// Contours extraction
    cv::Mat copy_blobs;
    m_blobs.copyTo(copy_blobs); //copie de l'image binaire pour trouver les contours
    extractContours(copy_blobs);
    copy_blobs.release();

    if (!m_contours.empty()){
        m_contours.erase(std::remove_if(m_contours.begin(),m_contours.end(),compareAreaBelow),m_contours.end());
        /// polygon approximation and maximum inscribed circle
        for (unsigned int i = 0; i < m_contours.size(); i++) {
            contoursApproximation(i);
            findMaximumInscribebCircle(i);
cv::circle(contour_detection,m_centerMax[i],m_radiusMax[i],cv::Scalar(255,255,255));
        }
        /// set a new region of interest based on the maximum inscribed circle's center
        setRegionOfInterest();
    }
    m_contours.clear();

    /// Contours extraction of new ROI
    extractContours(m_blobs);
    if (!m_contours.empty()){
        m_contours.erase(std::remove_if(m_contours.begin(),m_contours.end(),compareAreaBelow),m_contours.end());
        //resize based on contours size
        m_centerMin.resize(m_centerMax.size());
        m_radiusMin.resize(m_radiusMax.size());
        m_convexHullIndex.resize(m_contours.size());
        m_convexHullPoints.resize(m_contours.size());
        m_convexityDefects.resize(m_contours.size());
        m_fingersPosition.resize(m_contours.size());
        m_fingersDirection.resize(m_contours.size());

        /// Minimum englobing circle + convex hull
        for (unsigned int i = 0; i < m_contours.size(); i++) {
cv::drawContours(contour_detection,m_contours,i,cv::Scalar(255,255,255));
            findMinimumEnglobingCircle(i);
cv::circle(contour_detection,m_centerMin[i],m_radiusMin[i],cv::Scalar(255,255,255));
            findConvexHullIndex(i);
            findConvexHullPoints(i);
cv::drawContours(contour_detection,m_convexHullPoints,i,cv::Scalar(255,255,255));
            extractFingers(i,15,15);
        }
    }
    gestureRecognition();
    putText(contour_detection,std::to_string(m_stateLeft),cv::Point(100,100),cv::FONT_ITALIC,1,cv::Scalar(255,255,255),2);
    putText(contour_detection,std::to_string(m_stateRight),cv::Point(500,100),cv::FONT_ITALIC,1,cv::Scalar(255,255,255),2);
 /*   for (unsigned int i = 0; i < m_fingersPosition.size();i++)
        for (unsigned int j = 0; j < m_fingersPosition[i].size(); j++)
            cv::circle(contour_detection,m_fingersPosition[i][j],2,cv::Scalar(255,255,255));
    cv::imshow("detection final",contour_detection);*/
}

void Detection::loadBinary(const cv::Mat & binary){
    binary.copyTo(m_blobs);
}

void Detection::extractContours(cv::Mat & image){
    cv::findContours(image,m_contours,CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE);
}

void Detection::contoursApproximation(int index){
    float epsilon = 0.002*arcLength(m_contours[index],true);
    cv::approxPolyDP(m_contours[index],m_contours[index],epsilon,true);
}

void Detection::findMaximumInscribebCircle(int index){
    //recherche cercle max inscrit
    cv::Rect roi = minRectangleContour(m_contours[index]);
    cv::Point center;
    double dist, maxdist = -1;
    //wander only in the box (region of interest)
    for (int k = roi.y; k < roi.y+roi.height; k+=4){
        for (int l = roi.x; l < roi.x+roi.width; l+=4){
            dist = cv::pointPolygonTest(m_contours[index], cv::Point(l,k),true);
            if(dist > maxdist){
                maxdist = dist;
                center = cv::Point(l,k);
            }
        }
    }
    if (maxdist < 0) maxdist = 0; //rajout, sur une frame, il peut y avoir un rayon(maxdist) < 0
    //stock les infos sur centre + rayon
    m_centerMax.push_back(center);
    m_radiusMax.push_back(maxdist);
}

void Detection::setRegionOfInterest(){
    cv::Mat mask_roi = cv::Mat::zeros(m_blobs.rows, m_blobs.cols, CV_8UC1);
    for (unsigned int i = 0; i < m_centerMax.size();i++) {
        cv::circle(mask_roi,m_centerMax[i],m_radiusMax[i]*3.5,cv::Scalar(255,255,255),-1);
    }
    m_blobs &= mask_roi;
}

void Detection::findMinimumEnglobingCircle(int index){
    cv::minEnclosingCircle(m_contours[index],m_centerMin[index],m_radiusMin[index]);
}

void Detection::findConvexHullIndex(int index){
    cv::convexHull(m_contours[index],m_convexHullIndex[index]);
}

void Detection::findConvexHullPoints(int index){
    cv::convexHull(m_contours[index],m_convexHullPoints[index]);
}

void Detection::extractFingers(int index, int K, int fwindow) {
    cv::convexityDefects(m_contours[index],m_convexHullIndex[index],m_convexityDefects[index]);
    //add "shift" last points to the beginning
    int indexEnd = m_contours[index].size() - 1;
    int shift = K + fwindow;
    for (int m = 0; m < shift; m++) {
        m_contours[index].insert(m_contours[index].begin(),m_contours[index][indexEnd]);
    }
    //add "shift" first points to the end
    for (int m = 0; m < shift; m++) {
        m_contours[index].push_back(m_contours[index][m+shift]);
    }

    //convexity defects of contour i
    //keep only what meet the requirements
    for (unsigned int j = 0; j < m_convexityDefects[index].size(); j++) {
        //get all information we need
        cv::Vec4i currentDefects = m_convexityDefects[index][j];
        cv::Point start = m_contours[index][currentDefects[0]+shift];
        cv::Point end = m_contours[index][currentDefects[1]+shift];
        cv::Point farthest_pt = m_contours[index][currentDefects[2]+shift];
        float dist = currentDefects[3]/256.f;
        cv::Point minCurrent, minCurrentMK, minCurrentPK;

        //meet the first requirement
        //convexity defect with radiusMaxInscribe < dist < radiusMinEnclosing
        if ((m_radiusMax[index]<dist && dist<m_radiusMin[index]) &&
            //second requirement
            //angle between start and end point < 90°
            (calculateAngle(start,end,farthest_pt) < 100.f)) {
                //k-curvature < 60°
                //add to vector the defects
            for (int p = 0; p < 2; p++){ //start + end
                float minAngle = 60;
                for (int l = -fwindow; l <= fwindow; l++) {
                    cv::Point current,currentMK, currentPK;
                    current = m_contours[index][currentDefects[p]+l+shift]; //from start with the shift (50 points)
                    currentMK = m_contours[index][currentDefects[p]+l+shift-K]; // minus K
                    currentPK = m_contours[index][currentDefects[p]+l+shift+K]; // plus K
                    float currentAngle = calculateAngle(currentMK,currentPK,current);
                    if (currentAngle < minAngle) {
                        minAngle = currentAngle;
                        minCurrent = current;
                        minCurrentMK = currentMK;
                        minCurrentPK = currentPK;
                    }
                }
                if (minAngle < 60) { //on a effectivement un doigt
                    if (m_fingersDirection[index].size() >= 6 && p == 0) { //moyenner les deux points
                        //modifier les 3 derniers points (mk,curr,pk) de end du dernier defect
                        //avec les 3 nouveaux de start qui devraient être proche
                        m_fingersDirection[index][m_fingersDirection[index].size() - 1] =
                                cv::Point((m_fingersDirection[index][m_fingersDirection[index].size() - 1].x + minCurrentPK.x)/2,
                                (m_fingersDirection[index][m_fingersDirection[index].size() - 1].y + minCurrentPK.y)/2);
                        m_fingersDirection[index][m_fingersDirection[index].size() - 2] =
                                cv::Point((m_fingersDirection[index][m_fingersDirection[index].size() - 2].x + minCurrent.x)/2,
                                (m_fingersDirection[index][m_fingersDirection[index].size() - 2].y + minCurrent.y)/2);
                        m_fingersDirection[index][m_fingersDirection[index].size() - 3] =
                                cv::Point((m_fingersDirection[index][m_fingersDirection[index].size() - 3].x + minCurrentMK.x)/2,
                                (m_fingersDirection[index][m_fingersDirection[index].size() - 3].y + minCurrentMK.y)/2);
                    }
                    else {
                        //store MK current PK
                        m_fingersDirection[index].push_back(minCurrentMK);
                        m_fingersDirection[index].push_back(minCurrent);
                        m_fingersDirection[index].push_back(minCurrentPK);
                        if (m_fingersPosition[index].size() >= 1)
                            m_fingersPosition[index].push_back(farthest_pt);
                        m_fingersPosition[index].push_back(minCurrent);
                    }
                }
            }
        }
    }
    ///cas particulier à revoir (un doigt, finger pointing)
}

//basic gesture recognition
void Detection::gestureRecognition(){
    StateHand s_left = e_nothing, s_right = e_nothing, s_current;
    int nbFingers;

    for (unsigned int i = 0; i < m_fingersPosition.size(); i++){ //max 2 contours in image matching the 2 hands
        (m_fingersPosition[i].empty()) ? nbFingers = 0 : nbFingers = (m_fingersPosition[i].size()/2)+1;
        s_current = e_nothing;
        //do all processing to get pattern
        if (nbFingers == 2) {
            //gun
            if (calculateAngle(m_fingersPosition[i][0],m_fingersPosition[i][2],m_fingersPosition[i][1]) >= 70 &&
                    calculateAngle(m_fingersPosition[i][0],m_fingersPosition[i][2],m_fingersPosition[i][1]) <= 100)
                    s_current = e_gun;
            //pinch
            if (calculateAngle(m_fingersPosition[i][0],m_fingersPosition[i][2],m_fingersPosition[i][1]) < 70)
                    s_current = e_pinch;
        }
        //all other patterns
	if (nbFingers == 4 || nbFingers == 5) {
		s_current = e_open;
	}
        //check if it's left or right hand
        (m_centerMax[i].x < m_blobs.cols/2) ? s_left = s_current : s_right = s_current;
    }
    //update states
    updateStates(s_left, s_right);

}

//update hands' states if gesture recognize for more than 600ms
void Detection::updateStates(StateHand left, StateHand right){
    //check left hand
    if (left == m_stateLeft){
        //reset timer
        m_timerLeft.start();
    }
    else{
        if (m_timerLeft.isTimeout(150)){
            //change state and reset timer
            setStateLeft(left);
            m_timerLeft.start();
        }
        //else don't change anything, let the timer be
    }
    //check right hand
    if (right == m_stateRight){
        //reset timer
        m_timerRight.start();
    }
    else{
        if (m_timerRight.isTimeout(150)){
            //change state and reset timer
            setStateRight(right);
            m_timerRight.start();
        }
        //else don't change anything, let the timer be
    }
}

void Detection::clearAll(){
    m_contours.clear();
    m_convexHullIndex.clear();
    m_convexHullPoints.clear();
    m_convexityDefects.clear();
    m_centerMax.clear();
    m_centerMin.clear();
    m_radiusMax.clear();
    m_radiusMin.clear();
    m_fingersPosition.clear();
    m_fingersDirection.clear();
}

float Detection::calculateAngle(cv::Point start, cv::Point end, cv::Point farthest) const {
    float distStartFar, distEndFar, distStartEnd;
    distStartFar = sqrt((start.x - farthest.x)*(start.x - farthest.x) + (start.y - farthest.y)*(start.y - farthest.y));
    distEndFar = sqrt((end.x - farthest.x)*(end.x - farthest.x) + (end.y - farthest.y)*(end.y - farthest.y));
    distStartEnd = sqrt((start.x - end.x)*(start.x - end.x) + (start.y - end.y)*(start.y - end.y));

    return acos((distStartFar*distStartFar + distEndFar*distEndFar - distStartEnd*distStartEnd) / (2*distStartFar*distEndFar))*180/M_PI;
}


///set hand state and get
StateHand Detection::getStateLeft() const {
    return m_stateLeft;
}
StateHand Detection::getStateRight() const {
    return m_stateRight;
}

void Detection::setStateLeft(const StateHand & state){
    m_stateLeft = state;
}
void Detection::setStateRight(const StateHand & state){
    m_stateRight = state;
}

///get all infos
std::vector<std::vector<cv::Point>> Detection::getVectorContours() const{
    return m_contours;
}
std::vector<cv::Point2f> Detection::getVectorCenterMax() const{
    return m_centerMax;
}
std::vector<cv::Point2f> Detection::getVectorCenterMin() const{
    return m_centerMin;
}
std::vector<float> Detection::getVectorRadiusMax() const{
    return m_radiusMax;
}
std::vector<float> Detection::getVectorRadiusMin() const{
    return m_radiusMin;
}
std::vector<std::vector<int>> Detection::getVectorConvexIndex() const{
    return m_convexHullIndex;
}
std::vector<std::vector<cv::Vec4i>> Detection::getVectorConvexityDefects() const{
    return m_convexityDefects;
}
std::vector<std::vector<cv::Point>> Detection::getVectorFingersPosition() const{
    return m_fingersPosition;
}
std::vector<std::vector<cv::Point>> Detection::getVectorFingersDirection() const{
    return m_fingersDirection;
}


/**
  compare operator
  */
bool compareVectorX(const cv::Point& a,const cv::Point& b) {
   return a.x < b.x;
}
bool compareVectorY(const cv::Point& a,const cv::Point& b) {
   return a.y < b.y;
}

bool compareVectorArea(const std::vector<cv::Point>& a,const std::vector<cv::Point>& b) {
   return contourArea(a) > contourArea(b);
}

bool compareAreaBelow(const std::vector<cv::Point>& a) {
    return contourArea(a) < 5000;
}

cv::Rect minRectangleContour(const std::vector<cv::Point>& c) {
    int minx, miny, maxx, maxy;
    minx = (*min_element(c.begin(),c.end(),compareVectorX)).x;
    miny = (*min_element(c.begin(),c.end(),compareVectorY)).y;
    maxx = (*max_element(c.begin(),c.end(),compareVectorX)).x;
    maxy = (*max_element(c.begin(),c.end(),compareVectorY)).y;
    return cv::Rect(minx,miny,maxx-minx,maxy-miny);
}

