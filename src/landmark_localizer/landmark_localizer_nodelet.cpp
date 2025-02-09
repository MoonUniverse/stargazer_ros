#include <nodelet/nodelet.h>
#include <pluginlib/class_list_macros.h>

#include "LandmarkLocalizerInterface.h"

namespace stargazer_ros_tool {

class LandmarkLocalizerInterfaceNodelet : public nodelet::Nodelet {

    virtual void onInit();
    std::unique_ptr<LandmarkLocalizerInterface> m_;
};

void LandmarkLocalizerInterfaceNodelet::onInit() {
    m_ = std::make_unique<LandmarkLocalizerInterface>(getNodeHandle(), getPrivateNodeHandle());
}
} // namespace stargazer_ros_tool

PLUGINLIB_EXPORT_CLASS(stargazer_ros_tool::LandmarkLocalizerInterfaceNodelet, nodelet::Nodelet);