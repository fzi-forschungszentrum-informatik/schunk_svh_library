// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-


// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner <heppner@fzi.de>
 * \date    2014-5-22
 *
 */
//----------------------------------------------------------------------

#include "schunk_svh_library/websocket/SVHState.h"

#include <json/json.h>
#include <json/writer.h>

namespace schunk_svh_library {
namespace websocket {

//! Description for enum matching:
const std::string SVHState::m_joint_descriptions[] = {"thumb_flexion",
                                                      "thumb_opposition", // wrist
                                                      "index_distal",
                                                      "index_proximal",
                                                      "middle_distal",
                                                      "middle_proximal",
                                                      "ring",
                                                      "pinky",
                                                      "spread"};


SVHState::SVHState()
  : RobotState()
  , m_movement_state(eST_DEACTIVATED)
{
}


std::string SVHState::getStateJSON()
{
  Json::Value message;
  Json::FastWriter json_writer;

  message["type"] = "hand";
  switch (m_movement_state)
  {
    case eST_DEACTIVATED:
      message["state"] = "deactivated";
      break;
    case eST_RESETTING:
      message["state"] = "resetting";
      break;
    case eST_RESETTED:
      message["state"] = "resetted, waiting for activation";
      break;
    case eST_FAULT:
      message["state"] = "fault";
      break;
    case eST_ENABLED:
      message["state"] = "enabled";
      break;
    case eST_PARTIALLY_ENABLED:
      message["state"] = "partially enabled";
      break;
    default:
      message["state"] = "unknown";
  };

  for (jnt_value_map::iterator it = m_joints.begin(); it != m_joints.end(); it++)
  {
    Json::Value joint;
    // joint["id"] = (unsigned int)i++;
    joint["id"]       = (unsigned int)it->first;
    joint["position"] = it->second.position;
    joint["speed"]    = it->second.velocity;
    joint["enabled"]  = it->second.enabled;
    joint["homed"]    = it->second.homed;
    joint["error"]    = it->second.error;

    message["data"].append(joint);
  }

  return json_writer.write(message);

  /*
   *This will Produce the following message:
   * {"data":[{"enabled":2,"error":0,"homed":0,"id":0,"position":0.05278511769938632,"speed":0.0},
   *          {"enabled":2,"error":0,"homed":0,"id":1,"position":0.09083249904051627,"speed":0.0},usw
   *         ],"state":"resetting","type":"hand"}
   */
}


void SVHState::setMovementState(const int& movement_state)
{
  m_movement_state = static_cast<MovementState>(movement_state);
}


} // namespace websocket
} // namespace schunk_svh_library
