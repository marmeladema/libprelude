#include "idmef.hxx"
#include "prelude-error.hxx"
#include "prelude-client.hxx"
#include "prelude-client-profile.hxx"


using namespace Prelude;


Client::Client(const char *profile)
        : _recv_timeout(-1)
{
        int ret;

        ret = prelude_client_new(&_client, profile);
        if ( ret < 0 )
                throw PreludeError(ret);

        _profile = prelude_client_get_profile(_client);
        _pool = ConnectionPool(prelude_connection_pool_ref(prelude_client_get_connection_pool(_client)));
}


Client::Client(const Client &client)
{
        _client = (client._client) ? prelude_client_ref(client._client) : NULL;
}


Client::~Client()
{
        _profile = NULL;
        prelude_client_destroy(_client, PRELUDE_CLIENT_EXIT_STATUS_SUCCESS);
}


void Client::start()
{
        int ret;

        init();

        ret = prelude_client_start(_client);
        if ( ret < 0 )
                throw PreludeError(ret);
}


void Client::init()
{
        int ret;

        ret = prelude_client_init(_client);
        if ( ret < 0 )
                throw PreludeError(ret);

        _profile = prelude_client_get_profile(_client);
}


prelude_client_t *Client::getClient()
{
        return _client;
}


void Client::sendIDMEF(const IDMEF &message)
{
        prelude_client_send_idmef(_client, (idmef_message_t *) (idmef_object_t *) message);
}


int Client::recvIDMEF(Prelude::IDMEF &idmef, int timeout)
{
        int ret;
        idmef_message_t *idmef_p;

        ret = prelude_client_recv_idmef(_client, timeout, &idmef_p);
        if ( ret < 0 )
                throw PreludeError(ret);

        else if ( ret == 0 )
                return 0;

        idmef = IDMEF((idmef_object_t *)idmef_p);

        return 1;
}


int Client::getFlags()
{
        return prelude_client_get_flags(_client);
}


void Client::setFlags(int flags)
{
        int ret;

        ret = prelude_client_set_flags(_client, (prelude_client_flags_t) flags);
        if ( ret < 0 )
                throw PreludeError(ret);
}


int Client::getRequiredPermission()
{
        return prelude_client_get_required_permission(_client);
}


void Client::setRequiredPermission(int permission)
{
        prelude_client_set_required_permission(_client, (prelude_connection_permission_t) permission);
}


const char *Client::getConfigFilename()
{
        return prelude_client_get_config_filename(_client);
}


void Client::setConfigFilename(const char *name)
{
        int ret;

        ret = prelude_client_set_config_filename(_client, name);
        if ( ret < 0 )
                throw PreludeError(ret);
}


ConnectionPool &Client::getConnectionPool()
{
        return _pool;
}


void Client::setConnectionPool(ConnectionPool pool)
{
        _pool = pool;
        prelude_client_set_connection_pool(_client, prelude_connection_pool_ref(pool));
}


Client &Client::operator << (IDMEF &idmef)
{
        sendIDMEF(idmef);
        return *this;
}


Client &Client::operator >> (IDMEF &idmef)
{
        int ret;

        ret = recvIDMEF(idmef, _recv_timeout);
        if ( ret <= 0 )
                throw PreludeError(ret);

        return *this;
}

Client &Client::setRecvTimeout(Client &c, int timeout)
{
        c._recv_timeout = timeout;
        return c;
}


Client &Client::operator=(const Client &c)
{
        if ( this != &c && _client != c._client ) {
                if ( _client )
                        prelude_client_destroy(_client, PRELUDE_CLIENT_EXIT_STATUS_SUCCESS);

                _client = (c._client) ? prelude_client_ref(c._client) : NULL;
        }

        return *this;
}
