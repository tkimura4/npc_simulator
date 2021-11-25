/*
 * Copyright (c) 2012, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <algorithm>
#include <random>

#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include "autoware_perception_msgs/msg/shape.hpp"
#include "autoware_perception_msgs/msg/semantic.hpp"

#include <unique_identifier_msgs/msg/uuid.hpp>

#include "car_pose.hpp"

namespace rviz_plugins
{
NPCSimCarInitialPoseTool::NPCSimCarInitialPoseTool()
{
  shortcut_key_ = 'k';

  topic_property_ = new rviz_common::properties::StringProperty(
    "Pose Topic", "/simulation/npc_simulator/object_info",
    "The topic on which to publish dummy object info.", getPropertyContainer(), SLOT(updateTopic()),
    this);
  std_dev_x_ = new rviz_common::properties::FloatProperty(
    "X std deviation", 0.03, "X standard deviation for initial pose [m]", getPropertyContainer());
  std_dev_y_ = new rviz_common::properties::FloatProperty(
    "Y std deviation", 0.03, "Y standard deviation for initial pose [m]", getPropertyContainer());
  std_dev_z_ = new rviz_common::properties::FloatProperty(
    "Z std deviation", 0.03, "Z standard deviation for initial pose [m]", getPropertyContainer());
  std_dev_theta_ = new rviz_common::properties::FloatProperty(
    "Theta std deviation", 5.0 * M_PI / 180.0, "Theta standard deviation for initial pose [rad]",
    getPropertyContainer());
  position_z_ = new rviz_common::properties::FloatProperty(
    "Z position", 0.0, "Z position for initial pose [m]", getPropertyContainer());
  velocity_ = new rviz_common::properties::FloatProperty(
    "Velocity", 0.0, "velocity [m/s]", getPropertyContainer());
  std_dev_x_->setMin(0);
  std_dev_y_->setMin(0);
  std_dev_z_->setMin(0);
  std_dev_theta_->setMin(0);
  position_z_->setMin(0);
}

void NPCSimCarInitialPoseTool::onInitialize()
{
  PoseTool::onInitialize();
  setName("2D Dummy Car");
  updateTopic();
}

void NPCSimCarInitialPoseTool::updateTopic()
{
  rclcpp::Node::SharedPtr raw_node = context_->getRosNodeAbstraction().lock()->get_raw_node();
  dummy_object_info_pub_ =
    raw_node->create_publisher<npc_simulator::msg::Object>(topic_property_->getStdString(), 1);
  clock_ = raw_node->get_clock();
}

void NPCSimCarInitialPoseTool::onPoseSet(double x, double y, double theta)
{
  npc_simulator::msg::Object output_msg;
  std::string fixed_frame = context_->getFixedFrame().toStdString();

  // header
  output_msg.header.frame_id = fixed_frame;
  output_msg.header.stamp = clock_->now();

  // semantic
  output_msg.semantic.type = autoware_perception_msgs::msg::Semantic::CAR;
  output_msg.semantic.confidence = 1.0;

  // shape
  output_msg.shape.type = autoware_perception_msgs::msg::Shape::BOUNDING_BOX;
  const double width = 1.8;
  const double length = 4.0;
  output_msg.shape.dimensions.x = length;
  output_msg.shape.dimensions.y = width;
  output_msg.shape.dimensions.z = 2.0;

  // initial state
  // pose
  output_msg.initial_state.pose_covariance.pose.position.x = x;
  output_msg.initial_state.pose_covariance.pose.position.y = y;
  output_msg.initial_state.pose_covariance.pose.position.z = position_z_->getFloat();
  output_msg.initial_state.pose_covariance.covariance[0] =
    std_dev_x_->getFloat() * std_dev_x_->getFloat();
  output_msg.initial_state.pose_covariance.covariance[7] =
    std_dev_y_->getFloat() * std_dev_y_->getFloat();
  output_msg.initial_state.pose_covariance.covariance[14] =
    std_dev_z_->getFloat() * std_dev_z_->getFloat();
  output_msg.initial_state.pose_covariance.covariance[35] =
    std_dev_theta_->getFloat() * std_dev_theta_->getFloat();
  tf2::Quaternion quat;
  quat.setRPY(0.0, 0.0, theta);
  output_msg.initial_state.pose_covariance.pose.orientation = tf2::toMsg(quat);
  RCLCPP_INFO(
    rclcpp::get_logger("NPCSimCarInitialPoseTool"), "Setting pose: %.3f %.3f %.3f %.3f [frame=%s]",
    x, y, position_z_->getFloat(), theta, fixed_frame.c_str());
  // twist
  output_msg.initial_state.twist_covariance.twist.linear.x = velocity_->getFloat();
  output_msg.initial_state.twist_covariance.twist.linear.y = 0.0;
  output_msg.initial_state.twist_covariance.twist.linear.z = 0.0;
  RCLCPP_INFO(
    rclcpp::get_logger("NPCSimCarInitialPoseTool"), "Setting twist: %.3f %.3f %.3f [frame=%s]",
    velocity_->getFloat(), 0.0, 0.0, fixed_frame.c_str());

  // action
  output_msg.action = npc_simulator::msg::Object::ADD;

  // id
  std::mt19937 gen(std::random_device{}());
  std::independent_bits_engine<std::mt19937, 8, uint8_t> bit_eng(gen);
  std::generate(output_msg.id.uuid.begin(), output_msg.id.uuid.end(), bit_eng);

  dummy_object_info_pub_->publish(output_msg);
}

}  // end namespace rviz_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(rviz_plugins::NPCSimCarInitialPoseTool, rviz_common::Tool)
