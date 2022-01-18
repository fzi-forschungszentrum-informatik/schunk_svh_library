// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Klaus Fischnaller <fischnal@fzi.de>
 * \date    2015-01-23
 *
 */
//----------------------------------------------------------------------

#include "schunk_svh_library/websocket/ZMQClient.h"
#include <string>
#include <thread>

namespace schunk_svh_library {
namespace websocket {

ZMQClient::ZMQClient(unsigned short recvPort, unsigned short sendPort)
  : m_recvPort(recvPort)
  , m_sendPort(sendPort)
  , m_zmqContext(1)
  , m_sender(m_zmqContext, ZMQ_PUSH)
  , m_receiver(m_zmqContext, ZMQ_SUB)
  , m_listening(false)
{
  m_receiver.setsockopt(ZMQ_SUBSCRIBE, "", 0);

  int val = 0;
  m_sender.setsockopt(ZMQ_LINGER, &val, sizeof(val));

  std::string sendTo = "tcp://127.0.0.1:" + std::to_string(m_sendPort);
  m_sender.connect(sendTo.c_str());
}

ZMQClient::~ZMQClient()
{
  stop();
}

bool ZMQClient::sendRawMessage(std::string msg)
{
  return zmq_send(static_cast<void*>(m_sender), msg.data(), msg.size(), ZMQ_DONTWAIT);
}

void ZMQClient::addCallback(WsbCallback* cb)
{
  // add new message_receiver to list
  m_callbacks.push_back(cb);
}

void ZMQClient::startListening()
{
  // Fresh restart on repetitive calls.
  if (m_listener.joinable())
  {
    m_listener.join();
  }
  m_listener = std::thread(&ZMQClient::msgListener, this);
}

void ZMQClient::msgListener()
{
  m_listening = true;

  std::string recvFrom = "tcp://127.0.0.1:" + std::to_string(m_recvPort);
  m_receiver.connect(recvFrom.c_str());

  zmq::pollitem_t items[] = {
    {static_cast<void*>(m_receiver), 0, ZMQ_POLLIN, 0},
  };

  while (m_listening)
  {
    zmq::poll(&items[0], 1, 1000);
    if (items[0].revents & ZMQ_POLLIN)
    {
      zmq::message_t message;
      m_receiver.recv(message);

      std::string msg = std::string(static_cast<char*>(message.data()), message.size());

      for (size_t i = 0; i < m_callbacks.size(); i++)
      {
        m_callbacks[i]->onWSBClientMessage(msg);
      }
    }
  }
}

void ZMQClient::stop()
{
  m_listening = false;
  if (m_listener.joinable())
  {
    m_listener.join();
  }
}


} // namespace websocket
} // namespace schunk_svh_library
