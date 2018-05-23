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

#ifndef LOAM_SCANREGISTRATION_H
#define LOAM_SCANREGISTRATION_H


#include "loam_utils/common.h"
#include "loam_utils/Angle.h"
#include "loam_utils/Vector3.h"
#include "loam_utils/CircularBuffer.h"
#include "loam_utils/IMUState.h"
#include "loam/Parameters.h"

#include <stdint.h>
#include <vector>
#include <ros/node_handle.h>
#include <sensor_msgs/Imu.h>
#include <pcl/point_cloud.h>


namespace loam {

/** \brief A pair describing the start end end index of a range. */
typedef std::pair<size_t, size_t> IndexRange;



/** Point label options. */
enum PointLabel {
  CORNER_SHARP = 2,       ///< sharp corner point
  CORNER_LESS_SHARP = 1,  ///< less sharp corner point
  SURFACE_LESS_FLAT = 0,  ///< less flat surface point
  SURFACE_FLAT = -1       ///< flat surface point
};



/** \brief Base class for LOAM scan registration implementations.
 *
 * As there exist various sensor devices, producing differently formatted point clouds,
 * specific implementations are needed for each group of sensor devices to achieve an accurate registration.
 * This class provides common configurations, buffering and processing logic.
 */
class ScanRegistration {
public:
  explicit ScanRegistration(const ScanRegistrationParams& params = ScanRegistrationParams());

  /** \brief Setup component.
   *
   * @param node the ROS node handle
   * @param privateNode the private ROS node handle
   */
  virtual bool setup(ros::NodeHandle& node,
                     ros::NodeHandle& privateNode);

  /** \brief Handler method for IMU messages.
   *
   * @param imuIn the new IMU message
   */
  virtual void handleIMUMessage(const sensor_msgs::Imu::ConstPtr& imuIn);


protected:
  /** \brief Prepare for next scan / sweep.
   *
   * @param scanTime the current scan time
   * @param newSweep indicator if a new sweep has started
   */
  void reset(const ros::Time& scanTime,
             const bool& newSweep = true);

  /** \breif Check is IMU data is available. */
  inline bool hasIMUData() { return _imuHistory.size() > 0; };

  /** \brief Set up the current IMU transformation for the specified relative time.
   *
   * @param relTime the time relative to the scan time
   */
  void setIMUTransformFor(const float& relTime);

  /** \brief Project the given point to the start of the sweep, using the current IMU state and position shift.
   *
   * @param point the point to project
   */
  void transformToStartIMU(pcl::PointXYZI& point);

  /** \brief Extract features from current laser cloud.
   *
   * @param beginIdx the index of the first scan to extract features from
   */
  void extractFeatures(const uint16_t& beginIdx = 0);

  /** \brief Set up region buffers for the specified point range.
   *
   * @param startIdx the region start index
   * @param endIdx the region end index
   */
  void setRegionBuffersFor(const size_t& startIdx,
                           const size_t& endIdx);

  /** \brief Set up scan buffers for the specified point range.
   *
   * @param startIdx the scan start index
   * @param endIdx the scan start index
   */
  void setScanBuffersFor(const size_t& startIdx,
                         const size_t& endIdx);

  /** \brief Mark a point and its neighbors as picked.
   *
   * This method will mark neighboring points within the curvature region as picked,
   * as long as they remain within close distance to each other.
   *
   * @param cloudIdx the index of the picked point in the full resolution cloud
   * @param scanIdx the index of the picked point relative to the current scan
   */
  void markAsPicked(const size_t& cloudIdx,
                    const size_t& scanIdx);

  /** \brief Publish the current result via the respective topics. */
  void publishResult();


private:
  /** \brief Try to interpolate the IMU state for the given time.
   *
   * @param relTime the time relative to the scan time
   * @param outputState the output state instance
   */
  void interpolateIMUStateFor(const float& relTime,
                              IMUState& outputState);


protected:
  ScanRegistrationParams _params;   ///< registration parameter

  Time _sweepStart;                  ///< time stamp of beginning of current sweep
  Time _scanTime;                    ///< time stamp of most recent scan
  IMUState _imuStart;                     ///< the interpolated IMU state corresponding to the start time of the currently processed laser scan
  IMUState _imuCur;                       ///< the interpolated IMU state corresponding to the time of the currently processed laser scan point
  Vector3 _imuPositionShift;              ///< position shift between accumulated IMU position and interpolated IMU position
  size_t _imuIdx;                         ///< the current index in the IMU history
  CircularBuffer<IMUState> _imuHistory;   ///< history of IMU states for cloud registration

  pcl::PointCloud<pcl::PointXYZI> _laserCloud;   ///< full resolution input cloud
  std::vector<IndexRange> _scanIndices;          ///< start and end indices of the individual scans withing the full resolution cloud

  pcl::PointCloud<pcl::PointXYZI> _cornerPointsSharp;      ///< sharp corner points cloud
  pcl::PointCloud<pcl::PointXYZI> _cornerPointsLessSharp;  ///< less sharp corner points cloud
  pcl::PointCloud<pcl::PointXYZI> _surfacePointsFlat;      ///< flat surface points cloud
  pcl::PointCloud<pcl::PointXYZI> _surfacePointsLessFlat;  ///< less flat surface points cloud
  pcl::PointCloud<pcl::PointXYZ> _imuTrans;                ///< IMU transformation information

  std::vector<float> _regionCurvature;      ///< point curvature buffer
  std::vector<PointLabel> _regionLabel;     ///< point label buffer
  std::vector<size_t> _regionSortIndices;   ///< sorted region indices based on point curvature
  std::vector<int> _scanNeighborPicked;     ///< flag if neighboring point was already picked

  ros::Subscriber _subImu;    ///< IMU message subscriber

  ros::Publisher _pubLaserCloud;              ///< full resolution cloud message publisher
  ros::Publisher _pubCornerPointsSharp;       ///< sharp corner cloud message publisher
  ros::Publisher _pubCornerPointsLessSharp;   ///< less sharp corner cloud message publisher
  ros::Publisher _pubSurfPointsFlat;          ///< flat surface cloud message publisher
  ros::Publisher _pubSurfPointsLessFlat;      ///< less flat surface cloud message publisher
  ros::Publisher _pubImuTrans;                ///< IMU transformation message publisher
};

} // end namespace loam


#endif //LOAM_SCANREGISTRATION_H
