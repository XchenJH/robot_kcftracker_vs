#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>  
#include <fstream>  
#include <strstream>
#include "kcftracker.hpp"
#include "parameter.h"

using namespace cv;
using namespace std;

string VIDEO_WINDOW_NAME = "kcftrack result";
//roi
Mat tmp_3;
Point2i pre_pt, end_pt, cur_pt;
bool select_end_flag = false;
void mouseSelectROI(int event,int x,int y,int flags,void *ustc)
{
  cur_pt = Point2i(x,y);
  
  if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))
  {
    if(select_end_flag == false)
    {
      imshow(VIDEO_WINDOW_NAME,tmp_3);
      waitKey(1);
    }
    return;
  }
  if (event == CV_EVENT_LBUTTONDOWN)
  {
    pre_pt = Point2i(x,y);
  }
  if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
  {
	end_pt = Point2i(cur_pt.x,pre_pt.y+(cur_pt.x-pre_pt.x)/widthHeightRatio);
	rectangle(tmp_3,pre_pt,end_pt,Scalar(0,255,0,0),1,8,0);
  }
  if (event == CV_EVENT_LBUTTONUP)
  {
    rectangle(tmp_3,pre_pt,end_pt,Scalar(0,255,0,0),1,8,0);
    select_end_flag = true;
  }
  imshow(VIDEO_WINDOW_NAME,tmp_3);
  waitKey(1);
  return;
}

int main(int argc, char** argv)
{
	//video
	double image_hight;
	double image_width;
	VideoCapture input_video(source_video_name);
	VideoWriter output_video;
	//frame
	int frame_num;
	Mat src_3, dst_3;
	//kcf
	KCFTracker tracker;
	Rect track_result;
	float init_xMin;
	float init_yMin;
	float init_xMax;
	float init_yMax;
	float init_width;
	float init_height;
	bool kcf_init_end_flag;
	
	//initial
	if(!input_video.isOpened()){cout<<"��Ƶ��ȡ����"<<endl;system("puase");return -1;}
	if(show_video_flag)
	{
		namedWindow(VIDEO_WINDOW_NAME);
	}
	//frame
	frame_num = 1;
	//kcf
	if(lab_flag == true)hog_flag = true;
	tracker = KCFTracker(hog_flag, fixed_window_flag, multiscale_flag, lab_flag);
	kcf_init_end_flag = false;

	bool stop = false;
	for (int fnum = 1;!stop;fnum++)
	{
		cout<<fnum<<endl;
		if (!input_video.read(src_3)){cout<<"��Ƶ����"<<endl;waitKey(0); break;}//��ȡ��Ƶ֡

		src_3.copyTo(dst_3);
		//cout<<"rows"<<src_3.rows<<endl;
		//cout<<"cols"<<src_3.cols<<endl;

		//fitst frame
		if(frame_num == 1)
		{
			image_hight = src_3.rows;
			image_width = src_3.cols;
			if(save_video_flag)
				output_video = VideoWriter(video_file_name, CV_FOURCC('M', 'J', 'P', 'G'), video_rate, Size(image_width, image_hight));
			//set call back function for selecting inital target roi
			src_3.copyTo(tmp_3);
			setMouseCallback(VIDEO_WINDOW_NAME,mouseSelectROI,0);
		}
		frame_num++;
		//imshow image for roi selecting
		if(select_end_flag == false)
		{
			imshow(VIDEO_WINDOW_NAME, tmp_3);
			waitKey(0);
			src_3.copyTo(tmp_3);
		}
		//initializing kcf tracker
		if(select_end_flag == true && kcf_init_end_flag == false)
		{
			// Using min and max of X and Y for groundtruth rectangle
			init_xMin =  min(pre_pt.x, end_pt.x);
			init_yMin =  min(pre_pt.y, end_pt.y);
			init_xMax =  max(pre_pt.x, end_pt.x);
			init_yMax =  max(pre_pt.y, end_pt.y);
			init_width = init_xMax - init_xMin;
			init_height = init_yMax - init_yMin;
			// First frame, give the groundtruth to the tracker
			tracker.init( Rect(init_xMin, init_yMin, init_width, init_height), src_3 );
			rectangle( dst_3, Point( init_xMin, init_yMin ), Point( init_xMax, init_yMax), Scalar( 0, 255, 255 ), 1, 8 );
			kcf_init_end_flag = true;
			continue;
		}

		//kcf updata
		track_result = tracker.update(src_3);
		rectangle( dst_3, Point( track_result.x, track_result.y ), Point( track_result.x+track_result.width, track_result.y+track_result.height), Scalar( 0, 255, 255 ), 1, 8 );

		//save set
		if(save_set_flag)
		{
			strstream ss;
			string s;
			ss<<set_folder<<frame_num<<".jpg";
			ss>>s;
			imwrite(s,src_3(track_result));
			cout<<s<<endl;
		}

		//save and show video
		if(save_video_flag)
		{
			output_video<<dst_3;
		}
		if(show_video_flag)
		{
			imshow(VIDEO_WINDOW_NAME, dst_3);
			if(waitKey(1)>=0)stop = true;
		}
	}

	return 0;
}
