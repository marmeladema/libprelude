#ifndef _LIBPRELUDE_PRELUDE_CLIENT_HXX
#define _LIBPRELUDE_PRELUDE_CLIENT_HXX

#include "idmef.hxx"
#include "prelude-connection-pool.hxx"
#include "prelude-client-profile.hxx"


namespace Prelude {
        class IDMEF;

        class Client : public ClientProfile {
            private:
                prelude_client_t *_client;
                ConnectionPool _pool;

            protected:
                int _recv_timeout;

            public:
                enum ClientFlagsEnum {
                        ASYNC_SEND  = PRELUDE_CLIENT_FLAGS_ASYNC_SEND,
                        FLAGS_ASYNC_SEND   = PRELUDE_CLIENT_FLAGS_ASYNC_SEND,
                        ASYNC_TIMER = PRELUDE_CLIENT_FLAGS_ASYNC_TIMER,
                        FLAGS_ASYNC_TIMER  = PRELUDE_CLIENT_FLAGS_ASYNC_TIMER,
                        HEARTBEAT   = PRELUDE_CLIENT_FLAGS_HEARTBEAT,
                        FLAGS_HEARTBEAT   = PRELUDE_CLIENT_FLAGS_HEARTBEAT,
                        CONNECT     = PRELUDE_CLIENT_FLAGS_CONNECT,
                        FLAGS_CONNECT     = PRELUDE_CLIENT_FLAGS_CONNECT,
                        AUTOCONFIG  = PRELUDE_CLIENT_FLAGS_AUTOCONFIG,
                        FLAGS_AUTOCONFIG = PRELUDE_CLIENT_FLAGS_AUTOCONFIG,
                };

                enum ConnectionPermissionEnum {
                        IDMEF_READ  = PRELUDE_CONNECTION_PERMISSION_IDMEF_READ,
                        PERMISSION_IDMEF_READ = PRELUDE_CONNECTION_PERMISSION_IDMEF_READ,
                        ADMIN_READ  = PRELUDE_CONNECTION_PERMISSION_ADMIN_READ,
                        PERMISSION_ADMIN_READ  = PRELUDE_CONNECTION_PERMISSION_ADMIN_READ,
                        IDMEF_WRITE = PRELUDE_CONNECTION_PERMISSION_IDMEF_WRITE,
                        PERMISSION_IDMEF_WRITE  = PRELUDE_CONNECTION_PERMISSION_IDMEF_WRITE,
                        ADMIN_WRITE = PRELUDE_CONNECTION_PERMISSION_ADMIN_WRITE,
                        PERMISSION_ADMIN_WRITE  = PRELUDE_CONNECTION_PERMISSION_ADMIN_WRITE,
                };

                ~Client();
                Client(const char *profile);
                Client(const Client &client);

                void start();
                void init();

                prelude_client_t *getClient();

                void sendIDMEF(const Prelude::IDMEF &message);
                int recvIDMEF(Prelude::IDMEF &idmef, int timeout=-1);

                int getFlags();
                void setFlags(int flags);

                int getRequiredPermission();
                void setRequiredPermission(int permission);

                const char *getConfigFilename();
                void setConfigFilename(const char *name);

                Prelude::ConnectionPool &getConnectionPool();
                void setConnectionPool(Prelude::ConnectionPool pool);

                Client &operator << (Prelude::IDMEF &idmef);
                Client &operator >> (Prelude::IDMEF &idmef);
                Client &operator=(const Client &p);

                static Client &setRecvTimeout(Client &c, int timeout);
        };
};

#endif
