// Copyright 2013, Ji Zhang, Carnegie Mellon University
// Further contributions copyright (c) 2016, Southwest Research Institute
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is an implementation of the algorithm described in the following paper:
//   J. Zhang and S. Singh. LOAM: Lidar Odometry and Mapping in Real-time.
//     Robotics: Science and Systems Conference (RSS). Berkeley, CA, July 2014.

#ifndef LOAM_TRANSFORM_MAINTENANCE_H
#define LOAM_TRANSFORM_MAINTENANCE_H

#include <mutex>

#include <Eigen/Dense>

#include "loam_utils/common.h"

namespace loam {

/** \brief Implementation of the LOAM transformation maintenance component.
 *
 */
class TransformMaintenance {
public:
  TransformMaintenance();

  void correctEstimate(const Eigen::Vector3d& pos = Eigen::Vector3d::Zero(), 
                       const Eigen::Vector3d& rpy = Eigen::Vector3d::Zero());

  float* getIntegratedTransform() { return _transformMapped; }

  /** \brief Handler method for laser odometry transforms.
   *
   * @param pos the position estimated by the odometry node
   * @param rot the rotation estimated by the odometry node
   */
  void processOdometryTransform(const Eigen::Vector3d& pos, const Eigen::Quaterniond& rot);

  /** \brief Handler method for mapping odometry transforms.
   *
   * @param pos the position estimated by the mapping node
   * @param rot the rotation estimated by the mapping node
   * @param linear_vel position estimated by the mapping node
   * @param angular_vel the rotation estimated by the mapping node
   */
  void processMappingTransform(const Eigen::Vector3d& pos, const Eigen::Quaterniond& rot, const Eigen::Vector3d& linear_vel, const Eigen::Vector3d& angular_vel);


protected:
  void transformAssociateToMap();


private:
  std::mutex main_thread_mutex_;

  float _transformSum[6];
  float _transformIncre[6];
  float _transformMapped[6];
  float _transformBefMapped[6];
  float _transformAftMapped[6];
};

} // end namespace loam


#endif //LOAM_TRANSFORM_MAINTENANCE_H
