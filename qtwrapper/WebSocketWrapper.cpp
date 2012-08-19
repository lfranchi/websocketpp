/*
   Copyright 2012,      Leo Franchi <lfranchi@kde.org>
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of the WebSocket++ Project nor the
          names of its contributors may be used to endorse or promote products
          derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
    ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "WebSocketWrapper.h"

#include <QDebug>

#include <websocketpp/sockets/tls.hpp>
#include <websocketpp/roles/client.hpp>
#include <websocketpp/websocketpp.hpp>
#include <websocketpp/endpoint.hpp>

using websocketpp::client;
using websocketpp::client_tls;

class WebSocketWrapperPrivate
{
public:
    WebSocketWrapperPrivate(const QString& theUrl, WebSocketWrapper* qq) : q(qq), isTls(false), url( theUrl ) {}

    void receivedMessage(const QString& msg) {
        q->message(msg);
    }

    void onFail(const QString& reason) {
        q->failed(reason);
    }

    void onClose(const QString& reason) {
        q->closed(reason);
    }

    void onOpen() {
        q->opened();
    }

    WebSocketWrapper* q;

    client::handler::ptr handler;

    client_tls::handler::ptr handler_tls;

    bool isTls;
    QString url;
};

template <typename endpoint_type>
class ClientHandler : public endpoint_type::handler
{
public:
    ClientHandler(WebSocketWrapperPrivate* p) : pimpl(p) {}
    virtual ~ClientHandler() {}

    typedef typename endpoint_type::handler::connection_ptr connection_ptr;
    typedef typename endpoint_type::handler::message_ptr message_ptr;

    void on_fail(connection_ptr con)
    {
        const QString reason = QString::fromStdString(con->get_fail_reason());
        pimpl->onFail(reason);
        qDebug() << "Connection Failed: " << reason;
    }

    void on_open(connection_ptr con)
    {
        qDebug() << "Connection Opened";
        pimpl->onOpen();
        m_con = con;
    }

    void on_close(connection_ptr con)
    {
        const QString reason = QString::fromStdString(con->get_remote_close_reason());
        pimpl->onClose(reason);
        qDebug() << "Connection Closed";
        m_con = connection_ptr();
    }

    boost::shared_ptr<boost::asio::ssl::context> on_tls_init() {
        boost::shared_ptr<boost::asio::ssl::context> context(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));
        context->set_options(boost::asio::ssl::context::default_workarounds);
        return context;
    }

    void on_message(connection_ptr con, message_ptr msg)
    {
        const QString payload = QString::fromStdString(msg->get_payload());
        pimpl->receivedMessage(payload);
        qDebug() << "Got Message:" << payload;
    }


    void send(const QString& msg)
    {
        if (!m_con) {
            qDebug() << "Tried to send on a disconnected connection! Aborting.";
            return;
        }

        std::string payload = msg.toStdString();

        m_con->send(payload);
    }

    void close()
    {
        if (!m_con) {
            qDebug() << "Tried to close a disconnected connection!";
            return;
        }

        m_con->close(websocketpp::close::status::GOING_AWAY,"");
    }

    websocketpp::session::state::value state() const {
        if (!m_con) {
            return websocketpp::session::state::CLOSED;
        }

        return m_con->get_state();
    }
private:
    WebSocketWrapperPrivate* pimpl;
    connection_ptr m_con;
};

WebSocketWrapper::WebSocketWrapper(const QString& url, QObject* parent)
    : QThread(parent)
    , pimpl(new WebSocketWrapperPrivate(url, this))
{
}

WebSocketWrapper::~WebSocketWrapper() {
    // If the thread is still running, close the connection and wait for the run() function to exit
    if (isRunning()) {
        stop();
        wait(10000);
    }
}


void WebSocketWrapper::run()
{
    const bool isTls = pimpl->url.startsWith( "wss:/" );

//     con->add_subprotocol("com.zaphoyd.websocketpp.chat");

//     Origin not required for non-browser clients
//     con->set_origin("http://zaphoyd.com");

    try {
        if (isTls) {
            pimpl->isTls = true;
            pimpl->handler_tls = client_tls::handler::ptr(new ClientHandler<client_tls>(pimpl.data()));

            client_tls::connection_ptr con;
            client_tls endpoint(pimpl->handler_tls);

            endpoint.alog().set_level(websocketpp::log::alevel::ALL);
            endpoint.elog().set_level(websocketpp::log::elevel::ALL);

            con = endpoint.get_connection(pimpl->url.toStdString());

            endpoint.connect(con);

            con->add_request_header("User-Agent","Tomahawk/0.2.0 TomahawkAccount/0.2.0");
            endpoint.run(false);
        } else {
            pimpl->isTls = false;
            pimpl->handler = client::handler::ptr(new ClientHandler<client>(pimpl.data()));

            client::connection_ptr con;
            client endpoint(pimpl->handler);

            endpoint.alog().set_level(websocketpp::log::alevel::ALL);
            endpoint.elog().set_level(websocketpp::log::elevel::ALL);

            con = endpoint.get_connection(pimpl->url.toStdString());

            endpoint.connect(con);

            con->add_request_header("User-Agent","Tomahawk/0.2.0 TomahawkAccount/0.2.0");
            endpoint.run(false);
        }
    } catch(websocketpp::exception& e) {
        qWarning() << "Caught exception trying to get connection to endpoint: " << pimpl->url << e.code() << e.what();
        return;
    } catch (const char* msg) {
        qWarning() << "Const const char& exception:" << msg;
        return;
    }
}


void WebSocketWrapper::send(const QString& msg)
{
    Q_ASSERT(!pimpl.isNull());
    if (pimpl.isNull())
        return;

    // NOTE connection::send() is threadsafe
    if (pimpl->isTls) {
        ClientHandler<client_tls>* client = dynamic_cast<ClientHandler<client_tls>*>(pimpl->handler_tls.get());
        if (!client)
            return;

        client->send(msg);
    } else {
        ClientHandler<client>* theClient = dynamic_cast<ClientHandler<client>*>(pimpl->handler.get());
        if (!theClient)
            return;
        theClient->send(msg);
    }
}

void
WebSocketWrapper::stop()
{
    Q_ASSERT(!pimpl.isNull());
    if (pimpl.isNull())
        return;

    // NOTE connection::close() is threadsafe
    if (pimpl->isTls) {
        ClientHandler<client_tls>* client = dynamic_cast<ClientHandler<client_tls>*>(pimpl->handler_tls.get());
        if (!client)
            return;

        client->close();
    } else {
        ClientHandler<client>* theClient = dynamic_cast<ClientHandler<client>*>(pimpl->handler.get());
        if (!theClient)
            return;
        theClient->close();
    }
}
