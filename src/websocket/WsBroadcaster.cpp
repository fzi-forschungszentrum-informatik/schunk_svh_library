#include "schunk_svh_library/websocket/WsBroadcaster.h"

#include <icl_core_logging/Logging.h>
#include <json/json.h>
#include <thread>
#include <chrono>



namespace schunk_svh_library {
namespace websocket {

DECLARE_LOG_STREAM(WebSocket);
REGISTER_LOG_STREAM(WebSocket);
 
using icl_core::logging::endl;

bool WsBroadcaster::checkSocket()
{
  if (! m_socket)
  {
      m_socket.reset(new schunk_svh_library::websocket::ZMQClient(5566,5567));
      m_socket->addCallback(this);
      m_socket->startListening();
  }

  return true;
}

bool WsBroadcaster::sendState()
{
  // Abort immediately of the socket could not even be created
  if (!checkSocket())
  {
    return false;
  }

  // Send out the state as JSON encoded String
  if (!m_socket->sendRawMessage(robot->getStateJSON()) )
  {
    return false;
  }

  // This is ugly.. but otherwise the other process is never active that reads the content of the socket and after that the messages will be parsed as one....
  std::this_thread::sleep_for(std::chrono::microseconds(5));
  return true;
}


bool WsBroadcaster::sendHints()
{
  // Abort immediately of the socket could not even be created
  if (!checkSocket())
  {
    return false;
  }

  // Send out the state as JSON encoded String
  if (!m_socket->sendRawMessage(robot->getHintsJSON()) )
  {
    return false;
  }

  // This is ugly.. but otherwise the other process is never active that reads the content of the socket and after that the messages will be parsed as one....
  std::this_thread::sleep_for(std::chrono::microseconds(5));
  return true;
}


void WsBroadcaster::onWSBClientMessage(std::string msg)
{
  // In case we get something we will always update all the information
  sendState();



  // Parse the rest of the message to get hints
  Json::Value root;
  Json::Reader reader;

  bool parsedSuccess = reader.parse(msg,root,false);
  if (parsedSuccess && root["cmd"] == "hint_special")
  {
     int code = root["data"]["code"].asInt();
     // and allow for custom handling
     if (m_received_callback)
     {
       m_received_callback(code);
     }

  }

  // NOTE! We may NOT send to fast to the unix socket as messages will get mangled otherwise! If you send 2 Messages right after another -> use sleeps! :/
  // Send all Hints that are currently active
  sendHints();
}

}
}
