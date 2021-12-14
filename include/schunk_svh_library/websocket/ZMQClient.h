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

#include <zmq.hpp>
#include <vector>
#include <schunk_svh_library/websocket/WsbCallback.h>
#include <schunk_svh_library/ImportExport.h>

#include <boost/thread.hpp>

#ifndef ZMQCLIENT_H
#define ZMQCLIENT_H

namespace schunk_svh_library {
namespace websocket {

class DRIVER_SVH_IMPORT_EXPORT ZMQClient
{
public:
    /*!
     * \brief ZMQClient constructor
     * \param recvPort receiving port
     * \param sendPort sending port
     */
    ZMQClient(unsigned short recvPort, unsigned short sendPort);

    /*!
     * \brief ZMQClient destructor
     */
    ~ZMQClient();

    /*!
      * \brief Sends a message to the broadcasting server
      * \param msg the message to be sent
      */
    bool sendRawMessage(std::string msg);

    /*!
      * \brief Adds WsbCallback instance
      * \param cb callback instance
      */
    void addCallback(WsbCallback *cb);

    /*!
      * \brief Starts the receiver thread.
      */
    void startListening();

    /*!
      * \brief Receiver thread.
      */
    void msgListener();

    /*!
      * \brief Closes the socket and stops the listeing thread.
      */
    void stop();

private:

    unsigned short m_recvPort;
    unsigned short m_sendPort;

    zmq::context_t m_zmqContext;
    zmq::socket_t  m_sender;
    zmq::socket_t  m_receiver;

    std::vector<WsbCallback *> m_callbacks;

    bool m_listening;

    boost::thread m_listener;
};

}}

#endif // ZMQCLIENT_H
