//
// Created by bandera on 09.06.16.
//

#include "LandmarkLocalizerInterface.h"
#include "../StargazerConversionMethods.h"
#include "stargazer/CeresLocalizer.h"
#include "stargazer/TriangulationLocalizer.h"

using namespace stargazer_ros_tool;

LandmarkLocalizerInterface::LandmarkLocalizerInterface(ros::NodeHandle node_handle, ros::NodeHandle private_node_handle)
        : params_{LandmarkLocalizerInterfaceParameters::getInstance()} {

    // Set parameters
    params_.fromNodeHandle(private_node_handle);
    last_timestamp_ = ros::Time::now();
    debugVisualizer_.SetWaitTime(10);

    if (params_.use_ceres)
        localizer_ = std::make_unique<stargazer::CeresLocalizer>(params_.stargazer_config);
    else
        localizer_ = std::make_unique<stargazer::TriangulationLocalizer>(params_.stargazer_config);

    // Initialize publisher
    pose_pub = private_node_handle.advertise<geometry_msgs::PoseStamped>("pose", 1);
    lm_sub = private_node_handle.subscribe<stargazer_ros_tool::Landmarks>(
        "/landmarks_seen", 1, &LandmarkLocalizerInterface::landmarkCallback, this);
}

void LandmarkLocalizerInterface::landmarkCallback(const stargazer_ros_tool::Landmarks::ConstPtr& msg) {

    ros::Time this_timestamp = msg->header.stamp;
    double dt = (this_timestamp - last_timestamp_).toSec();
    ros::Time last_timestamp = this_timestamp;

    std::vector<stargazer::ImgLandmark> detected_landmarks = convert2ImgLandmarks(*msg);

    // Localize
    localizer_->UpdatePose(detected_landmarks, dt);
    stargazer::pose_t pose = localizer_->getPose();

    // Publish tf pose
    tf::StampedTransform transform;
    pose2tf(pose, transform);
    transform.stamp_ = msg->header.stamp;
    transform.frame_id_ = params_.map_frame;
    transform.child_frame_id_ = params_.robot_frame;
    tf_pub.sendTransform(transform);

    geometry_msgs::PoseStamped poseStamped;
    poseStamped.header.frame_id = params_.robot_frame;
    poseStamped.header.stamp = msg->header.stamp;
    poseStamped.pose.orientation.w = 1;
    pose_pub.publish(poseStamped);

    //  Visualize
    if (params_.debug_mode) {
        cv::Mat img = cv::Mat::zeros(1024, 1360, CV_8UC3);
        debugVisualizer_.DrawLandmarks(img, localizer_->getLandmarks(), localizer_->getIntrinsics(), pose);
        debugVisualizer_.DrawLandmarks(img, detected_landmarks);
        debugVisualizer_.ShowImage(img, "ReprojectionImage");
    }
}
