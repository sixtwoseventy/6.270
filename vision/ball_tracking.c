#include <stdlib.h>
#include <stdbool.h>
#include "vision.h"


double getObjectDistance(board_coord a, board_coord b){
    if (a.id != 0xFF || b.id != 0xFF){
        return INFINITY;
    }
    return dist_sq(cvPoint(a.x,a.y), cvPoint(b.x,b.y));
}

typedef struct {
    int prevIdx;
    int curIdx;
    double distance;
} obj_dist;

int compareDists(const void *a, const void *b){
    obj_dist *A = (obj_dist*)a;
    obj_dist *B = (obj_dist*)b;

    if (A->distance < B->distance)
        return -1;
    else if (A->distance == B->distance)
        return 0;
    else 
        return 1;
}

//push currentObjects into previousObjects by matching balls to try and maintain
//proper ordering
void matchObjects(board_coord previousObjects[NUM_OBJECTS], board_coord currentObjects[NUM_OBJECTS]){

    obj_dist distances[(NUM_OBJECTS-2)*(NUM_OBJECTS-2)];

    int i = 0;
    for (int prevIdx = 2; prevIdx < NUM_OBJECTS; prevIdx++) { //don't match index 0 or 1 - these are always robots
        for (int curIdx = 2; curIdx < NUM_OBJECTS; curIdx++){
           obj_dist d;
           d.prevIdx = prevIdx;
           d.curIdx = curIdx;
           d.distance = getObjectDistance(previousObjects[prevIdx], currentObjects[curIdx]);
           distances[i] = d;
           i++;
        }
    }

    //clear previousObjects
    for (int j = 2; j < NUM_OBJECTS; j++){
        previousObjects[j].id = 0;
        previousObjects[j].x = 0;
        previousObjects[j].y = 0;
        previousObjects[j].radius = 0;
        previousObjects[j].hue = 0;
        previousObjects[j].saturation = 0;
    }

    qsort(distances, (NUM_OBJECTS-2)*(NUM_OBJECTS-2), sizeof(obj_dist), &compareDists);

    bool curObjectPlaced[NUM_OBJECTS]; //flags for if an object has been placed 
    bool prevObjectUsed[NUM_OBJECTS]; //flags for if an object has been placed 
    for (int j = 0; j < NUM_OBJECTS; j++){
        curObjectPlaced[j] = 0;
        prevObjectUsed[j] = 0;
    }


    //loop through sorted obj_dist's and match lowest distance first
    for (int j = 0; j < (NUM_OBJECTS-2)*(NUM_OBJECTS-2); j++){
        if (!curObjectPlaced[distances[j].curIdx] &&
            !prevObjectUsed[distances[j].prevIdx]){

            //printf("placing curobject %i into objects[%i]; dist=%.2f\n", distances[j].curIdx, distances[j].prevIdx, distances[j].distance);
            //place curObject into prevObjects array
            previousObjects[distances[j].prevIdx] = currentObjects[distances[j].curIdx];

            curObjectPlaced[distances[j].curIdx] = 1;
            prevObjectUsed[distances[j].prevIdx] = 1;
        }
    }
}

void processBalls(IplImage *img, IplImage *gray, IplImage *out){
    CvSeq *contours;

    cvSmooth(gray, gray, CV_GAUSSIAN, 5, 5, 0, 0);
    cvThreshold( gray, gray, ball_threshold, 255, CV_THRESH_BINARY );
#if SHOW_FILTERED_OUTPUT
    cvShowImage( WND_FILTERED, gray );
#endif

    // find contours and store them all as a list
    cvFindContours( gray, storage, &contours, sizeof(CvContour),
        CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );


    int curObject = 2;


    board_coord tempObjects[NUM_OBJECTS];

    // test each contour
    while( contours ) {

        CvRect boundRect = cvBoundingRect(contours, 0);

        // take the next contour
        contours = contours->h_next;

        if (boundRect.width >= min_ball_dim &&
            boundRect.width <= max_ball_dim &&
            boundRect.height>= min_ball_dim &&
            boundRect.height<= max_ball_dim){
    
            int lesserDim, greaterDim;

            if (boundRect.width>boundRect.height){
                lesserDim = boundRect.height;
                greaterDim = boundRect.width;
            } else {
                lesserDim = boundRect.width;
                greaterDim = boundRect.height;
            }

            //check to make sure the boundRect is relatively square
            if (lesserDim < 0.6 * greaterDim)
                continue;

            CvPoint2D32f center = project(projection, cvPoint2D32f(boundRect.x + boundRect.width/2,boundRect.y+boundRect.height/2));

            //check if the contour is within the playing field
            if (center.x >= X_MIN &&
                center.x <= X_MAX &&
                center.y >= Y_MIN &&
                center.y <= Y_MAX){

                int x1 = boundRect.x;
                int y1 = boundRect.y;
                int x2 = boundRect.x + boundRect.width;
                int y2 = boundRect.y + boundRect.height;

                int goodPoints = 0;
                goodPoints += get_5pixel_avg(gray,x1,y1) < 0.1;
                goodPoints += get_5pixel_avg(gray,x2,y1) < 0.1;
                goodPoints += get_5pixel_avg(gray,x1,y2) < 0.1;
                goodPoints += get_5pixel_avg(gray,x2,y2) < 0.1;
                goodPoints += get_5pixel_avg(gray,(x1+x2)/2,(y1+y2)/2) > 0.9;

                goodPoints += get_5pixel_avg(gray,(x1+x2)/2,(y1*1 + y2*3)/4) > 0.9;
                goodPoints += get_5pixel_avg(gray,(x1+x2)/2,(y1*3 + y2*1)/4) > 0.9;
                goodPoints += get_5pixel_avg(gray,(x1*1+x2*3)/4,(y1 + y2)/2) > 0.9;
                goodPoints += get_5pixel_avg(gray,(x1*3+x2*1)/4,(y1 + y2)/2) > 0.9;

                if (goodPoints >= 7){
                    CvPoint2D32f display_pt = project(displayMatrix, center);
                    CvScalar color = CV_RGB(0,255,255);
                    int thickness = 2;
                    if (curObject < NUM_OBJECTS){
                        float x = boundRect.x + boundRect.width/2;
                        float y = boundRect.y + boundRect.height/2;

                        IplImage *tempBGR = cvCreateImage( cvSize(1,1), 8, 3 );
                        IplImage *tempHSV = cvCreateImage( cvSize(1,1), 8, 3 );

                        cvSet2D(tempBGR, 0,0, cvGet2D(img,y,x));
                        cvCvtColor(tempBGR, tempHSV, CV_BGR2HSV);
                        CvScalar pixelHSV = cvGet2D(tempHSV,0,0);

                        cvReleaseImage(&tempBGR);
                        cvReleaseImage(&tempHSV);

                        tempObjects[curObject].id = 0xFF;
                        tempObjects[curObject].x = center.x;
                        tempObjects[curObject].y = center.y;
                        tempObjects[curObject].radius = clamp(((float)boundRect.width-min_ball_dim)/(max_ball_dim-min_ball_dim)*16.0, 0, 15);
                        tempObjects[curObject].hue = (int)(pixelHSV.val[0] * 16.0 / 180.0 + 0.5) % 16;
                        tempObjects[curObject].saturation = pixelHSV.val[1] * 16.0 / 256.0;
                        curObject++;
                    } else {
                        color = CV_RGB(255,0,0);
                        thickness = 4;
                    }
                    cvCircle(out, cvPoint(display_pt.x, display_pt.y), 10, color, thickness, CV_AA,0);
                }
            }
        }
    }

    //zero all other objects
    for (; curObject < NUM_OBJECTS; curObject++){
        tempObjects[curObject].id = 0xAA;
    }

    matchObjects(objects, tempObjects);

    if (!warpDisplay) {
        for (int i = 2; i<NUM_OBJECTS; i++) {
            if (objects[i].id != 0xFF) {
                continue;
            }
            CvPoint2D32f p = project(displayMatrix, cvPoint2D32f(objects[i].x,objects[i].y));

            char buf[256];
            sprintf(buf, "%d", i);
            CvSize textSize;
            int baseline;
            cvGetTextSize(buf, &font, &textSize, &baseline);
            cvPutText(out, buf, cvPoint(p.x-textSize.width/2.0, p.y+textSize.height+10+5), &font, CV_RGB(0,255,255));
        }
    }

}


