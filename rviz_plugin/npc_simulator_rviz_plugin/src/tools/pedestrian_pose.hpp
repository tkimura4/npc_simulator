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

#pragma once

#ifndef Q_MOC_RUN  // See: https://bugreports.qt-project.org/browse/QTBUG-22829
#include <QObject>

#include <rclcpp/node.hpp>

#include <rviz_default_plugins/tools/pose/pose_tool.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/properties/float_property.hpp>
#include <rviz_common/properties/string_property.hpp>
#endif

#include "npc_simulator/msg/object.hpp"

namespace rviz_plugins
{

class NPCSimPedestrianInitialPoseTool : public rviz_default_plugins::tools::PoseTool
{
  Q_OBJECT

public:
  NPCSimPedestrianInitialPoseTool();
  virtual ~NPCSimPedestrianInitialPoseTool() {}
  virtual void onInitialize();

protected:
  virtual void onPoseSet(double x, double y, double theta);

private Q_SLOTS:
  void updateTopic();

private:
  rclcpp::Clock::SharedPtr clock_;
  rclcpp::Publisher<npc_simulator::msg::Object>::SharedPtr dummy_object_info_pub_;

  rviz_common::properties::StringProperty * topic_property_;
  rviz_common::properties::FloatProperty * std_dev_x_;
  rviz_common::properties::FloatProperty * std_dev_y_;
  rviz_common::properties::FloatProperty * std_dev_z_;
  rviz_common::properties::FloatProperty * std_dev_theta_;
  rviz_common::properties::FloatProperty * position_z_;
  rviz_common::properties::FloatProperty * velocity_;
};

}  // namespace rviz_plugins
