#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include "eco.hpp"
#include "parameters.hpp"
#include "debug.hpp"
//#include <unistd.h>   // sleep()

using namespace std;
using namespace cv;
using namespace eco;


cv::Mat frame;
cv::Rect select_rect;
bool select_flag=false;
cv::Point origin;
void onMouse(int event,int x,int y,int,void*);

int main(int argc, char **argv)
{
    // Read from the video OR camera =========================================
    // VideoCapture video("/home/lg/projects/OpenTracker/sequences/VID_20190329_005828.mp4");
    // VideoCapture video("/media/lg/Windows/BaiduNetdiskDownload/night1.ts");
    VideoCapture video(0); // Using the computer's camera
    // Exit if video is not opened
    if(!video.isOpened())
    {
        std::cout << "Could not read video file" << std::endl;
        return 1;      
    }
    video.read(frame);
    cv::imshow("OpenTracker",frame);
    namedWindow("OpenTracker",1);
    setMouseCallback("OpenTracker",onMouse,0); //捕捉鼠标
    cv::waitKey(0);  // draw the init rectangle by hand

    float x, y, w, h;
    x = select_rect.x;
    y = select_rect.y;
    w = select_rect.width;
    h = select_rect.height;

    double timereco = (double)getTickCount();
    ECO ecotracker;
    Rect2f ecobbox(x, y, w, h);
    eco::EcoParameters parameters;
    parameters.useCnFeature = false;
    ecotracker.init(frame, ecobbox, parameters);
    float fpsecoini = getTickFrequency() / ((double)getTickCount() - timereco);

    int f = 0;
    cv::Mat frameDraw;
    while(video.read(frame))
    {     
        frame.copyTo(frameDraw);
        if (!frame.data)
        {
            std::cout << "Could not open or find the image" << std::endl;
            return -1;
        }
        timereco = (double)getTickCount();
        bool okeco = ecotracker.update(frame, ecobbox);
        float fpseco = getTickFrequency() / ((double)getTickCount() - timereco);
        if (okeco)
        {
            rectangle(frameDraw, ecobbox, Scalar(255, 0, 255), 2, 1); //blue
        }
        else
        {
            putText(frameDraw, "ECO tracking failure detected", cv::Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 255), 2);
            imshow("OpenTracker", frameDraw);
            //waitKey(0);
        }
        // Display FPS on frameDraw
        ostringstream os; 
        os << float(fpseco); 
        putText(frameDraw, "FPS: " + os.str(), Point(100, 30), FONT_HERSHEY_SIMPLEX,
                0.75, Scalar(255, 0, 255), 2);

        if (parameters.debug == 0)
        {
            imshow("OpenTracker", frameDraw);
        }

        int c = cvWaitKey(1);
        if (c != -1)
            c = c % 256;
        if (c == 27)
        {
            cvDestroyWindow("OpenTracker");
            exit(1);
        }

        std::cout << "Frame:" << f << " FPS:" << fpseco << endl;
        f++;
    }

    std::cout << "Frames:" << f - 2 << " IniFps:" << fpsecoini << std::endl;

    return 0;
}

/************************************************************************************************************************/
void onMouse(int event,int x,int y,int,void*)
{
    std::cout<<"-----------------onMouse-----------------"<<std::endl;
    //Point origin;//不能在这个地方进行定义，因为这是基于消息响应的函数，执行完后origin就释放了，所以达不到效果。
    if(select_flag)
    {
        select_rect.x=MIN(origin.x,x);//不一定要等鼠标弹起才计算矩形框，而应该在鼠标按下开始到弹起这段时间实时计算所选矩形框
        select_rect.y=MIN(origin.y,y);
        select_rect.width=abs(x-origin.x);//算矩形宽度和高度
        select_rect.height=abs(y-origin.y);
        select_rect&=Rect(0,0,frame.cols,frame.rows);//保证所选矩形框在视频显示区域之内
    }
    if(event==CV_EVENT_LBUTTONDOWN)
    {
        select_flag=true;//鼠标按下的标志赋真值
        origin=Point(x,y);//保存下来单击是捕捉到的点
        select_rect=Rect(x,y,0,0);//这里一定要初始化，宽和高为(0,0)是因为在opencv中Rect矩形框类内的点是包含左上角那个点的，但是不含右下角那个点
    }
    else if(event==CV_EVENT_LBUTTONUP)
    {
        select_flag=false;
    }
}
