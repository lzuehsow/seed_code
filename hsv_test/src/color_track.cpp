//Includes all the headers necessary to use the most common public pieces of the ROS system.
#include <ros/ros.h>
//Use image_transport for publishing and subscribing to images in ROS
#include <image_transport/image_transport.h>
//Use cv_bridge to convert between ROS and OpenCV Image formats
#include <cv_bridge/cv_bridge.h>
//Include some useful constants for image encoding. Refer to: http://www.ros.org/doc/api/sensor_msgs/html/namespacesensor__msgs_1_1image__encodings.html for more info.
#include <sensor_msgs/image_encodings.h>
//Include headers for OpenCV Image processing
#include <opencv2/imgproc/imgproc.hpp>
//Include headers for OpenCV GUI handling
#include <opencv2/highgui/highgui.hpp>

//Store all constants for image encodings in the enc namespace to be used later.
namespace enc = sensor_msgs::image_encodings;

//Declare a string with the name of the window that we will create using OpenCV where processed images will be displayed.
static char *WINDOW = "Image Processed";

//Use method of ImageTransport to create image publisher
image_transport::Publisher pub;

int LowerH = 0;
int LowerS = 0;
int LowerV = 0;
int UpperH = 180;
int UpperS = 196;
int UpperV = 170;

void colorDetectionCallback(const sensor_msgs::ImageConstPtr& original_image)
{
    //Convert from the ROS image message to a CvImage suitable for working with OpenCV for processing
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
        //Always copy, returning a mutable CvImage
        //OpenCV expects color images to use BGR channel order.
        cv_ptr = cv_bridge::toCvCopy(original_image, enc::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
        //if there is an error during conversion, display it
        ROS_ERROR("tutorialROSOpenCV::main.cpp::cv_bridge exception: %s", e.what());
        return;
    }
    cv::Mat img_mask,img_hsv;
    cv::cvtColor(cv_ptr->image,img_hsv,CV_BGR2HSV);
    cv::inRange(img_hsv,cv::Scalar(LowerH,LowerS,LowerV),cv::Scalar(UpperH,UpperS,UpperV),img_mask);
    //Display the image using OpenCV
    cv::circle(img_mask, cv::Point(100,100), 5, cv::Scalar( 0, 0, 255 ), 1, 8, 0);
    cv::imshow(WINDOW, img_mask);
    //Add some delay in miliseconds. The function only works if there is at least one HighGUI window created and the window is active. If there are several HighGUI windows, any of them can be active
    /**
    * The publish() function is how you send messages. The parameter
    * is the message object. The type of this object must agree with the type
    * given as a template parameter to the advertise<>() call, as was done
    * in the constructor in main().
    */
    //Convert the CvImage to a ROS image message and publish it on the "camera/image_processed" topic.
    pub.publish(cv_ptr->toImageMsg());
}

int main(int argc, char **argv)
{

    ros::init(argc, argv, "image_processor");

    // //OpenCV HighGUI call to create a display window on start-up.
    cv::namedWindow(WINDOW, CV_WINDOW_AUTOSIZE);

    ros::NodeHandle nh;
    //Create an ImageTransport instance, initializing it with our NodeHandle.
    image_transport::ImageTransport it(nh);

    cv::namedWindow("Ball");
    cv::createTrackbar("LowerH","Ball",&LowerH,180,NULL);
    cv::createTrackbar("UpperH","Ball",&UpperH,180,NULL);
    cv::createTrackbar("LowerS","Ball",&LowerS,256,NULL);
    cv::createTrackbar("UpperS","Ball",&UpperS,256,NULL);
    cv::createTrackbar("LowerV","Ball",&LowerV,256,NULL);
    cv::createTrackbar("UpperV","Ball",&UpperV,256,NULL);

    //image_transport::Subscriber sub = it.subscribe("/image_raw", 1, imageCallback);
    image_transport::Subscriber sub = it.subscribe("my_stereo/left/image_raw", 1, colorDetectionCallback);

    //OpenCV HighGUI call to destroy a display window on shut-down.
    cv::destroyWindow(WINDOW);

    pub = it.advertise("/imageconverter/image_processed", 1);

    while(ros::ok()){
      ros::spinOnce();
      cv::waitKey(3);
    }

    //ROS_INFO is the replacement for printf/cout.
    ROS_INFO("tutorialROSOpenCV::main.cpp::No error.");

}