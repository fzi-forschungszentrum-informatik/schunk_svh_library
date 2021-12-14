// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-


// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner <heppner@fzi.de>
 * \date    2014-5-21
 *
 */
//----------------------------------------------------------------------

#include "schunk_svh_library/websocket/RobotState.h"


#include <cmath>
#include <json/json.h>
#include <json/writer.h>


// In Case you prefer a version without JSONCPP dependency
#include <iostream>
//#include <unistd.h>
//#include <sstream>
//#include <iomanip>



namespace schunk_svh_library {
namespace websocket {

RobotState::RobotState() :
  m_tps(100),
  m_ticks(0),
  m_input_to_rad_factor(1)
{
//  for(size_t axe = 0; axe < m_num_axes; axe++)
//  {
//    m_joint_positions[axe] = 0.0;
//    m_joint_velocities[axe] = 0.0;
//    m_joint_errors[axe] = eERR_NO_ERROR;
//    m_joints_homed[axe] = eERR_WARNING;
//    m_joints_enabled[axe] = eERR_WARNING;
//  }
}


// Version without JSONCPP dependency.. but that seams to be reasonable enough
//  /*!
//   * \brief getStateJSON Returns the current robot state as JSON encoded String
//   * \return current robot state as JSON encoded String
//   */
//  std::string getStateJSON()
//  {
//    std::stringstream ss;
//    ss << std::setiosflags(std::ios::fixed) << std::setprecision(5);
//    ss <<   "{";
//    ss <<     "\"type\": \"roboter\",";
//    ss <<     "\"state\": \"";
//    switch (m_movement_state)
//    {
//    case eST_MOVING:
//      ss << "moving";
//      break;
//    case eST_WAITING:
//      ss << "waiting";
//      break;
//    default:
//      ss << "unknown";
//    };
//    ss << "\",";
//    ss <<     "\"position\": {";
//    for (int i = 0; i < m_num_axes; ++i)
//    {
//      ss <<       "\"joint"<<i<<"\": " << m_joint_positions[i] << (i < m_num_axes-1 ? ",":"");
//    }
//    ss <<     "},";
//    ss <<     "\"speed\": {";
//    for (int i = 0; i < m_num_axes; ++i)
//    {
//      ss <<       "\"joint"<<i<<"\": " << (m_joint_velocities[i]*m_tps) << (i < m_num_axes-1 ? ",":"");
//    }
//    ss <<     "},";
//    ss <<     "\"leds\": {";
//    for (int i = 0; i < m_num_axes; ++i)
//    {
//      ss <<       "\"joint"<<i<<"\": " << m_joint_errors[i] << (i < m_num_axes-1 ? ",":"");
//    }
//    ss <<     "}";
//    ss <<   "}";
//    return ss.str();
//  }

void RobotState::shuffleErrors()
{
  for (jnt_value_map::iterator it=m_joints.begin(); it != m_joints.end(); it++)
  {
    it->second.error = static_cast<ErrorState>(rand()% eERR_DIMENSION);
    it->second.homed = static_cast<ErrorState>(rand()% eERR_DIMENSION);
    it->second.enabled = static_cast<ErrorState>(rand()% eERR_DIMENSION);
  }
}

void RobotState::simulateTick()
{
  // simple integration of positions
  for (jnt_value_map::iterator it=m_joints.begin(); it != m_joints.end(); it++)
  {
    it->second.position += it->second.velocity;
    it->second.velocity = std::sin(static_cast<double>(-m_ticks)/m_tps)/m_tps;
  }

  // Do Joint 0 differnt just to get a bit of variation
  m_joints.begin()->second.velocity = std::sin(static_cast<double>(m_ticks)/m_tps)/m_tps;

  ++m_ticks;

  // Shuffle Errors once per seconds (if the simulation is called accordingly)
  if (m_ticks % m_tps == 0)
  {
    shuffleErrors();
  }
}




std::string RobotState::getHintsJSON()
{
  Json::Value message;
  Json::FastWriter json_writer;

  message["type"] = "hints";

  Json::Value hints;

  // Append all the Hints into an Array
  for (std::set<int>::iterator it = m_hints.begin(); it != m_hints.end(); ++it)
  {
    hints.append(*it);
  }

  message["data"] = hints;

  return json_writer.write(message);

  /*
   *This should produce the following:
   * {"type": "hints", "data": [123,23]}
   * where each number in the data array corresponds to a certain hint that was given to the user
   **/

}

void RobotState::setHint(const int &hint)
{
  m_hints.insert(hint);
}

void RobotState::clearHint(const int &hint)
{
  m_hints.erase(hint);
}

void RobotState::clearAllHints()
{
  m_hints.clear();
}

std::set<int> RobotState::getActiveHints()
{
  return m_hints;
}


void RobotState::setJointPositions(const std::vector<double> &joint_positions)
{
  jnt_value_map::iterator it=m_joints.begin();
  for (size_t i = 0; i < m_joints.size() && i < joint_positions.size(); ++i)
  {
    it->second.position = static_cast<double>(joint_positions[i]*m_input_to_rad_factor);
    it++;
  }
}

void RobotState::setJointPosition(const double &joint_position,size_t joint)
{
  m_joints[joint].position = joint_position*m_input_to_rad_factor;
  //std::cout << "setJointPositions: jnt " << joint << " updated to " << joint_position*m_input_to_rad_factor << std::endl;
}


void RobotState::setJointPositions(const std::vector<int> &joint_positions)
{
  jnt_value_map::iterator it=m_joints.begin();

  for (size_t i = 0; i < m_joints.size() && i < joint_positions.size(); ++i)
  {
    it->second.position = static_cast<double>(joint_positions[i]*m_input_to_rad_factor);
    it++;
  }
}

void RobotState::setJointPosition(const int &joint_position,size_t joint)
{
  m_joints[joint].position = joint_position*m_input_to_rad_factor;
  //std::cout << "setJointPositions: jnt " << joint << " updated to " << joint_position*m_input_to_rad_factor << std::endl;
}

void RobotState::setJointVelocites(const std::vector<double> &joint_velocities)
{
  jnt_value_map::iterator it=m_joints.begin();

  for (size_t i = 0; i < m_joints.size() && i < joint_velocities.size(); ++i)
  {
    it->second.velocity = static_cast<double>(joint_velocities[i]*m_input_to_rad_factor);
    it++;
  }
}

void RobotState::setJointVelocity(const double &joint_velocity, const size_t &joint)
{
  m_joints[joint].velocity = joint_velocity*m_input_to_rad_factor;
}


void RobotState::setJointErrors(const std::vector<ErrorState> &joint_errors)
{
  jnt_value_map::iterator it=m_joints.begin();

  for (size_t i = 0; i < m_joints.size() && i < joint_errors.size(); ++i)
  {
    it->second.error = joint_errors[i] ;//?eERR_FAULT:eERR_NO_ERROR;
    it++;
  }
}

void RobotState::setJointError(const ErrorState &joint_error,const size_t &joint)
{
  m_joints[joint].error = joint_error ;//?eERR_FAULT:eERR_NO_ERROR;
}

void RobotState::setJointErrors(const std::vector<bool> &faults)
{
  std::vector<ErrorState> joint_errors(faults.size(), eERR_NO_ERROR);
  for (size_t i = 0; i < faults.size(); ++i)
  {
    joint_errors[i] = faults[i]?eERR_FAULT:eERR_NO_ERROR;
  }
  setJointErrors(joint_errors);
}

void RobotState::setJointError(const bool &fault, const size_t &joint)
{
    m_joints[joint].error = fault?eERR_FAULT:eERR_NO_ERROR;
}

void RobotState::setJointsEnabled(const std::vector<bool> &joints_enabled)
{
  jnt_value_map::iterator it=m_joints.begin();

  for (size_t i = 0; i < m_joints.size() && i < joints_enabled.size(); ++i)
  {
    it->second.enabled = joints_enabled[i]?eERR_NO_ERROR:eERR_FAULT;
    std::cout << "setJointEnabled: jnt " << i << " updated to " << it->second.enabled << std::endl;
    it++;
  }

}

void RobotState::setJointEnabled(const bool &enabled,const size_t &joint)
{
  m_joints[joint].enabled = enabled?eERR_NO_ERROR:eERR_FAULT;
}

void RobotState::setJointsHomed(const std::vector<bool> &joints_homed)
{
  jnt_value_map::iterator it=m_joints.begin();
  for (size_t i = 0; i < m_joints.size() && i < joints_homed.size(); ++i)
  {
    it->second.homed = joints_homed[i]?eERR_NO_ERROR:eERR_FAULT;
    it++;
  }
}

void RobotState::setJointHomed(const bool &homed,const size_t &joint)
{
  m_joints[joint].homed = homed?eERR_NO_ERROR:eERR_FAULT;
}

size_t RobotState::getNumAxes() const
{
  return m_joints.size();
}


//void RobotState::setState(const std::vector<double> &joint_positions,const std::vector<double> &joint_velocities,const std::vector<bool> &faults,const MovementState &movement_state)
//{
//  setJointPositions(joint_positions);
//  setJointVelocites(joint_velocities);
//  setJointErrors(faults);
//  setMovementState(movement_state);
//}


}}
